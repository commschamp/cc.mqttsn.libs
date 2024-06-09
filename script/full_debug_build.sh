#!/bin/bash

if [ -z "${CC}" -o -z "${CXX}" ]; then
    echo "ERROR: Compilers are not provided"
    exit 1
fi

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
ROOT_DIR=$( dirname ${SCRIPT_DIR} )
export BUILD_DIR="${ROOT_DIR}/build.full.${CC}"
export COMMON_INSTALL_DIR=${BUILD_DIR}/install
export COMMON_BUILD_TYPE=Debug
export EXTERNALS_DIR=${ROOT_DIR}/externals
mkdir -p ${BUILD_DIR}

${SCRIPT_DIR}/prepare_externals.sh

CONFIGS_DIR="${ROOT_DIR}/client/lib/script"
CONFIGS="${CONFIGS_DIR}/BareMetalTestConfig.cmake;${CONFIGS_DIR}/NoGwDiscoverConfig.cmake"

cd ${BUILD_DIR}
cmake .. -DCMAKE_INSTALL_PREFIX=${COMMON_INSTALL_DIR} -DCMAKE_BUILD_TYPE=Debug \
    -DCC_MQTTSN_USE_CCACHE=ON \
    -DCC_MQTTSN_WITH_SANITIZERS=ON -DCC_MQTTSN_BUILD_UNIT_TESTS=ON \
    -DCC_MQTTSN_CUSTOM_CLIENT_CONFIG_FILES="${CONFIGS}" \
    "$@"

procs=$(nproc)
if [ -n "${procs}" ]; then
    procs_param="--parallel ${procs}"
fi

cmake --build ${BUILD_DIR} --config ${COMMON_BUILD_TYPE} --target install ${procs_param}
