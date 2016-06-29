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
#include "cc_plugin/protocol/Message.h"

#include "cc_plugin/protocol/message/Advertise.h"
#include "cc_plugin/protocol/message/Searchgw.h"
#include "cc_plugin/protocol/message/Gwinfo.h"
#include "cc_plugin/protocol/message/Connect.h"
#include "cc_plugin/protocol/message/Connack.h"
#include "cc_plugin/protocol/message/Willtopicreq.h"
#include "cc_plugin/protocol/message/Willtopic.h"
#include "cc_plugin/protocol/message/Willmsgreq.h"
#include "cc_plugin/protocol/message/Willmsg.h"
#include "cc_plugin/protocol/message/Register.h"
#include "cc_plugin/protocol/message/Regack.h"
#include "cc_plugin/protocol/message/Publish.h"
#include "cc_plugin/protocol/message/Puback.h"
#include "cc_plugin/protocol/message/Pubcomp.h"
#include "cc_plugin/protocol/message/Pubrec.h"
#include "cc_plugin/protocol/message/Pubrel.h"
#include "cc_plugin/protocol/message/Subscribe.h"
#include "cc_plugin/protocol/message/Suback.h"
#include "cc_plugin/protocol/message/Unsubscribe.h"
#include "cc_plugin/protocol/message/Unsuback.h"

namespace mqttsn
{

namespace cc_plugin
{

namespace protocol
{

typedef std::tuple<
    cc_plugin::protocol::message::Advertise,
    cc_plugin::protocol::message::Searchgw,
    cc_plugin::protocol::message::Gwinfo,
    cc_plugin::protocol::message::Connect,
    cc_plugin::protocol::message::Connack,
    cc_plugin::protocol::message::Willtopicreq,
    cc_plugin::protocol::message::Willtopic,
    cc_plugin::protocol::message::Willmsgreq,
    cc_plugin::protocol::message::Willmsg,
    cc_plugin::protocol::message::Register,
    cc_plugin::protocol::message::Regack,
    cc_plugin::protocol::message::Publish,
    cc_plugin::protocol::message::Puback,
    cc_plugin::protocol::message::Pubcomp,
    cc_plugin::protocol::message::Pubrec,
    cc_plugin::protocol::message::Pubrel,
    cc_plugin::protocol::message::Subscribe,
    cc_plugin::protocol::message::Suback,
    cc_plugin::protocol::message::Unsubscribe,
    cc_plugin::protocol::message::Unsuback
> AllMessages;

}  // namespace protocol

}  // namespace cc_plugin

}  // namespace mqttsn




