//
// Copyright 2016 - 2020 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "comms/comms.h"

namespace mqttsn
{

namespace client
{

namespace details
{

template <typename TClient, typename TOpts, bool THasClientsAllocLimit>
struct ClientAllocatorType;

template <typename TClient, typename TOpts>
struct ClientAllocatorType<TClient, TOpts, true>
{
    typedef comms::util::alloc::InPlacePool<TClient, TOpts::ClientsAllocLimit> Type;
};

template <typename TClient, typename TOpts>
struct ClientAllocatorType<TClient, TOpts, false>
{
    typedef comms::util::alloc::DynMemory<TClient> Type;
};

template <typename TClient, typename TOpts>
using ClientAllocatorTypeT =
    typename ClientAllocatorType<TClient, TOpts, TOpts::HasClientsAllocLimit>::Type;


}  // namespace details

template <typename TClient, typename TClientOpts>
class ClientMgr
{
    typedef details::ClientAllocatorTypeT<TClient, TClientOpts> Alloc;
public:
    typedef TClient Client;

    typedef typename Alloc::Ptr ClientPtr;

    ClientPtr alloc()
    {
        return m_alloc.template alloc<TClient>();
    }

    void free(Client* client) {
        auto ptr = m_alloc.wrap(client);
        static_cast<void>(ptr);
    }

private:
    Alloc m_alloc;
};

}  // namespace client

}  // namespace mqttsn


