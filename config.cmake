set (OPT_CC_MQTTSN_LIBS ON)

set (CC_MQTTSN_LIBS_GIT_TAG "v2.0.5")

list (APPEND CC_MQTTSN_LIBS_CMAKE_ARGS
    -DCMAKE_BUILD_TYPE=Release
    -DCMAKE_CXX_STANDARD=17
    -DCC_MQTTSN_CLIENT_DEFAULT_LIB=OFF
)
