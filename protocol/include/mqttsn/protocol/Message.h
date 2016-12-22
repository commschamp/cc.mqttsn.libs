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

#include <tuple>

#include "comms/comms.h"

#include "MsgTypeId.h"

namespace mqttsn
{

namespace protocol
{

using DefaultOptions =
    std::tuple<
        comms::option::ReadIterator<const std::uint8_t*>,
        comms::option::WriteIterator<std::uint8_t*>
    >;

template <typename... TOptions>
class MessageT : public
    comms::Message<
        comms::option::BigEndian,
        comms::option::MsgIdType<MsgTypeId>,
        comms::option::RefreshInterface,
        TOptions...
    >
{
    typedef
        comms::Message<
            comms::option::BigEndian,
            comms::option::MsgIdType<MsgTypeId>,
            comms::option::RefreshInterface,
            TOptions...
        > Base;
public:

    typedef typename Base::Field Field;

    MessageT() = default;
    MessageT(const MessageT&) = default;
    MessageT(MessageT&&) = default;
    virtual ~MessageT() = default;

    MessageT& operator=(const MessageT&) = default;
    MessageT& operator=(MessageT&&) = default;
};

typedef MessageT<DefaultOptions> Message;

}  // namespace protocol

}  // namespace mqttsn


