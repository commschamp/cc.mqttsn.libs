#!/bin/bash

if [ -z "${CC}" -o -z "${CXX}" ]; then
    echo "ERROR: Compilers are not provided"
    exit 1
fi

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
ROOT_DIR=$( dirname ${SCRIPT_DIR} )
BUILD_DIR="${ROOT_DIR}/build.${CC}"
mkdir -p ${BUILD_DIR}
cd ${BUILD_DIR}

cmake .. -DCMAKE_INSTALL_PREFIX=install -DCMAKE_BUILD_TYPE=Debug "$@"
