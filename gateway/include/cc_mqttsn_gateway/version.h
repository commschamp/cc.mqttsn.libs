//
// Copyright 2020 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

/// @file
/// @brief Contains version information of the library

#pragma once

/// @brief Major verion of the library
#define CC_MQTTSN_GW_MAJOR_VERSION 1U

/// @brief Minor verion of the library
#define CC_MQTTSN_GW_MINOR_VERSION 0U

/// @brief Patch level of the library
#define CC_MQTTSN_GW_PATCH_VERSION 2U

/// @brief Macro to create numeric version as single unsigned number
#define CC_MQTTSN_GW_MAKE_VERSION(major_, minor_, patch_) \
    ((static_cast<unsigned>(major_) << 24) | \
     (static_cast<unsigned>(minor_) << 8) | \
     (static_cast<unsigned>(patch_)))

/// @brief Version of the library as single numeric value
#define CC_MQTTSN_GW_VERSION CC_MQTTSN_GW_MAKE_VERSION(CC_MQTTSN_GW_MAJOR_VERSION, CC_MQTTSN_GW_MINOR_VERSION, CC_MQTTSN_GW_PATCH_VERSION)
