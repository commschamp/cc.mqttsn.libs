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

#include <cstdint>

namespace mqttsn
{

namespace protocol
{

enum MsgTypeId : std::uint8_t
{
    MsgTypeId_ADVERTISE = 0x00,
    MsgTypeId_SEARCHGW = 0x01,
    MsgTypeId_GWINFO = 0x02,
    MsgTypeId_CONNECT = 0x04,
    MsgTypeId_CONNACK = 0x05,
    MsgTypeId_WILLTOPICREQ = 0x06,
    MsgTypeId_WILLTOPIC = 0x07,
    MsgTypeId_WILLMSGREQ = 0x08,
    MsgTypeId_WILLMSG = 0x09,
    MsgTypeId_REGISTER = 0x0a,
    MsgTypeId_REGACK = 0x0b,
    MsgTypeId_PUBLISH = 0x0c,
    MsgTypeId_PUBACK = 0x0d,
    MsgTypeId_PUBCOMP = 0x0e,
    MsgTypeId_PUBREC = 0x0f,
    MsgTypeId_PUBREL = 0x10,
    MsgTypeId_SUBSCRIBE = 0x12,
    MsgTypeId_SUBACK = 0x13,
    MsgTypeId_UNSUBSCRIBE = 0x14,
    MsgTypeId_UNSUBACK = 0x15,
    MsgTypeId_PINGREQ = 0x16,
    MsgTypeId_PINGRESP = 0x17,
    MsgTypeId_DISCONNECT = 0x18,
    MsgTypeId_WILLTOPICUPD = 0x1a,
    MsgTypeId_WILLTOPICRESP = 0x1b,
    MsgTypeId_WILLMSGUPD = 0x1c,
    MsgTypeId_WILLMSGRESP = 0x1d,
};

}  // namespace protocol

}  // namespace mqttsn


