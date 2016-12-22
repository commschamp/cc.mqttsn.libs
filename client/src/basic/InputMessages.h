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

#include "mqttsn/protocol/AllMessages.h"

namespace mqttsn
{

namespace client
{

template <typename TMessage, typename TOptions>
using InputMessages =
    std::tuple<
        protocol::message::Advertise<TMessage, TOptions>,
        protocol::message::Gwinfo<TMessage, TOptions>,
        protocol::message::Connack<TMessage, TOptions>,
        protocol::message::Willtopicreq<TMessage>,
        protocol::message::Willmsgreq<TMessage>,
        protocol::message::Register<TMessage, TOptions>,
        protocol::message::Regack<TMessage>,
        protocol::message::Publish<TMessage, TOptions>,
        protocol::message::Puback<TMessage>,
        protocol::message::Pubcomp<TMessage>,
        protocol::message::Pubrec<TMessage>,
        protocol::message::Pubrel<TMessage>,
        protocol::message::Suback<TMessage>,
        protocol::message::Unsuback<TMessage>,
        protocol::message::Pingreq<TMessage, TOptions>,
        protocol::message::Pingresp<TMessage>,
        protocol::message::Disconnect<TMessage>,
        protocol::message::Willtopicresp<TMessage>,
        protocol::message::Willmsgresp<TMessage>
    >;
}  // namespace client

}  // namespace mqttsn


