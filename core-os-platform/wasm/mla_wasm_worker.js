// Polyfill for Node.js worker_threads compatibility
const isNode = typeof process !== 'undefined' && process.versions && process.versions.node;

if (isNode && typeof self === 'undefined') {
    global.self = global;
}

// Create unified messaging functions
let parentPort;
if (isNode) {
    parentPort = require('worker_threads').parentPort;
}

const postMessageToParent = (msg) => {
    if (isNode) {
        parentPort.postMessage(msg);
    } else {
        self.postMessage(msg);
    }
};

const addMessageListener = (handler) => {
    if (isNode) {
        parentPort.on('message', handler);
    } else {
        self.addEventListener('message', (e) => handler(e.data));
    }
};

let wasmInstance = null;
let wasmMemory = null;
let memoryBuffer = null;
let heapBase = 0;
let heapPointer = 0;
let customFunctions = {}; // Store custom functions loaded from external JS

// Text encoder/decoder for string operations
const textEncoder = new TextEncoder();
const textDecoder = new TextDecoder();

// Memory allocation tracking
const allocations = new Map(); // Track allocated blocks: ptr -> size
const freeList = []; // List of freed blocks: { ptr, size }

let sleepView = null;
try {
    const sleepBuffer = new SharedArrayBuffer(4);
    sleepView = new Int32Array(sleepBuffer);
} catch (e) {
    sleepView = undefined;
    console.error('SharedArrayBuffer not supported, sleep function will be limited.', e);
}


// Helper functions
function getMemoryView() {
    if (!memoryBuffer || memoryBuffer.buffer !== wasmMemory.buffer) {
        memoryBuffer = new Uint8Array(wasmMemory.buffer);
    }
    return memoryBuffer;
}

function readString(ptr) {
    const memory = getMemoryView();
    let end = ptr;
    while (memory[end] !== 0) end++;
    return textDecoder.decode(memory.slice(ptr, end));
}

function writeString(ptr, str) {
    const memory = getMemoryView();
    const bytes = textEncoder.encode(str);
    memory.set(bytes, ptr);
    memory[ptr + bytes.length] = 0;
    return ptr;
}

// MLA External Functions
const mlaImports = {
    external_memcpy: (dest, src, n) => {
        const memory = getMemoryView();
        memory.copyWithin(dest, src, src + n);
        return dest;
    },

    external_memset: (ptr, value, num) => {
        const memory = getMemoryView();
        memory.fill(value, ptr, ptr + num);
        return ptr;
    },

    external_memcmp: (ptr1, ptr2, num) => {
        const memory = getMemoryView();
        for (let i = 0; i < num; i++) {
            const diff = memory[ptr1 + i] - memory[ptr2 + i];
            if (diff !== 0) return diff > 0 ? 1 : -1;
        }
        return 0;
    },

    external_memmove: (dest, src, n) => {
        const memory = getMemoryView();
        const temp = new Uint8Array(n);
        temp.set(memory.slice(src, src + n));
        memory.set(temp, dest);
        return dest;
    },

    external_strcpy: (dest, src) => {
        const memory = getMemoryView();
        let i = 0;
        while (memory[src + i] !== 0) {
            memory[dest + i] = memory[src + i];
            i++;
        }
        memory[dest + i] = 0;
        return dest;
    },

    external_strlen: (ptr) => {
        const memory = getMemoryView();
        let len = 0;
        while (memory[ptr + len] !== 0) len++;
        return len;
    },

    external_strstr: (haystack, needle) => {
        const haystackStr = readString(haystack);
        const needleStr = readString(needle);
        const index = haystackStr.indexOf(needleStr);
        return index === -1 ? 0 : haystack + index;
    },

    external_malloc: (size) => {
        // Try to find a suitable freed block first
        const freeIndex = freeList.findIndex(block => block.size >= size);

        if (freeIndex !== -1) {
            const block = freeList[freeIndex];
            freeList.splice(freeIndex, 1);

            // If block is larger than needed, split it
            if (block.size > size + 8) { // 8 bytes minimum for a useful block
                freeList.push({ ptr: block.ptr + size, size: block.size - size });
            }

            allocations.set(block.ptr, size);
            return block.ptr;
        }

        // No suitable freed block, allocate new memory
        const ptr = heapPointer;
        heapPointer += size;

        // Grow memory if needed
        const required = heapPointer;
        const currentPages = wasmMemory.buffer.byteLength / 65536;
        const requiredPages = Math.ceil(required / 65536);

        if (requiredPages > currentPages) {
            try {
                wasmMemory.grow(requiredPages - currentPages);
                memoryBuffer = null; // Force refresh
            } catch (e) {
                postMessageToParent({ type: 'error', message: 'Memory allocation failed: ' + e.message });
                return 0;
            }
        }

        allocations.set(ptr, size);
        return ptr;
    },

    external_free: (ptr) => {
        if (ptr === 0) return; // Null pointer, nothing to free

        const size = allocations.get(ptr);
        if (size === undefined) {
            postMessageToParent({
                type: 'warning',
                message: `Attempted to free unallocated or already freed memory at ${ptr}`
            });
            return;
        }

        // Remove from allocations
        allocations.delete(ptr);

        // Add to free list
        freeList.push({ ptr, size });

        // Optional: Coalesce adjacent free blocks
        freeList.sort((a, b) => a.ptr - b.ptr);
        for (let i = 0; i < freeList.length - 1; i++) {
            const current = freeList[i];
            const next = freeList[i + 1];

            if (current.ptr + current.size === next.ptr) {
                current.size += next.size;
                freeList.splice(i + 1, 1);
                i--; // Check again in case of multiple adjacent blocks
            }
        }
    },

    external_printf: (formatPtr, ...args) => {
        try {
            const format = readString(formatPtr);
            const memory = getMemoryView();
            const view = new DataView(memory.buffer);

            let argIndex = 0;
            let output = '';
            let i = 0;

            while (i < format.length) {
                if (format[i] === '%' && i + 1 < format.length) {
                    i++;

                    // Handle %%
                    if (format[i] === '%') {
                        output += '%';
                        i++;
                        continue;
                    }

                    // Parse flags
                    let flags = { leftAlign: false, showSign: false, space: false, altForm: false, zeroPad: false };
                    while ('-+ #0'.includes(format[i])) {
                        if (format[i] === '-') flags.leftAlign = true;
                        else if (format[i] === '+') flags.showSign = true;
                        else if (format[i] === ' ') flags.space = true;
                        else if (format[i] === '#') flags.altForm = true;
                        else if (format[i] === '0') flags.zeroPad = true;
                        i++;
                    }

                    // Parse width
                    let width = 0;
                    while (format[i] >= '0' && format[i] <= '9') {
                        width = width * 10 + (format[i].charCodeAt(0) - '0'.charCodeAt(0));
                        i++;
                    }

                    // Parse precision
                    let precision = -1;
                    if (format[i] === '.') {
                        i++;
                        precision = 0;
                        while (format[i] >= '0' && format[i] <= '9') {
                            precision = precision * 10 + (format[i].charCodeAt(0) - '0'.charCodeAt(0));
                            i++;
                        }
                    }

                    // Parse length modifier
                    let length = '';
                    if ('hlLzjt'.includes(format[i])) {
                        length = format[i];
                        i++;
                        if ((length === 'h' || length === 'l') && format[i] === length) {
                            length += format[i];
                            i++;
                        }
                    }

                    // Parse conversion specifier
                    const spec = format[i];
                    const arg = args[argIndex++];

                    let value = '';

                    switch (spec) {
                        case 'd':
                        case 'i': {
                            let num = length === 'll' ? view.getBigInt64(arg, true) : view.getInt32(arg, true);
                            num = Number(num);
                            value = num.toString();
                            if (flags.showSign && num >= 0) value = '+' + value;
                            else if (flags.space && num >= 0) value = ' ' + value;
                            break;
                        }

                        case 'u': {
                            let num = length === 'll' ? view.getBigUint64(arg, true) : view.getUint32(arg, true);
                            value = Number(num).toString();
                            break;
                        }

                        case 'o': {
                            let num = length === 'll' ? view.getBigUint64(arg, true) : view.getUint32(arg, true);
                            value = Number(num).toString(8);
                            if (flags.altForm && value !== '0') value = '0' + value;
                            break;
                        }

                        case 'x':
                        case 'X': {
                            let num = length === 'll' ? view.getBigUint64(arg, true) : view.getUint32(arg, true);
                            value = Number(num).toString(16);
                            if (spec === 'X') value = value.toUpperCase();
                            if (flags.altForm && num !== 0n && num !== 0) value = '0x' + value;
                            break;
                        }

                        case 'f':
                        case 'F': {
                            const num = view.getFloat64(arg, true);
                            value = precision >= 0 ? num.toFixed(precision) : num.toString();
                            if (flags.showSign && num >= 0) value = '+' + value;
                            else if (flags.space && num >= 0) value = ' ' + value;
                            break;
                        }

                        case 'e':
                        case 'E': {
                            const num = view.getFloat64(arg, true);
                            value = precision >= 0 ? num.toExponential(precision) : num.toExponential();
                            if (spec === 'E') value = value.toUpperCase();
                            if (flags.showSign && num >= 0) value = '+' + value;
                            break;
                        }

                        case 'g':
                        case 'G': {
                            const num = view.getFloat64(arg, true);
                            const expForm = num.toExponential(precision >= 0 ? precision : 6);
                            const fixedForm = num.toFixed(precision >= 0 ? precision : 6);
                            value = expForm.length < fixedForm.length ? expForm : fixedForm;
                            if (spec === 'G') value = value.toUpperCase();
                            break;
                        }

                        case 'c': {
                            const charCode = view.getInt32(arg, true);
                            value = String.fromCharCode(charCode);
                            break;
                        }

                        case 's': {
                            const strPtr = view.getUint32(arg, true);
                            value = readString(strPtr);
                            if (precision >= 0 && value.length > precision) {
                                value = value.substring(0, precision);
                            }
                            break;
                        }

                        case 'p': {
                            const ptr = view.getUint32(arg, true);
                            value = '0x' + ptr.toString(16).padStart(8, '0');
                            break;
                        }

                        default:
                            value = '%' + spec;
                    }

                    // Apply width and padding
                    if (width > value.length) {
                        const padding = width - value.length;
                        const padChar = (flags.zeroPad && !flags.leftAlign && precision < 0) ? '0' : ' ';

                        if (flags.leftAlign) {
                            value = value + padChar.repeat(padding);
                        } else {
                            value = padChar.repeat(padding) + value;
                        }
                    }

                    output += value;
                    i++;
                } else {
                    output += format[i];
                    i++;
                }
            }

            // Send the complete output as-is - caller must include newlines explicitly
            if (output.length > 0) {
                postMessageToParent({
                    type: 'output',
                    message: output
                });
            }


            return output.length;
        } catch (e) {
            postMessageToParent({ type: 'error', message: 'printf error: ' + e.message });
            return 0;
        }
    },


    external_std_read: (bufferPtr, size) => {
        // Read from stdin - not implemented in web context
        postMessageToParent({ type: 'warning', message: 'std_read not supported in web context' });
        return 0;
    },

    external_strtod: (strPtr, endPtrPtr, result) => {
        try {
            const str = readString(strPtr);
            const value = parseFloat(str);
            const memory = getMemoryView();
            const view = new DataView(memory.buffer);
            view.setFloat64(result, value, true);
            return result;
        } catch (e) {
            return 0;
        }
    },

    external_strtoll: (strPtr, endPtrPtr, result) => {
        try {
            const str = readString(strPtr);
            const value = parseInt(str, 10);
            const memory = getMemoryView();
            const view = new DataView(memory.buffer);
            view.setBigInt64(result, BigInt(value), true);
            return result;
        } catch (e) {
            return 0;
        }
    },

    external_strtoull: (strPtr, endPtrPtr, result) => {
        try {
            const str = readString(strPtr);
            const value = parseInt(str, 10);
            const memory = getMemoryView();
            const view = new DataView(memory.buffer);
            view.setBigUint64(result, BigInt(value), true);
            return result;
        } catch (e) {
            return 0;
        }
    },

    external_sleep: (milliseconds) => {

        if (!sleepView) {
            postMessageToParent({ type: 'warning', message: 'Sleep not supported in this environment.' });
            return;
        }

        Atomics.wait(sleepView, 0, 0, milliseconds);
    }
};

// Message handler
addMessageListener(async (data) => {
    const { type, data: msgData, code } = data;

    try {
        if (type === 'loadCustomFunctions') {
            postMessageToParent({ type: 'log', message: 'Loading custom functions...\n' });

            try {
                const context = {
                    getMemoryView,
                    readString,
                    writeString,
                    wasmMemory: () => wasmMemory,
                    externalMalloc: mlaImports.external_malloc,
                    externalFree: mlaImports.external_free,
                    externalPrintf: mlaImports.external_printf,
                    wasmInstance: () => wasmInstance,
                    textEncoder,
                    textDecoder,
                    self,
                    postMessage: postMessageToParent
                };

                const customFn = new Function('context', `
                    const { getMemoryView, readString, writeString, wasmMemory, externalMalloc, externalFree, externalPrintf, wasmInstance, textEncoder, textDecoder, self, postMessage } = context;
                    ${code}
                    return typeof customMlaFunctions !== 'undefined' ? customMlaFunctions : {};
                `);

                customFunctions = customFn(context);

                const moduleNames = Object.keys(customFunctions);
                if (moduleNames.length > 0) {
                    const details = moduleNames.map(moduleName => {
                        const funcs = Object.keys(customFunctions[moduleName] || {});
                        return `${moduleName} (${funcs.length} functions: ${funcs.join(', ')})`;
                    }).join('; ');

                    postMessageToParent({
                        type: 'log',
                        message: `Custom modules loaded: ${details}\n`
                    });
                } else {
                    postMessageToParent({
                        type: 'warning',
                        message: 'No custom functions found. Make sure to define customMlaFunctions object.\n'
                    });
                }
            } catch (error) {
                postMessageToParent({
                    type: 'error',
                    message: 'Failed to load custom functions: ' + error.message + '\n',
                    stack: error.stack
                });
            }

        } else if (type === 'load') {
            postMessageToParent({ type: 'log', message: 'Loading WASM module...\n' });

            const module = await WebAssembly.compile(msgData);

            const importObject = {
                mla: { ...mlaImports, ...(customFunctions.mla || {}) },
                env: {
                    memory: wasmMemory
                }
            };

            for (const moduleName in customFunctions) {
                if (moduleName !== 'mla' && moduleName !== 'env') {
                    importObject[moduleName] = customFunctions[moduleName];
                }
            }

            const instance = await WebAssembly.instantiate(module, importObject);
            wasmInstance = instance;

            if (instance.exports.memory) {
                wasmMemory = instance.exports.memory;
                memoryBuffer = null;
            }

            if (instance.exports.__heap_base) {
                heapBase = instance.exports.__heap_base.value;
                heapPointer = heapBase;
            } else {
                heapBase = 65536;
                heapPointer = 65536;
            }

            postMessageToParent({ type: 'loaded', message: 'WASM module loaded successfully\n' });

        } else if (type === 'run') {
            if (!wasmInstance) {
                throw new Error('No WASM module loaded');
            }

            postMessageToParent({ type: 'log', message: 'Executing main function...\n' });

            if (wasmInstance.exports.main) {
                const result = wasmInstance.exports.main();
                postMessageToParent({
                    type: 'completed',
                    message: `Main function executed successfully. Return value: ${result}\n`,
                    returnValue: result
                });
            } else if (wasmInstance.exports._start) {
                wasmInstance.exports._start();
                postMessageToParent({
                    type: 'completed',
                    message: 'Start function executed successfully\n'
                });
            } else {
                throw new Error('No main or _start function found in WASM module');
            }

        } else if (type === 'stop') {
            wasmInstance = null;
            wasmMemory = null;
            memoryBuffer = null;
            postMessageToParent({ type: 'stopped', message: 'Worker stopped\n' });
        }
    } catch (error) {
        postMessageToParent({
            type: 'error',
            message: error.message + '\n',
            stack: error.stack
        });
    }
});

postMessageToParent({ type: 'ready', message: 'Worker initialized' });

