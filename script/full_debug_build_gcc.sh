#!/bin/bash

if [ -z "${CC}" ]; then
    export CC=gcc
fi

if [ -z "${CXX}" ]; then
    export CXX=g++
fi

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
${SCRIPT_DIR}/full_debug_build.sh "$@"

