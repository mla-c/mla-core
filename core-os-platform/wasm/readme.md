# WebAssembly Platform Module

The WebAssembly (WASM) platform module provides platform-specific implementations and configurations for running CoreOS in web browsers and WASM runtimes. It enables CoreOS applications to run in JavaScript environments with near-native performance.

## Architecture

The WASM platform module implements the following components:

- **Platform Initialization** (`mla_global_platform_wasm.h`): WebAssembly-specific platform setup and utilities
- **Task Management** (`mla_global_platform_task_manager_wasm.h`): Single-threaded cooperative task scheduling for WASM

Note: Network and file system operations in WASM typically require JavaScript interop, as these are not directly available in the WASM sandbox for security reasons.

## Features

### Single-Threaded Execution

WASM currently runs in a single-threaded environment (though multi-threading support is emerging):
- Cooperative multitasking through task manager
- Event-driven architecture
- Non-blocking operations
- Integration with browser event loop

### Sleep Implementation

Sleep in WASM is handled through JavaScript interop and promises:

```cpp
// Sleep implementation yields control back to the JavaScript event loop
// allowing the browser to remain responsive
```

### Memory Management

WASM has a linear memory model:
- Predictable memory layout
- No direct OS memory management
- Growth of memory module handled by runtime
- Efficient memory access patterns

### JavaScript Interoperability

The WASM platform enables calling JavaScript functions and being called from JavaScript:
- Export C++ functions to JavaScript
- Import JavaScript functions into C++
- Exchange data between WASM and JavaScript
- Event handling from browser events

## Usage

### Including the Platform Headers

To use the WASM platform, include the main platform header:

```cpp
#include "core-os-platform/wasm/mla_global_platform_wasm.h"
```

### Configuration

The platform configuration focuses on WASM-specific concerns:
- Memory allocation strategies
- Task manager configuration for single-threaded execution
- JavaScript interop settings
- Buffer sizes optimized for web use

### Building for WebAssembly

CoreOS can be compiled to WebAssembly using Emscripten:

```bash
# Using Emscripten
emconfigure cmake -DPLATFORM=wasm ..
emmake make

# This produces a .wasm file and JavaScript glue code
```

### Integrating with JavaScript

```html
<!DOCTYPE html>
<html>
<head>
    <title>CoreOS WASM Application</title>
</head>
<body>
    <script src="your_application.js"></script>
    <script>
        // Load and initialize the WASM module
        Module.onRuntimeInitialized = function() {
            // Call exported WASM functions
            Module._your_exported_function();
        };
    </script>
</body>
</html>
```

### Exporting Functions

Export C++ functions to make them callable from JavaScript:

```cpp
#include <emscripten.h>

extern "C" {
    EMSCRIPTEN_KEEPALIVE
    void my_exported_function() {
        // This function can be called from JavaScript
    }
}
```

### Importing JavaScript Functions

Import JavaScript functions to call from C++:

```cpp
#include <emscripten.h>

extern "C" {
    // Declare JavaScript function
    extern void js_alert(const char* message);
}

// Call from C++
void show_message() {
    js_alert("Hello from WASM!");
}
```

## Platform-Specific Notes

### Dependencies

The WASM platform requires:
- Emscripten SDK 2.0 or later
- Node.js (for testing WASM modules outside browser)
- Modern web browser with WebAssembly support

### Memory Considerations

WASM memory is limited and controlled:
- Initial memory size specified at compile time
- Can grow up to maximum size
- Browser may limit total memory available
- Optimize memory usage for web deployment

### Threading Limitations

Current WASM standard limitations:
- Primarily single-threaded execution
- Web Workers can run separate WASM instances
- SharedArrayBuffer enables shared memory (with COOP/COEP headers)
- Atomics support for synchronization (in multi-threaded WASM)

### Network Operations

Network in WASM requires JavaScript interop:
- Use Fetch API via JavaScript
- WebSocket support through browser APIs
- CORS restrictions apply
- XMLHttpRequest for HTTP calls

### File System Operations

File system in WASM is virtual:
- Emscripten provides virtual file system (MEMFS, IDBFS)
- MEMFS: In-memory file system (data lost on reload)
- IDBFS: Persistent storage using IndexedDB
- Can mount file systems from JavaScript

```cpp
// Example: Using Emscripten file system
#include <emscripten.h>

// Mount IDBFS for persistent storage
EM_ASM(
    FS.mkdir('/data');
    FS.mount(IDBFS, {}, '/data');
    FS.syncfs(true, function(err) {
        // File system ready
    });
);
```

### Performance Considerations

- WASM executes with near-native performance
- Minimize JavaScript interop overhead (batching calls)
- Optimize memory access patterns
- Use SIMD when available
- Compile with optimization flags (-O3, -Oz for size)

### Size Optimization

For web deployment, minimize WASM size:
- Use `-Oz` for size optimization
- Enable link-time optimization (LTO)
- Strip debug symbols for production
- Use compression (gzip/brotli) for transfer
- Split large modules if possible

### Asynchronous Operations

WASM integrates with JavaScript's async model:
- Use Emscripten's Asyncify for C++ async operations
- Integrate with JavaScript Promises
- Event-driven architecture for UI responsiveness

## Integration with Other Modules

The WASM platform module adapts CoreOS modules for web:
- Task module (single-threaded cooperative scheduling)
- Serializer module (JSON for web communication)
- HTTP module (client via Fetch API with interop)
- UI module (can serve web UI or integrate with existing page)

Note: Network and File System modules require JavaScript interop adapters.

## Use Cases

CoreOS on WASM is ideal for:
- **Web Applications**: Run CoreOS apps in the browser
- **Serverless Functions**: Edge computing with WASM runtimes
- **Cross-Platform Tools**: Single codebase for native and web
- **Interactive Demos**: Show CoreOS capabilities on web
- **Educational Tools**: Learn embedded concepts in browser
- **Sandboxed Execution**: Secure execution of untrusted code

## Browser Compatibility

Modern browsers with WASM support:
- Chrome 57+
- Firefox 52+
- Safari 11+
- Edge 16+
- Opera 44+

For older browsers, consider providing a JavaScript fallback or warning.

## Debugging

Debug WASM applications using:
- Browser DevTools WASM debugging
- Source maps for C++ debugging
- Console logging via JavaScript interop
- Emscripten debugging options (-g flag)
