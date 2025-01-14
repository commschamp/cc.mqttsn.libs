//
// Copyright 2024 - 2025 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "ClientImpl.h"
#include "Config.h"
#include "ObjAllocator.h"

namespace cc_mqttsn_client
{

using ClientAllocator = ObjAllocator<ClientImpl, Config::ClientAllocLimit>;

} // namespace cc_mqttsn_client
