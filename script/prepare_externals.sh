#!/bin/bash

# Input
# BUILD_DIR - Main build directory
# CC - Main C compiler
# CXX - Main C++ compiler
# EXTERNALS_DIR - (Optional) Directory where externals need to be located
# COMMS_REPO - (Optional) Repository of the COMMS library
# COMMS_TAG - (Optional) Tag of the COMMS library
# CC_MQTTSN_REPO - (Optional) Repository of the cc.mqttsn.generated
# CC_MQTTSN_TAG - (Optional) Tag of the cc.mqttsn.generated
# CC_MQTT311_REPO - (Optional) Repository of the cc.mqtt311.generated
# CC_MQTT311_TAG - (Optional) Tag of the cc.mqtt311.generated
# COMMON_INSTALL_DIR - (Optional) Common directory to perform installations
# COMMON_BUILD_TYPE - (Optional) CMake build type
# COMMON_CXX_STANDARD - (Optional) CMake C++ standard
# COMMON_CMAKE_GENERATOR - (Optional) CMake generator
# COMMON_CMAKE_PLATFORM - (Optional) CMake platform

#####################################

if [ -z "${BUILD_DIR}" ]; then
    echo "BUILD_DIR hasn't been specified"
    exit 1
fi

if [ -z "${EXTERNALS_DIR}" ]; then
    EXTERNALS_DIR=${BUILD_DIR}/externals
fi

if [ -z "${COMMS_REPO}" ]; then
    COMMS_REPO=https://github.com/commschamp/comms.git
fi

if [ -z "${COMMS_TAG}" ]; then
    COMMS_TAG=master
fi

if [ -z "${CC_MQTTSN_REPO}" ]; then
    CC_MQTTSN_REPO=https://github.com/commschamp/cc.mqttsn.generated.git
fi

if [ -z "${CC_MQTTSN_TAG}" ]; then
    CC_MQTTSN_TAG=master
fi

if [ -z "${CC_MQTT311_REPO}" ]; then
    CC_MQTT311_REPO=https://github.com/commschamp/cc.mqtt311.generated.git
fi

if [ -z "${CC_MQTT311_TAG}" ]; then
    CC_MQTT311_TAG=master
fi

if [ -z "${COMMON_BUILD_TYPE}" ]; then
    COMMON_BUILD_TYPE=Debug
fi

COMMS_SRC_DIR=${EXTERNALS_DIR}/comms
COMMS_BUILD_DIR=${BUILD_DIR}/externals/comms/build
COMMS_INSTALL_DIR=${COMMS_BUILD_DIR}/install
if [ -n "${COMMON_INSTALL_DIR}" ]; then
    COMMS_INSTALL_DIR=${COMMON_INSTALL_DIR}
fi

CC_MQTTSN_SRC_DIR=${EXTERNALS_DIR}/cc.mqttsn.generated
CC_MQTTSN_BUILD_DIR=${BUILD_DIR}/externals/cc.mqttsn.generated/build
CC_MQTTSN_INSTALL_DIR=${CC_MQTTSN_BUILD_DIR}/install
if [ -n "${COMMON_INSTALL_DIR}" ]; then
    CC_MQTTSN_INSTALL_DIR=${COMMON_INSTALL_DIR}
fi

CC_MQTT311_SRC_DIR=${EXTERNALS_DIR}/cc.mqtt311.generated
CC_MQTT311_BUILD_DIR=${BUILD_DIR}/externals/cc.mqtt311.generated/build
CC_MQTT311_INSTALL_DIR=${CC_MQTT311_BUILD_DIR}/install
if [ -n "${COMMON_INSTALL_DIR}" ]; then
    CC_MQTT311_INSTALL_DIR=${COMMON_INSTALL_DIR}
fi

procs=$(nproc)
if [ -n "${procs}" ]; then
    procs_param="-- -j${procs}"
fi

#####################################

function build_comms() {
    if [ -e ${COMMS_SRC_DIR}/.git ]; then
        echo "Updating COMMS library..."
        cd ${COMMS_SRC_DIR}
        git fetch --all
        git checkout .
        git checkout ${COMMS_TAG}
        git pull --all
    else
        echo "Cloning COMMS library..."
        mkdir -p ${EXTERNALS_DIR}
        git clone -b ${COMMS_TAG} ${COMMS_REPO} ${COMMS_SRC_DIR}
    fi

    echo "Building COMMS library..."
    mkdir -p ${COMMS_BUILD_DIR}
    cmake \
        ${COMMON_CMAKE_GENERATOR:+"-G ${COMMON_CMAKE_GENERATOR}"} ${COMMON_CMAKE_PLATFORM:+"-A ${COMMON_CMAKE_PLATFORM}"} \
        -S ${COMMS_SRC_DIR} -B ${COMMS_BUILD_DIR} -DCMAKE_INSTALL_PREFIX=${COMMS_INSTALL_DIR} \
        -DCMAKE_BUILD_TYPE=${COMMON_BUILD_TYPE} -DCMAKE_CXX_STANDARD=${COMMON_CXX_STANDARD}
    cmake --build ${COMMS_BUILD_DIR} --config ${COMMON_BUILD_TYPE} --target install ${procs_param}
}

function build_mqttsn() {
    if [ -e ${CC_MQTTSN_SRC_DIR}/.git ]; then
        echo "Updating cc.mqttsn.generated library..."
        cd ${CC_MQTTSN_SRC_DIR}
        git fetch --all
        git checkout .
        git checkout ${CC_MQTTSN_TAG}
        git pull --all
    else
        echo "Cloning cc.mqttsn.generated library..."
        mkdir -p ${EXTERNALS_DIR}
        git clone -b ${CC_MQTTSN_TAG} ${CC_MQTTSN_REPO} ${CC_MQTTSN_SRC_DIR}
    fi

    echo "Building cc.mqttsn.generated library..."
    mkdir -p ${CC_MQTTSN_BUILD_DIR}
    cmake \
        ${COMMON_CMAKE_GENERATOR:+"-G ${COMMON_CMAKE_GENERATOR}"} ${COMMON_CMAKE_PLATFORM:+"-A ${COMMON_CMAKE_PLATFORM}"} \
        -S ${CC_MQTTSN_SRC_DIR} -B ${CC_MQTTSN_BUILD_DIR} \
        -DCMAKE_INSTALL_PREFIX=${CC_MQTTSN_INSTALL_DIR} -DCMAKE_BUILD_TYPE=${COMMON_BUILD_TYPE} \
        -DCMAKE_CXX_STANDARD=${COMMON_CXX_STANDARD} -DOPT_REQUIRE_COMMS_LIB=OFF
    cmake --build ${CC_MQTTSN_BUILD_DIR} --config ${COMMON_BUILD_TYPE} --target install ${procs_param}
}

function build_mqtt311() {
    if [ -e ${CC_MQTT311_SRC_DIR}/.git ]; then
        echo "Updating cc.mqtt311.generated library..."
        cd ${CC_MQTT311_SRC_DIR}
        git fetch --all
        git checkout .
        git checkout ${CC_MQTT311_TAG}
        git pull --all
    else
        echo "Cloning cc.mqtt311.generated library..."
        mkdir -p ${EXTERNALS_DIR}
        git clone -b ${CC_MQTT311_TAG} ${CC_MQTT311_REPO} ${CC_MQTT311_SRC_DIR}
    fi

    echo "Building cc.mqtt311.generated library..."
    mkdir -p ${CC_MQTT311_BUILD_DIR}
    cmake \
        ${COMMON_CMAKE_GENERATOR:+"-G ${COMMON_CMAKE_GENERATOR}"} ${COMMON_CMAKE_PLATFORM:+"-A ${COMMON_CMAKE_PLATFORM}"} \
        -S ${CC_MQTT311_SRC_DIR} -B ${CC_MQTT311_BUILD_DIR} \
        -DCMAKE_INSTALL_PREFIX=${CC_MQTT311_INSTALL_DIR} -DCMAKE_BUILD_TYPE=${COMMON_BUILD_TYPE} \
        -DCMAKE_CXX_STANDARD=${COMMON_CXX_STANDARD} -DOPT_REQUIRE_COMMS_LIB=OFF
    cmake --build ${CC_MQTT311_BUILD_DIR} --config ${COMMON_BUILD_TYPE} --target install ${procs_param}
}

set -e
export VERBOSE=1
build_comms
build_mqttsn
build_mqtt311



