//
// Copyright 2016 - 2023 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

/// @file
/// @brief Contains interface of cc_mqttsn_gateway::Session class.

#pragma once

#include <memory>
#include <functional>
#include <cstdint>
#include <vector>
#include <string>
#include <utility>

namespace cc_mqttsn_gateway
{

class SessionImpl;

/// @brief Interface for @b Session entity.
/// @details The responsibility of the @b Session object is to manage and forward
///     traffic of messages between single MQTT-SN client and broker.
class Session
{
public:

    /// @brief Type for buffer of binary data.
    typedef std::vector<std::uint8_t> BinaryData;

    /// @brief Type of authentication information.
    /// @details The first element of the pair is @b username, and the
    ///     second element of the pair is binary @b password.
    typedef std::pair<std::string, BinaryData> AuthInfo;

    /// @brief Type of callback, used to request new time measurement.
    /// @details When the requested time is due, the driving code is expected
    ///     to call tick() member function.
    /// @param[in] value Number of @b milliseconds to measure.
    typedef std::function<void (unsigned value)> NextTickProgramReqCb;

    /// @brief Type of callback, used to cancel existing time measurement.
    /// @details When invoked the existing time measurement needs to be cancelled.
    ///     The function also needs to return amount of @b milliseconds elapsed
    ///     since last timer programming request.
    /// @return Number of elapsed @b milliseconds since last timer programming
    ///     request.
    typedef std::function<unsigned ()> CancelTickWaitReqCb;

    /// @brief Type of callback, used to request delivery of serialised message
    ///     to the client or broker.
    /// @param[in] buf Buffer containing serialised message.
    /// @param[in] bufSize Number of bytes in the buffer
    typedef std::function<void (const std::uint8_t* buf, std::size_t bufSize)> SendDataReqCb;

    /// @brief Type of callback, used to request session termination.
    /// @details When the callback is invoked, the driving code must flush
    ///     all the previously sent messages to appropriate I/O links and
    ///     delete this session object.
    typedef std::function<void ()> TerminationReqCb;

    /// @brief Type of callback used to request reconnection to the broker.
    /// @details When the callback is invoked, the driving code must close
    ///     existing TCP/IP connection to the broker and create a new one.
    typedef std::function<void ()> BrokerReconnectReqCb;

    /// @brief Type of callback used to report client ID of the newly connected
    ///     MQTT-SN client.
    /// @details The callback can be used to provide additional client specific
    ///     information, such as predefined topic IDs.
    /// @param[in] clientId Client ID
    typedef std::function<void (const std::string& clientId)> ClientConnectedReportCb;

    /// @brief Type of callback used to request authentication information of
    ///     the client that is trying connect.
    /// @param[in] clientId Client ID
    /// @return Authentication information
    typedef std::function<AuthInfo (const std::string& clientId)> AuthInfoReqCb;

    /// @brief Default constructor
    Session();

    /// @brief Destructor
    ~Session();

    /// @brief Set the callback to be invoked when new time measurement is required.
    /// @details This is a must have callback, without it the object can not
    ///     be started (see start()).
    /// @param[in] func R-value reference to the callback object
    void setNextTickProgramReqCb(NextTickProgramReqCb&& func);

    /// @brief Set the callback to be invoked when previously requested time
    ///     measurement needs to be cancelled.
    /// @details This is a must have callback, without it the object can not
    ///     be started (see start()).
    /// @param[in] func R-value reference to the callback object
    void setCancelTickWaitReqCb(CancelTickWaitReqCb&& func);

    /// @brief Set the callback to be invoked when new data needs to be sent
    ///     to the client.
    /// @details This is a must have callback, without it the object can not
    ///     be started (see start()).
    /// @param[in] func R-value reference to the callback object
    void setSendDataClientReqCb(SendDataReqCb&& func);

    /// @brief Set the callback to be invoked when new data needs to be sent
    ///     to the broker.
    /// @details This is a must have callback, without it the object can not
    ///     be started (see start()).
    /// @param[in] func R-value reference to the callback object
    void setSendDataBrokerReqCb(SendDataReqCb&& func);

    /// @brief Set the callback to be invoked when the session needs to be
    ///     terminated and this @ref Session object deleted.
    /// @details This is a must have callback, without it the object can not
    ///     be started (see start()).
    /// @param[in] func R-value reference to the callback object
    void setTerminationReqCb(TerminationReqCb&& func);

    /// @brief Set the callback to be invoked when the session needs to close
    ///     existing TCP/IP connection to the broker and open a new one.
    /// @details This is a must have callback, without it the object can not
    ///     be started (see start()).
    /// @param[in] func R-value reference to the callback object
    void setBrokerReconnectReqCb(BrokerReconnectReqCb&& func);

    /// @brief Set the callback to be invoked when MQTT-SN client is successfully
    ///     connected to the broker.
    /// @details This is an optional callback. It can be used when there is a
    ///     need to provide client specific configuration, such as predefined
    ///     topic IDs, valid only for specific client.
    /// @param[in] func R-value reference to the callback object
    void setClientConnectedReportCb(ClientConnectedReportCb&& func);

    /// @brief Set the callback to be used to request authentication information
    ///     for specific client.
    /// @details This is an optional callback. It can be used when there is a
    ///     need to provide authentication details (username/password) for
    ///     specific clients.
    /// @param[in] func R-value reference to the callback object
    void setAuthInfoReqCb(AuthInfoReqCb&& func);

    /// @brief Set gateway numeric ID to be reported when requested.
    /// @details If not set, default value 0 is assumed.
    /// @param[in] value Gateway numeric ID.
    void setGatewayId(std::uint8_t value);

    /// @brief Set retry period to wait between resending unacknowledged message
    ///     to the client and/or broker.
    /// @details Some messages, may require acknowledgement by
    ///     the client and/or broker. The delay (in seconds) between such
    ///     attempts to resend the message may be specified using this function.
    ///     The default value is @b 10 seconds.
    /// @param[in] value Number of @b seconds to wait before making an attempt to resend.
    void setRetryPeriod(unsigned value);

    /// @brief Set number of retry attempts to perform before abandoning attempt
    ///     to send unacknowledged message.
    /// @details Some messages, may require acknowledgement by
    ///     the client and/or broker. The amount of retry attempts before
    ///     abandoning the attempt to deliver the message may be specified
    ///     using this function. The default value is @b 3.
    /// @param[in] value Number of retry attempts.
    void setRetryCount(unsigned value);

    /// @brief Provide limit to number pending messages being accumulated for
    ///     the sleeping client.
    /// @details When client is known to be in "ASLEEP" state, the gateway must
    ///     accumulate all the messages the broker sends until client wakes up
    ///     or explicitly requests to send them. This function may be used
    ///     to limit amount of such messages to prevent acquiring lots of
    ///     RAM by the gateway application.
    /// @param[in] value Max number of pending messages.
    void setSleepingClientMsgLimit(std::size_t value);

    /// @brief Provide default client ID for clients that report empty one
    ///     in their attempt to connect.
    /// @param[in] value Default client ID string.
    void setDefaultClientId(const std::string& value);

    /// @brief Provide default "keep alive" period for "publish only" clients,
    ///     that do not make an attempt to connect to the gateway.
    /// @details MQTT-SN protocol allows "publish only" clients that don't
    ///     make any attempt to connect to the gateway/broker and send all
    ///     their messages with QoS=-1. In this case, the gateway must connect
    ///     to the broker on behalf of the "publish only" client. Such connection
    ///     attempt requires to specify "keep alive" period. Use this function
    ///     to set the value.
    /// @param[in] value Max number of seconds between messages the "publish only"
    ///     client is going to send.
    void setPubOnlyKeepAlive(std::uint16_t value);

    /// @brief Start this object's operation.
    /// @details The function will check whether all necessary callbacks have been
    ///     set.
    /// @return true if the operation has been successfully started, false in
    ///     case some necessary callback hasn't been set.
    bool start();

    /// @brief Stop the operation of the object
    void stop();

    /// @brief Check whether the object's operation has been successfull started.
    /// @return true/false
    bool isRunning() const;

    /// @brief Notify the @ref Session object about requested time period expiry.
    /// @details This function needs to be called from the driving code after
    ///     the requested time measurement has expired.
    void tick();

    /// @brief Provide data received from the client for processing.
    /// @details This call may cause invocation of some callbacks, such as
    ///     request to cancel the currently running time measurement,
    ///     send new message(s) and/or (re)start time measurement.
    /// @param[in] buf Pointer to the buffer of data to process.
    /// @param[in] len Number of bytes in the data buffer.
    /// @return Number of processed bytes.
    /// @note The function returns number of bytes that were actually consumed, and
    ///     can be removed from the holding buffer.
    std::size_t dataFromClient(const std::uint8_t* buf, std::size_t len);

    /// @brief Provide data received from the broker for processing.
    /// @details This call may cause invocation of some callbacks, such as
    ///     request to cancel the currently running time measurement,
    ///     send new message(s) and/or (re)start time measurement.
    /// @param[in] buf Pointer to the buffer of data to process.
    /// @param[in] len Number of bytes in the data buffer.
    /// @return Number of processed bytes.
    /// @note The function returns number of bytes that were actually consumed, and
    ///     can be removed from the holding buffer.
    std::size_t dataFromBroker(const std::uint8_t* buf, std::size_t len);

    /// @brief Notify the @ref Session object about broker being connected / disconnected
    /// @details The report of broker being connected or disconnected must
    ///     be performed only when the session's operation has been successfully
    ///     started (see start()). Otherwise the call to this function gets
    ///     ignored.
    /// @param[in] conneted Connection status - @b true means connected, @b false disconnected.
    void setBrokerConnected(bool connected);

    /// @brief Add predefined topic string and ID information.
    /// @param[in] topic Topic string
    /// @param[in] topicId Numeric topic ID.
    /// @return success/failure status
    bool addPredefinedTopic(const std::string& topic, std::uint16_t topicId);

    /// @brief Limit range of topic IDs allocated for newly registered topics.
    /// @param[in] minVal Min topic ID.
    /// @param[in] maxVal Max topic ID.
    /// @return success/failure status
    bool setTopicIdAllocationRange(std::uint16_t minVal, std::uint16_t maxVal);

private:
    std::unique_ptr<SessionImpl> m_pImpl;
};

}  // namespace cc_mqttsn_gateway

