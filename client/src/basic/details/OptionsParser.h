//
// Copyright 2016 - 2020 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "option.h"

namespace mqttsn
{

namespace client
{

namespace details
{

template <typename... TOptions>
class OptionsParser;

template <>
class OptionsParser<>
{
public:
    static const bool HasClientsAllocLimit = false;
    static const bool HasTrackedGatewaysLimit = false;
    static const bool HasRegisteredTopicsLimit = false;
    static const bool HasGwAddStaticStorageSize = false;
    static const bool HasClientIdStaticStorageSize = false;
    static const bool HasTopicNameStaticStorageSize = false;
    static const bool HasMessageDataStaticStorageSize = false;    
};

template <std::size_t TLimit, typename... TOptions>
class OptionsParser<
    mqttsn::client::option::ClientsAllocLimit<TLimit>,
    TOptions...> : public OptionsParser<TOptions...>
{
    typedef mqttsn::client::option::ClientsAllocLimit<TLimit> Option;
public:
    static const bool HasClientsAllocLimit = true;
    static const std::size_t ClientsAllocLimit = Option::Value;
};

template <std::size_t TLimit, typename... TOptions>
class OptionsParser<
    mqttsn::client::option::TrackedGatewaysLimit<TLimit>,
    TOptions...> : public OptionsParser<TOptions...>
{
    typedef mqttsn::client::option::TrackedGatewaysLimit<TLimit> Option;
public:
    static const bool HasTrackedGatewaysLimit = true;
    static const std::size_t TrackedGatewaysLimit = Option::Value;
};

template <std::size_t TLimit, typename... TOptions>
class OptionsParser<
    mqttsn::client::option::RegisteredTopicsLimit<TLimit>,
    TOptions...> : public OptionsParser<TOptions...>
{
    typedef mqttsn::client::option::RegisteredTopicsLimit<TLimit> Option;
public:
    static const bool HasRegisteredTopicsLimit = true;
    static const std::size_t RegisteredTopicsLimit = Option::Value;
};

template <std::size_t TSize, typename... TOptions>
class OptionsParser<
    mqttsn::client::option::GwAddStaticStorageSize<TSize>,
    TOptions...> : public OptionsParser<TOptions...>
{
    typedef mqttsn::client::option::GwAddStaticStorageSize<TSize> Option;
public:
    static const bool HasGwAddStaticStorageSize = true;
    static const std::size_t GwAddStaticStorageSize = Option::Value;
};

template <std::size_t TSize, typename... TOptions>
class OptionsParser<
    mqttsn::client::option::ClientIdStaticStorageSize<TSize>,
    TOptions...> : public OptionsParser<TOptions...>
{
    typedef mqttsn::client::option::ClientIdStaticStorageSize<TSize> Option;
public:
    static const bool HasClientIdStaticStorageSize = true;
    static const std::size_t ClientIdStaticStorageSize = Option::Value;
};

template <std::size_t TSize, typename... TOptions>
class OptionsParser<
    mqttsn::client::option::TopicNameStaticStorageSize<TSize>,
    TOptions...> : public OptionsParser<TOptions...>
{
    typedef mqttsn::client::option::TopicNameStaticStorageSize<TSize> Option;
public:
    static const bool HasTopicNameStaticStorageSize = true;
    static const std::size_t TopicNameStaticStorageSize = Option::Value;
};

template <std::size_t TSize, typename... TOptions>
class OptionsParser<
    mqttsn::client::option::MessageDataStaticStorageSize<TSize>,
    TOptions...> : public OptionsParser<TOptions...>
{
    typedef mqttsn::client::option::MessageDataStaticStorageSize<TSize> Option;
public:
    static const bool HasMessageDataStaticStorageSize = true;
    static const std::size_t MessageDataStaticStorageSize = Option::Value;
};


template <typename... TTupleOptions, typename... TOptions>
class OptionsParser<
    std::tuple<TTupleOptions...>,
    TOptions...> : public OptionsParser<TTupleOptions..., TOptions...>
{
};


}  // namespace details

}  // namespace client

}  // namespace mqttsn

