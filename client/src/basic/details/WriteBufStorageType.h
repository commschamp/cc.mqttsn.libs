//
// Copyright 2016 - 2024 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "comms/comms.h"

namespace cc_mqttsn_client
{

namespace details
{

template <typename TOpts, bool TAllStatic>
class WriteBufStorageType;

template <typename TOpts>
class WriteBufStorageType<TOpts, true>
{
    static_assert(
        TOpts::HasGwAddStaticStorageSize &&
        TOpts::HasClientIdStaticStorageSize &&
        TOpts::HasTopicNameStaticStorageSize &&
        TOpts::HasMessageDataStaticStorageSize,
        "Not all static storages");
    static const std::size_t Size1 =
        (TOpts::GwAddStaticStorageSize > TOpts::ClientIdStaticStorageSize) ?
            TOpts::GwAddStaticStorageSize : TOpts::ClientIdStaticStorageSize;

    static const std::size_t Size2 =
        (Size1 > TOpts::TopicNameStaticStorageSize) ?
            Size1 : TOpts::TopicNameStaticStorageSize;

    static const std::size_t Size3 =
        (Size2 > TOpts::MessageDataStaticStorageSize) ?
            Size2 : TOpts::MessageDataStaticStorageSize;

    static const std::size_t FinalSize = Size3;
    static const std::size_t MaxOverhead = 10U;

public:
    typedef comms::util::StaticVector<std::uint8_t, FinalSize + MaxOverhead> Type;

};

template <typename TOpts>
class WriteBufStorageType<TOpts, false>
{
public:
    typedef std::vector<std::uint8_t> Type;
};

template <typename TOpts>
using WriteBufStorageTypeT =
    typename WriteBufStorageType<
        TOpts,
        TOpts::HasGwAddStaticStorageSize &&
        TOpts::HasClientIdStaticStorageSize &&
        TOpts::HasTopicNameStaticStorageSize &&
        TOpts::HasMessageDataStaticStorageSize
    >::Type;


}  // namespace details

}  // namespace cc_mqttsn_client


