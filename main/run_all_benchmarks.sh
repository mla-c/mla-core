#!/usr/bin/env bash

# Resolve workspace directory
WORKSPACE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Source definitions and implementation
source "${WORKSPACE_DIR}/configs.sh"
source "${WORKSPACE_DIR}/lib/base-lib/build/tools/run_benchmarks_impl.sh" "$@"
