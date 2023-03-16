#!/bin/bash

export CC=gcc
export CXX=g++

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
${SCRIPT_DIR}/full_debug_build.sh "$@"

