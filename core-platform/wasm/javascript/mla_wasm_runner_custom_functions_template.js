// Minimal template for custom WASM functions
// Copy this file and add your own functions

const customMlaFunctions = {
    // First level: Module name (e.g., 'mla', 'custom', etc.)
    // Second level: Functions within that module
    mla_custom: {
        // Example: Simple function that adds two numbers
        external_custom_add: (a, b) => {
            return a + b;
        },

        // Example: Function that logs a message
        external_custom_log: (messagePtr) => {
            const message = readString(messagePtr);
            postMessage({ type: 'log', message: `[Custom] ${message}` });
            return 0;
        },

        // Add your custom functions here...
        // external_your_function: (arg1, arg2) => {
        //     // Your implementation
        //     return result;
        // },
    },

    // You can add other modules here:
    // custom: {
    //     my_custom_function: () => {
    //         // Your implementation
    //         return 0;
    //     },
    // },
};

// Don't modify this - the runner looks for customMlaFunctions

