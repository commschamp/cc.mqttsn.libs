#!/bin/bash

if [ -z "${CC}" ]; then
    export CC=afl-clang-fast
fi

if [ -z "${CXX}" ]; then
    export CXX=afl-clang-fast++
fi

if [ -z "${EXT_CC}" ]; then
    EXT_CC=clang
fi

if [ -z "${CXX}" ]; then
    EXT_CXX=clang++
fi

if [ -z "${COMMON_BUILD_TYPE}" ]; then
    export COMMON_BUILD_TYPE=Debug
fi

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
ROOT_DIR=$( dirname ${SCRIPT_DIR} )
CC_NAME=$(basename ${CC})
export BUILD_DIR="${ROOT_DIR}/build.afl_fuzz.${CC_NAME}.${COMMON_BUILD_TYPE}"
export COMMON_INSTALL_DIR=${BUILD_DIR}/install
export EXTERNALS_DIR=${ROOT_DIR}/externals
mkdir -p ${BUILD_DIR}

CC=${EXT_CC} CXX=${EXT_CXX} ${SCRIPT_DIR}/prepare_externals.sh

CONFIGS_DIR="${ROOT_DIR}/client/lib/script"
CONFIGS="${CONFIGS_DIR}/BareMetalTestConfig.cmake;${CONFIGS_DIR}/NoGwDiscoverConfig.cmake;${CONFIGS_DIR}/Qos1Config.cmake;${CONFIGS_DIR}/Qos0Config.cmake"

cd ${BUILD_DIR}
cmake .. -DCMAKE_INSTALL_PREFIX=${COMMON_INSTALL_DIR} -DCMAKE_BUILD_TYPE=${COMMON_BUILD_TYPE} \
    -DCC_MQTTSN_USE_CCACHE=OFF -DCC_MQTTSN_CLIENT_APPS=OFF -DCC_MQTTSN_GATEWAY_LIB=OFF \
    -DCC_MQTTSN_WITH_SANITIZERS=ON \
    -DCC_MQTTSN_AFL_FUZZ=ON \
    -DCC_MQTTSN_CUSTOM_CLIENT_CONFIG_FILES="${CONFIGS}" \
    "$@"

procs=$(nproc)
if [ -n "${procs}" ]; then
    procs_param="--parallel ${procs}"
fi

cmake --build ${BUILD_DIR} --config ${COMMON_BUILD_TYPE} --target install ${procs_param}
