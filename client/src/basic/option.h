//
// Copyright 2016 - 2020 (C). Alex Robenko. All rights reserved.
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

#include <cstddef>

namespace mqttsn
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

}  // namespace mqttsn


