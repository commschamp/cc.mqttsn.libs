//
// Copyright 2016 - 2024 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "SessionOp.h"

namespace cc_mqttsn_gateway
{

void SessionOp::sendDisconnectToClient()
{
    DisconnectMsg_SN msg;
    auto& fields = msg.fields();
    auto& durationField = std::get<decltype(msg)::FieldIdx_duration>(fields);
    durationField.setMode(comms::field::OptionalMode::Missing);
    sendToClient(msg);

}

}  // namespace cc_mqttsn_gateway
