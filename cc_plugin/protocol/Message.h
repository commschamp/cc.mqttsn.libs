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

#include "comms_champion/comms_champion.h"
#include "mqttsn/protocol/Message.h"


namespace mqttsn
{

namespace cc_plugin
{

namespace protocol
{

template <typename... TOptions>
class MessageT : public comms_champion::MessageBase<mqttsn::protocol::MessageT, TOptions...>
{
    typedef comms_champion::MessageBase<mqttsn::protocol::MessageT, TOptions...> Base;
public:
    MessageT() = default;
    MessageT(const MessageT&) = default;
    MessageT(MessageT&&) = default;
    virtual ~MessageT() = default;
    MessageT& operator=(const MessageT&) = default;
    MessageT& operator=(MessageT&&) = default;

protected:
    virtual QString idAsStringImpl() const override
    {
        return QString("%1").arg(Base::getId(), 1, 10, QChar('0'));
    }
};

typedef MessageT<> Message;

}  // namespace protocol

}  // namespace cc_plugin

}  // namespace mqttsn
