#!/bin/bash

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
${SCRIPT_DIR}/full_debug_build_gcc.sh -DCMAKE_CXX_STANDARD=20 "$@"

