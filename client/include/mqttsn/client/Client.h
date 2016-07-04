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

#include "mqttsn/protocol/Stack.h"
#include "Message.h"
#include "AllMessages.h"

namespace mqttsn
{

namespace client
{

template <typename... TOptions>
class Client
{
public:
    Client() = default;

    typedef Message::ReadIterator ReadIterator;

    void processData(ReadIterator& iter, std::size_t len)
    {
        MsgPtr msg;
        auto es = m_stack.read(msg, iter, len);
        if (es != comms::ErrorStatus::Success) {
            return;
        }
    }

private:
    typedef protocol::Stack<Message, AllMessages, comms::option::InPlaceAllocation> ProtStack;
    typedef ProtStack::MsgPtr MsgPtr;

   ProtStack m_stack;
};

}  // namespace client

}  // namespace mqttsn


