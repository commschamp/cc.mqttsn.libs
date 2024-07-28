//
// Copyright 2024 - 2024 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Config.h"
#include "ProtocolOptions.h"

#include "cc_mqttsn/Message.h"
#include "cc_mqttsn/Version.h"
#include "cc_mqttsn/frame/Frame.h"
#include "cc_mqttsn/input/AllMessages.h"
#include "cc_mqttsn/input/ProtClientInputMessages.h"

#include "comms/GenericHandler.h"

#include <cstdint>

static_assert(COMMS_MAKE_VERSION(2, 7, 1) <= cc_mqttsn::version(),
    "The version of cc.mqttsn.generated library is too old");

namespace cc_mqttsn_client
{

class ProtMsgHandler;

using ProtMessage = cc_mqttsn::Message<
    comms::option::app::ReadIterator<const std::uint8_t*>,
    comms::option::app::WriteIterator<std::uint8_t*>,
    comms::option::app::LengthInfoInterface,
    comms::option::app::IdInfoInterface,
    comms::option::app::Handler<ProtMsgHandler>
>;

CC_MQTTSN_ALIASES_FOR_ALL_MESSAGES(, Msg, ProtMessage, ProtocolOptions)

using ProtInputMessages =
    std::tuple<
#if CC_MQTTSN_CLIENT_HAS_GATEWAY_DISCOVERY    
        cc_mqttsn::message::Advertise<ProtMessage, ProtocolOptions>,
        cc_mqttsn::message::Searchgw<ProtMessage, ProtocolOptions>,
        cc_mqttsn::message::Gwinfo<ProtMessage, ProtocolOptions>,
#endif // CC_MQTTSN_CLIENT_HAS_GATEWAY_DISCOVERY        
        cc_mqttsn::message::Connack<ProtMessage, ProtocolOptions>,
#if CC_MQTTSN_CLIENT_HAS_WILL        
        cc_mqttsn::message::Willtopicreq<ProtMessage, ProtocolOptions>,
        cc_mqttsn::message::Willmsgreq<ProtMessage, ProtocolOptions>,
#endif // #if CC_MQTTSN_CLIENT_HAS_WILL
        cc_mqttsn::message::Register<ProtMessage, ProtocolOptions>,
        cc_mqttsn::message::Regack<ProtMessage, ProtocolOptions>,
        cc_mqttsn::message::Publish<ProtMessage, ProtocolOptions>,
        cc_mqttsn::message::Puback<ProtMessage, ProtocolOptions>,
#if CC_MQTTSN_CLIENT_MAX_QOS > 1                
        cc_mqttsn::message::Pubcomp<ProtMessage, ProtocolOptions>,
        cc_mqttsn::message::Pubrec<ProtMessage, ProtocolOptions>,
        cc_mqttsn::message::Pubrel<ProtMessage, ProtocolOptions>,
#endif // #if CC_MQTTSN_CLIENT_MAX_QOS > 1        
        cc_mqttsn::message::Suback<ProtMessage, ProtocolOptions>,
        cc_mqttsn::message::Unsuback<ProtMessage, ProtocolOptions>,
        cc_mqttsn::message::Pingreq<ProtMessage, ProtocolOptions>,
        cc_mqttsn::message::Pingresp<ProtMessage, ProtocolOptions>,
        cc_mqttsn::message::Disconnect<ProtMessage, ProtocolOptions>
#if CC_MQTTSN_CLIENT_HAS_WILL        
        ,
        cc_mqttsn::message::Willtopicresp<ProtMessage, ProtocolOptions>,
        cc_mqttsn::message::Willmsgresp<ProtMessage, ProtocolOptions>
#endif // #if CC_MQTTSN_CLIENT_HAS_WILL        
    >;

using ProtFrame = cc_mqttsn::frame::Frame<ProtMessage, ProtInputMessages, ProtocolOptions>;
using ProtMsgPtr = ProtFrame::MsgPtr;

class ProtMsgHandler : public comms::GenericHandler<ProtMessage, ProtInputMessages>
{
protected:
    ProtMsgHandler() = default;
    ~ProtMsgHandler() noexcept = default;
};

} // namespace cc_mqttsn_client
