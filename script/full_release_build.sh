#!/bin/bash

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
export COMMON_BUILD_TYPE=Release
${SCRIPT_DIR}/full_build.sh "$@"
