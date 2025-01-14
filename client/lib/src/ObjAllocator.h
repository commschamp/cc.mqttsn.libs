//
// Copyright 2024 - 2025 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "comms/util/alloc.h"
#include "comms/util/type_traits.h"

namespace cc_mqttsn_client
{

template <typename TObj, unsigned TLimit>
class ObjAllocator
{
    template <typename ...>
    using DynMemoryAlloc = comms::util::alloc::DynMemory<TObj>;

    template <typename ...>
    using InPlaceAlloc = comms::util::alloc::InPlacePool<TObj, TLimit>;

    template <typename... TParams>
    using Alloc = 
        typename comms::util::LazyShallowConditional<
            TLimit == 0U
        >::template Type<
            DynMemoryAlloc,
            InPlaceAlloc
        >;

    using AllocType = Alloc<>;
        
public:
    using Ptr = typename AllocType::Ptr;

    template <typename... TArgs>
    Ptr alloc(TArgs&&... args)
    {
        return m_alloc.template alloc<TObj>(std::forward<TArgs>(args)...);
    }

    void free(TObj* client) {
        auto ptr = m_alloc.wrap(client);
        static_cast<void>(ptr);
    }

private:
    AllocType m_alloc;
};

} // namespace cc_mqttsn_client
