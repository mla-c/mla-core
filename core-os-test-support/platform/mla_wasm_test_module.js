// Custom test functions for WASM
const customMlaFunctions = {
    mla_test: {
        // Allocates memory and returns pointer
        external_test_malloc: externalMalloc,

        // Frees allocated memory
        external_test_free: externalFree,

        // Returns current time in nanoseconds
        external_test_current_nanoseconds: () => {
            try {
                // Get current time in milliseconds and convert to nanoseconds
                const milliseconds = Date.now();
                const nanoseconds = BigInt(milliseconds) * BigInt(1000000);
                return nanoseconds;
            } catch (e) {
                postMessage({ type: 'error', message: 'test_current_nanoseconds failed: ' + e.message });
                return BigInt(0);
            }
        },

        // Printf-style logging function
        external_test_printf: externalPrintf
    }
};