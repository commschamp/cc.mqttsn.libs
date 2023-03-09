//
// Copyright 2016 - 2020 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "details/OptionsParser.h"

#pragma once

namespace cc_mqttsn_client
{

template <typename... TOptions>
using ParsedOptions = details::OptionsParser<TOptions...>;

}  // namespace cc_mqttsn_client


