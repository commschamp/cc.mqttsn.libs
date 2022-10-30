//
// Copyright 2016 - 2020 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>

namespace cc_mqttsn
{

namespace client
{

namespace option
{

template <std::size_t TLimit>
struct ClientsAllocLimit
{
    static const std::size_t Value = TLimit;
};

template <std::size_t TLimit>
struct TrackedGatewaysLimit
{
    static const std::size_t Value = TLimit;
};

template <std::size_t TLimit>
struct RegisteredTopicsLimit
{
    static const std::size_t Value = TLimit;
};

template <std::size_t TSize>
struct GwAddStaticStorageSize
{
    static const std::size_t Value = TSize;
};

template <std::size_t TSize>
struct ClientIdStaticStorageSize
{
    static const std::size_t Value = TSize;
};

template <std::size_t TSize>
struct TopicNameStaticStorageSize
{
    static const std::size_t Value = TSize;
};

template <std::size_t TSize>
struct MessageDataStaticStorageSize
{
    static const std::size_t Value = TSize;
};


}  // namespace option

}  // namespace client

}  // namespace cc_mqttsn


