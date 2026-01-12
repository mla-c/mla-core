#!/usr/bin/env node

/**
 * WASM Runner for Node.js - MLA Platform
 * Command-line interface to load and execute WebAssembly modules with MLA runtime support
 * Uses the same mla_wasm_worker.js API as the browser version
 */

const {Worker} = require('worker_threads');
const fs = require('fs');
const path = require('path');
const readline = require('readline');

// ANSI color codes for terminal output
const colors = {
    reset: '\x1b[0m',
    bright: '\x1b[1m',
    dim: '\x1b[2m',
    red: '\x1b[31m',
    green: '\x1b[32m',
    yellow: '\x1b[33m',
    blue: '\x1b[34m',
    magenta: '\x1b[35m',
    cyan: '\x1b[36m',
    white: '\x1b[37m'
};

// Statistics tracking
const stats = {
    moduleSize: 0,
    loadTime: 0,
    execTime: 0,
    status: 'Idle'
};

let worker = null;
let moduleLoaded = false;
let startTime = 0;

/**
 * Log formatted messages to console
 */
function log(message, type = 'info') {
    let prefix = '';
    let color = colors.white;

    switch (type) {
        case 'info':
            prefix = 'ℹ';
            color = colors.cyan;
            break;
        case 'success':
            prefix = '✓';
            color = colors.green;
            break;
        case 'warning':
            prefix = '⚠';
            color = colors.yellow;
            break;
        case 'error':
            prefix = '✗';
            color = colors.red;
            break;
    }

    // Split message by newlines to handle them properly
    const parts = message.split('\n');

    for (let i = 0; i < parts.length; i++) {
        if (i === 0) {
            // First part gets the prefix
            process.stdout.write(`${color}${prefix} ${parts[i]}`);
        } else {
            // Subsequent parts get a newline before them (but no prefix)
            process.stdout.write(`\n${parts[i]}`);
        }
    }

    // Don't add final newline - let the message control that
    process.stdout.write(colors.reset);
}

/**
 * Write raw output to console (from printf)
 */
function rawOutput(message) {
    // Just write the message as-is, no prefix or formatting
    process.stdout.write(message);
}

/**
 * Display help information
 */
function showHelp() {
    console.log(`
${colors.bright}${colors.cyan}🚀 WASM Runner - MLA Platform (Node.js)${colors.reset}

${colors.bright}Usage:${colors.reset}
  node mla_wasm_runner.js <wasm-file> [options]

${colors.bright}Options:${colors.reset}
  -c, --custom <js-file>    Load custom function implementations from JS file
  -h, --help                Display this help message
  -v, --verbose             Enable verbose output
  --no-run                  Load module only, don't execute

${colors.bright}Examples:${colors.reset}
  node mla_wasm_runner.js program.wasm
  node mla_wasm_runner.js program.wasm --custom functions.js
  node mla_wasm_runner.js program.wasm --no-run

${colors.bright}Supported MLA External Functions:${colors.reset}
  Memory: memcpy, memset, memcmp, memmove, malloc, free
  String: strcpy, strlen, strstr
  I/O: printf, std_read
  Conversion: strtod, strtoll, strtoull
  Utility: sleep
`);
}

/**
 * Display statistics
 */
function showStats() {
    console.log(`
${colors.bright}${colors.magenta}📊 Execution Statistics${colors.reset}
${'─'.repeat(50)}
${colors.bright}Module Size:${colors.reset}      ${(stats.moduleSize / 1024).toFixed(2)} KB
${colors.bright}Load Time:${colors.reset}        ${stats.loadTime} ms
${colors.bright}Execution Time:${colors.reset}   ${stats.execTime} ms
${colors.bright}Status:${colors.reset}           ${getStatusColor(stats.status)}
${'─'.repeat(50)}
    `);
}

function getStatusColor(status) {
    switch (status) {
        case 'Completed':
            return `${colors.green}${status}${colors.reset}`;
        case 'Error':
        case 'Failed':
            return `${colors.red}${status}${colors.reset}`;
        case 'Loading...':
        case 'Running...':
            return `${colors.yellow}${status}${colors.reset}`;
        default:
            return status;
    }
}

/**
 * Create and configure worker
 */
function createWorker() {
    // Create worker with mla_wasm_worker.js
    const workerPath = path.join(__dirname, 'mla_wasm_worker.js');

    if (!fs.existsSync(workerPath)) {
        log(`Worker file not found: ${workerPath}`, 'error');
        process.exit(1);
    }

    worker = new Worker(workerPath);

    worker.on('message', (data) => {
        const {type, message, returnValue, stack} = data;

        switch (type) {
            case 'ready':
                log('Worker initialized and ready\n', 'info');
                break;

            case 'log':
                log(message, 'info');
                break;

            case 'output':
                rawOutput(message);
                break;

            case 'warning':
                log(message, 'warning');
                break;

            case 'error':
                log(`Error: ${message}`, 'error');
                if (stack) {
                    console.log(`${colors.dim}${stack}${colors.reset}`);
                }
                stats.status = 'Error';
                showStats();
                process.exit(1);
                break;

            case 'loaded':
                moduleLoaded = true;
                stats.loadTime = Date.now() - startTime;
                stats.status = 'Loaded';
                log(message, 'success');
                break;

            case 'completed':
                stats.execTime = Date.now() - startTime;
                stats.status = 'Completed';
                log(message, 'success');
                if (returnValue !== undefined) {
                    log(`Return value: ${returnValue}`, 'info');
                }
                showStats();

                // Exit gracefully
                setTimeout(() => {
                    worker.terminate();
                    process.exit(0);
                }, 100);
                break;

            case 'stopped':
                log(message, 'warning');
                stats.status = 'Stopped';
                worker.terminate();
                process.exit(0);
                break;
        }
    });

    worker.on('error', (error) => {
        log(`Worker error: ${error.message}`, 'error');
        stats.status = 'Error';
        process.exit(1);
    });

    worker.on('exit', (code) => {
        if (code !== 0 && stats.status !== 'Completed') {
            log(`Worker stopped with exit code ${code}`, 'error');
        }
    });

    return worker;
}

/**
 * Load custom functions from JS file
 */
function loadCustomFunctions(jsFilePath) {
    try {
        if (!fs.existsSync(jsFilePath)) {
            log(`Custom JS file not found: ${jsFilePath}`, 'error');
            return false;
        }

        const jsCode = fs.readFileSync(jsFilePath, 'utf8');
        const fileSize = fs.statSync(jsFilePath).size;

        log(`Loading custom functions from: ${path.basename(jsFilePath)} (${(fileSize / 1024).toFixed(2)} KB)`, 'info');

        if (worker) {
            worker.postMessage({type: 'loadCustomFunctions', code: jsCode});
            log('Custom functions loaded successfully\n', 'success');
            return true;
        } else {
            log('Worker not initialized\n', 'error');
            return false;
        }
    } catch (error) {
        log(`Failed to load custom functions: ${error.message}\n`, 'error');
        return false;
    }
}

/**
 * Load WASM module
 */
async function loadWasmModule(wasmFilePath) {
    try {
        if (!fs.existsSync(wasmFilePath)) {
            log(`WASM file not found: ${wasmFilePath}`, 'error');
            return false;
        }

        const fileStats = fs.statSync(wasmFilePath);
        stats.moduleSize = fileStats.size;

        log(`Loading WASM module: ${path.basename(wasmFilePath)} (${(stats.moduleSize / 1024).toFixed(2)} KB)`, 'info');

        const wasmBuffer = fs.readFileSync(wasmFilePath);
        const arrayBuffer = wasmBuffer.buffer.slice(
            wasmBuffer.byteOffset,
            wasmBuffer.byteOffset + wasmBuffer.byteLength
        );

        stats.status = 'Loading...';
        startTime = Date.now();

        worker.postMessage({type: 'load', data: arrayBuffer});

        return true;
    } catch (error) {
        log(`Failed to load WASM module: ${error.message}`, 'error');
        stats.status = 'Failed';
        return false;
    }
}

/**
 * Execute main function
 */
function executeMain() {
    if (!moduleLoaded) {
        log('No module loaded\n', 'error');
        return false;
    }

    log('Starting execution...\n', 'info');
    stats.status = 'Running...';
    startTime = Date.now();

    worker.postMessage({type: 'run'});
    return true;
}

/**
 * Parse command line arguments
 */
function parseArgs() {
    const args = process.argv.slice(2);
    const options = {
        wasmFile: null,
        customJs: null,
        verbose: false,
        noRun: false,
        help: false
    };

    for (let i = 0; i < args.length; i++) {
        const arg = args[i];

        if (arg === '-h' || arg === '--help') {
            options.help = true;
        } else if (arg === '-v' || arg === '--verbose') {
            options.verbose = true;
        } else if (arg === '--no-run') {
            options.noRun = true;
        } else if (arg === '-c' || arg === '--custom') {
            if (i + 1 < args.length) {
                options.customJs = args[++i];
            } else {
                log('Missing argument for --custom\n', 'error');
                process.exit(1);
            }
        } else if (!arg.startsWith('-')) {
            if (!options.wasmFile) {
                options.wasmFile = arg;
            }
        } else {
            log(`Unknown option: ${arg}\n`, 'error');
            process.exit(1);
        }
    }

    return options;
}

/**
 * Main function
 */
async function main() {
    console.log(`${colors.bright}${colors.cyan}🚀 WASM Runner - MLA Platform (Node.js)${colors.reset}\n`);

    const options = parseArgs();

    if (options.help || !options.wasmFile) {
        showHelp();
        process.exit(0);
    }

    // Resolve file paths
    options.wasmFile = path.resolve(options.wasmFile);
    if (options.customJs) {
        options.customJs = path.resolve(options.customJs);
    }

    // Create worker
    log('Initializing worker...\n', 'info');
    createWorker();

    // Wait a bit for worker to be ready
    await new Promise(resolve => setTimeout(resolve, 100));

    // Load custom functions if provided
    if (options.customJs) {
        loadCustomFunctions(options.customJs);
        await new Promise(resolve => setTimeout(resolve, 100));
    }

    // Load WASM module
    const loaded = await loadWasmModule(options.wasmFile);
    if (!loaded) {
        process.exit(1);
    }

    // Wait for module to load
    await new Promise(resolve => setTimeout(resolve, 500));

    if (!moduleLoaded) {
        log('Module failed to load\n', 'error');
        process.exit(1);
    }

    // Execute main function unless --no-run is specified
    if (!options.noRun) {
        executeMain();
    } else {
        log('Module loaded successfully (not executing due to --no-run)\n', 'success');
        showStats();
        process.exit(0);
    }
}

// Handle process termination
process.on('SIGINT', () => {
    console.log('\n');
    log('Received interrupt signal, terminating...\n', 'warning');
    if (worker) {
        worker.terminate();
    }
    process.exit(0);
});

// Run main function
main().catch(error => {
    log(`Fatal error: ${error.message}\n`, 'error');
    console.error(error.stack);
    process.exit(1);
});

