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

#include "mqttsn/client/option.h"

namespace mqttsn
{

namespace client
{

namespace details
{

template <typename... TOptions>
class OptionsParser;

template <>
struct OptionsParser<>
{
    static const bool HasClientsAllocLimit = false;
};

template <std::size_t TLimit, typename... TOptions>
class OptionsParser<
    mqttsn::client::option::ClientAllocLimit<TLimit>,
    TOptions...> : public OptionsParser<TOptions...>
{
    typedef mqttsn::client::option::ClientAllocLimit<TLimit> Option;
public:
    static const bool HasClientsAllocLimit = true;
    static const std::size_t ClientsAllocLimit = Option::Value;
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

