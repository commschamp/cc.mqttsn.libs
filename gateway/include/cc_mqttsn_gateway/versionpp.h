//
// Copyright 2020 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

/// @file
/// @brief Contains version information of the library in C++ constexpr functions

#pragma once

#include "version.h"

namespace cc_mqttsn_gateway
{

/// @brief Major verion of the library
inline
constexpr unsigned versionMajor()
{
    return CC_MQTTSN_GW_MAJOR_VERSION;
}

/// @brief Minor verion of the library
inline
constexpr unsigned versionMinor()
{
    return CC_MQTTSN_GW_MINOR_VERSION;
}

/// @brief Patch level of the library
inline
constexpr unsigned versionPatch()
{
    return CC_MQTTSN_GW_PATCH_VERSION;
}

/// @brief Create version of the library as single unsigned numeric value.
inline
constexpr unsigned versionCreate(unsigned major, unsigned minor, unsigned patch)
{
    return CC_MQTTSN_GW_MAKE_VERSION(major, minor, patch);
}

/// @brief Version of the library as single numeric value
inline
constexpr unsigned version()
{
    return CC_MQTTSN_GW_VERSION;
}

} // namespace cc_mqttsn_gateway
