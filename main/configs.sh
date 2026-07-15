# Common configuration definitions for building and running MLA tests/benchmarks

# Workspace root
WORKSPACE_DIR="${WORKSPACE_DIR:-/workspace}"

# Find Node.js
if [ -f "/opt/emsdk/node/22.16.0_64bit/bin/node" ]; then
    NODE_BIN="/opt/emsdk/node/22.16.0_64bit/bin/node"
elif command -v node >/dev/null 2>&1; then
    NODE_BIN="node"
else
    NODE_BIN=""
fi

# Build configurations definitions
# Format: "config_name;C_COMPILER;CXX_COMPILER;EXTRA_CMAKE_FLAGS"
BUILD_CONFIGS=(
    "gcc;gcc;g++;"
    "clang;clang;clang++;"
    "filc;filcc;filcpp;"
    "zig_native;${WORKSPACE_DIR}/lib/base-lib/build/tools/zig/zig-cc.sh;${WORKSPACE_DIR}/lib/base-lib/build/tools/zig/zig-cxx.sh;"
    "emscripten_std;emcc;em++;-DMLA_EMSDK_PATH=/opt/emsdk"
    "emscripten_js;emcc;em++;-DMLA_EMSDK_PATH=/opt/emsdk -DMLA_JS_STANDALONE=ON"
    "zig_wasm;${WORKSPACE_DIR}/lib/base-lib/build/tools/zig/zig-cc.sh;${WORKSPACE_DIR}/lib/base-lib/build/tools/zig/zig-cxx.sh;-DMLA_WASM_STANDALONE=ON"
)

# Test/Benchmark suite executable definitions
# Format: "config_name;runner_type;binary_relative_path"
# runner_type can be: native, node, none
RUN_SUITES=(
    "gcc;native;build/gcc/MLA_C_Test_Linux_Single_Thread"
    "gcc;native;build/gcc/MLA_C_Test_Linux_Multi_Thread"
    "clang;native;build/clang/MLA_C_Test_Linux_Single_Thread"
    "clang;native;build/clang/MLA_C_Test_Linux_Multi_Thread"
    "zig_native;native;build/zig_native/MLA_C_Test_Linux_Single_Thread"
    "zig_native;native;build/zig_native/MLA_C_Test_Linux_Multi_Thread"
    "filc;native;build/filc/MLA_C_Test_Linux_Single_Thread"
    "filc;native;build/filc/MLA_C_Test_Linux_Multi_Thread"
    "emscripten_std;node;build/emscripten_std/MLA_C_Test_WASM_Single_Thread.js"
    "emscripten_std;none;build/emscripten_std/MLA_C_Test_WASM_Single_Thread_Standalone.js"
    "emscripten_js;node;build/emscripten_js/MLA_C_Test_WASM_Single_Thread.js"
    "emscripten_js;none;build/emscripten_js/MLA_C_Test_WASM_Single_Thread_Standalone.js"
    "zig_wasm;none;build/zig_wasm/MLA_C_Test_WASM_Single_Thread_Standalone.wasm"
)
