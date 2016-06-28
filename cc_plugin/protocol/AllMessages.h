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
    cc_plugin::protocol::message::Connect
> AllMessages;

}  // namespace protocol

}  // namespace cc_plugin

}  // namespace mqttsn




