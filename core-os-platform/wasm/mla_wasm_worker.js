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
                self.postMessage({ type: 'error', message: 'Memory allocation failed: ' + e.message });
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
            self.postMessage({
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

    external_printf: (formatPtr, argsPtr) => {
        try {
            const format = readString(formatPtr);
            self.postMessage({ type: 'log', message: format });
            return format.length;
        } catch (e) {
            self.postMessage({ type: 'error', message: 'printf error: ' + e.message });
            return 0;
        }
    },

    external_std_read: (bufferPtr, size) => {
        // Read from stdin - not implemented in web context
        self.postMessage({ type: 'warning', message: 'std_read not supported in web context' });
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
        // Cannot block in web worker, just log
        self.postMessage({ type: 'log', message: `Sleep requested: ${milliseconds}ms` });
    }
};

self.addEventListener('message', async (e) => {
    const { type, data, code } = e.data;

    try {
        if (type === 'loadCustomFunctions') {
            self.postMessage({ type: 'log', message: 'Loading custom functions...' });

            try {
                // Create a context with helper functions available
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
                    postMessage: self.postMessage.bind(self)
                };

                // Execute the custom JS code in a function scope
                const customFn = new Function('context', `
                    const { getMemoryView, readString, writeString, wasmMemory, externalMalloc, externalFree, externalPrintf, wasmInstance, textEncoder, textDecoder, self, postMessage } = context;
                    ${code}
                    return typeof customMlaFunctions !== 'undefined' ? customMlaFunctions : {};
                `);

                customFunctions = customFn(context);

                // Count modules and functions
                const moduleNames = Object.keys(customFunctions);
                if (moduleNames.length > 0) {
                    const details = moduleNames.map(moduleName => {
                        const funcs = Object.keys(customFunctions[moduleName] || {});
                        return `${moduleName} (${funcs.length} functions: ${funcs.join(', ')})`;
                    }).join('; ');

                    self.postMessage({
                        type: 'log',
                        message: `Custom modules loaded: ${details}`
                    });
                } else {
                    self.postMessage({
                        type: 'warning',
                        message: 'No custom functions found. Make sure to define customMlaFunctions object.'
                    });
                }
            } catch (error) {
                self.postMessage({
                    type: 'error',
                    message: 'Failed to load custom functions: ' + error.message,
                    stack: error.stack
                });
            }

        } else if (type === 'load') {
            self.postMessage({ type: 'log', message: 'Loading WASM module...' });

            // Compile and instantiate
            const module = await WebAssembly.compile(data);

            // Build import object with custom functions organized by module
            const importObject = {
                mla: { ...mlaImports, ...(customFunctions.mla || {}) },
                env: {
                    memory: wasmMemory
                }
            };

            // Add any additional custom modules beyond 'mla' and 'env'
            for (const moduleName in customFunctions) {
                if (moduleName !== 'mla' && moduleName !== 'env') {
                    importObject[moduleName] = customFunctions[moduleName];
                }
            }

            const instance = await WebAssembly.instantiate(module, importObject);
            wasmInstance = instance;

            // Use the WASM instance's memory if it exports one
            if (instance.exports.memory) {
                wasmMemory = instance.exports.memory;
                memoryBuffer = null; // Force refresh
            }

            // Initialize heap base from WASM if available
            if (instance.exports.__heap_base) {
                heapBase = instance.exports.__heap_base.value;
                heapPointer = heapBase;
            } else {
                heapBase = 65536;
                heapPointer = 65536;
            }

            self.postMessage({ type: 'loaded', message: 'WASM module loaded successfully' });

        } else if (type === 'run') {
            if (!wasmInstance) {
                throw new Error('No WASM module loaded');
            }

            self.postMessage({ type: 'log', message: 'Executing main function...' });

            // Try to find and execute main
            if (wasmInstance.exports.main) {
                const result = wasmInstance.exports.main();
                self.postMessage({
                    type: 'completed',
                    message: `Main function executed successfully. Return value: ${result}`,
                    returnValue: result
                });
            } else if (wasmInstance.exports._start) {
                wasmInstance.exports._start();
                self.postMessage({
                    type: 'completed',
                    message: 'Start function executed successfully'
                });
            } else {
                throw new Error('No main or _start function found in WASM module');
            }

        } else if (type === 'stop') {
            wasmInstance = null;
            wasmMemory = null;
            memoryBuffer = null;
            self.postMessage({ type: 'stopped', message: 'Worker stopped' });
        }
    } catch (error) {
        self.postMessage({
            type: 'error',
            message: error.message,
            stack: error.stack
        });
    }
});

self.postMessage({ type: 'ready', message: 'Worker initialized' });

