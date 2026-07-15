#!/usr/bin/env bash

# Resolve workspace directory
if [ -z "$WORKSPACE_DIR" ]; then
    WORKSPACE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../../../.." && pwd)"
fi

target_config=""
if [ -n "$1" ]; then
    target_config="$1"
fi

# Verify configuration exists if specified
if [ -n "$target_config" ]; then
    config_found=false
    for config in "${BUILD_CONFIGS[@]}"; do
        IFS=';' read -r config_name c_compiler cxx_compiler extra_flags <<< "$config"
        if [ "$target_config" = "$config_name" ]; then
            config_found=true
            break
        fi
    done
    if [ "$config_found" = false ]; then
        echo "Error: Unknown configuration '$target_config'"
        echo "Available configurations: "
        for config in "${BUILD_CONFIGS[@]}"; do
            IFS=';' read -r config_name c_compiler cxx_compiler extra_flags <<< "$config"
            echo "  - $config_name"
        done
        exit 1
    fi
fi

built_count=0
failed_configs=()
skipped_configs=()

for config in "${BUILD_CONFIGS[@]}"; do
    IFS=';' read -r config_name c_compiler cxx_compiler extra_flags <<< "$config"
    
    # If a specific configuration was requested, skip others
    if [ -n "$target_config" ] && [ "$target_config" != "$config_name" ]; then
        continue
    fi
    
    # Check if compiler exists
    eval_c_compiler=$(eval echo "$c_compiler")
    eval_cxx_compiler=$(eval echo "$cxx_compiler")
    
    if [[ "$eval_c_compiler" == /* ]] || [[ "$eval_c_compiler" == ./* ]] || [[ "$eval_c_compiler" == ../* ]]; then
        if [ ! -x "$eval_c_compiler" ]; then
            echo "Warning: Compiler executable not found at $eval_c_compiler. Skipping $config_name."
            skipped_configs+=("$config_name")
            continue
        fi
    else
        if ! command -v "$eval_c_compiler" >/dev/null 2>&1; then
            echo "Warning: Compiler $eval_c_compiler not found in PATH. Skipping $config_name."
            skipped_configs+=("$config_name")
            continue
        fi
    fi
    
    echo "========================================================================"
    echo "Building Configuration: $config_name"
    echo "C Compiler:             $eval_c_compiler"
    echo "C++ Compiler:           $eval_cxx_compiler"
    echo "Extra CMake Flags:      $extra_flags"
    echo "========================================================================"
    
    build_dir="$WORKSPACE_DIR/build/$config_name"
    mkdir -p "$build_dir"
    
    # Run cmake configure
    # Pass $extra_flags unquoted to allow multiple flags to split
    cmake -G Ninja \
          -DCMAKE_C_COMPILER="$eval_c_compiler" \
          -DCMAKE_CXX_COMPILER="$eval_cxx_compiler" \
          -DCMAKE_BUILD_TYPE=Release \
          $extra_flags \
          -S "$WORKSPACE_DIR" \
          -B "$build_dir"
          
    if [ $? -ne 0 ]; then
        echo "Error: CMake configuration failed for $config_name"
        failed_configs+=("$config_name")
        continue
    fi
    
    # Run cmake build
    cmake --build "$build_dir" -j$(nproc)
    if [ $? -ne 0 ]; then
        echo "Error: Build failed for $config_name"
        failed_configs+=("$config_name")
        continue
    fi
    
    built_count=$((built_count + 1))
done

echo "========================================================================"
echo "Build Summary:"
echo "  Successfully built: $built_count configurations"
if [ ${#skipped_configs[@]} -ne 0 ]; then
    echo "  Skipped (compiler missing): ${skipped_configs[*]}"
fi
if [ ${#failed_configs[@]} -ne 0 ]; then
    echo "  Failed: ${failed_configs[*]}"
    exit 1
else
    echo "All built configurations completed successfully!"
    exit 0
fi
