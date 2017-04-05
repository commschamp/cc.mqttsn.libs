//
// Copyright 2016 (C). Alex Robenko. All rights reserved.
//

// This file is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.


#pragma once

#include "comms/comms.h"

namespace mqttsn
{

namespace client
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

}  // namespace client

}  // namespace mqttsn


