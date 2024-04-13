//
// Copyright 2016 - 2024 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

/// @file
/// @brief Contains interface of cc_mqttsn_gateway::Gateway class.

#pragma once

#include <memory>
#include <cstdint>
#include <functional>

namespace cc_mqttsn_gateway
{

class GatewayImpl;
/// @brief Interface for @b Gateway entity.
/// @details The responsibility of the @b Gateway object is to advertise
///     its presence to all MQTT-SN clients on the network. It requests the
///     driving code to send serialised @b ADVERTISE message periodically, using
///     provided callback.
class Gateway
{
public:
    /// @brief Default constructor
    Gateway();

    /// @brief Destructor
    ~Gateway();

    /// @brief Type of the callback used to request time measurement.
    /// @details The driving code is expected to invoke tick() member function
    ///     when the requested time expires. The callback is set using
    ///     setNextTickProgramReqCb() member function.
    /// @param[in] ms Number of @b milliseconds to measure.
    typedef std::function<void (unsigned ms)> NextTickProgramReqCb;

    /// @brief Type of callback used to request send the serialised @b ADVERTISE message.
    /// @details According to MQTT-SN protocol, the @b ADVERTISE message needs
    ///     to be @b broadcasted. The driving code is responsible to broadcast
    ///     the requested message to all possible clients on the network. The
    ///     callback is set using setSendDataReqCb() member function.
    ///
    ///     The buffer containing serialised @b ADVERTISE message belongs
    ///     to the internal data structures of the @ref Gateway, and may be
    ///     tampered with when the callback function returns. The driving code
    ///     may be required to copy contents of the buffer to its internal data
    ///     structures and preserve it until send over I/O link operation is
    ///     complete.
    /// @param[in] buf Pointer to buffer containing serialised @b ADVERTISE
    ///     message.
    /// @param[in] bufSize Number of bytes in the buffer.
    typedef std::function<void (const std::uint8_t* buf, std::size_t bufSize)> SendDataReqCb;

    /// @brief Set the callback to be invoked when new time measurement is required.
    /// @details This is a must have callback, without it the object can not
    ///     be started (see start()).
    /// @param[in] func R-value reference to the callback object
    void setNextTickProgramReqCb(NextTickProgramReqCb&& func);

    /// @brief Set the callback to be invoked when new @b ADVERTISE message needs
    ///     to be broadcasted.
    /// @details This is a must have callback, without it the object can not
    ///     be started (see start()).
    /// @param[in] func R-value reference to the callback object
    void setSendDataReqCb(SendDataReqCb&& func);

    /// @brief Set period after which @b ADVERTISE message needs to be constatly sent.
    /// @details The value must be greater than 0. If the value is not set, the
    ///     object can not be started (see start()).
    /// @param[in] value Advertise period in @b seconds.
    void setAdvertisePeriod(std::uint16_t value);

    /// @brief Set gateway numeric ID to be advertised.
    /// @details If not set, default value 0 is assumed.
    /// @param[in] value Gateway numeric ID.
    void setGatewayId(std::uint8_t value);

    /// @brief Start the operation of the object
    /// @details The function will check whether all callbacks as well as
    ///     advertise period have been properly set. It  will also immediately
    ///     request to send the serialised @b ADVERTISE message and to re-program
    ///     the timer for the tick() after set advertise period.
    bool start();

    /// @brief Stop the operation of the object
    void stop();

    /// @brief Notify the @ref Gateway object about requested time period expiry.
    /// @details This function needs to be called from the driving code after
    ///     the requested time measurement has expired. The call will
    ///     cause invocation of send data request callback as well as new
    ///     time measurement request callback.
    void tick();

private:
    std::unique_ptr<GatewayImpl> m_pImpl;
};

}  // namespace cc_mqttsn_gateway


