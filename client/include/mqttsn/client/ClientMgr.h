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


