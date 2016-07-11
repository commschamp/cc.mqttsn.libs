//
// Copyright 2014 (C). Alex Robenko. All rights reserved.
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


// This function is required by common startup code
#include <type_traits>

extern "C"
void interruptHandler()
{
}

extern "C"
{
void* mqttsn_client_new();
void mqttsn_client_free(void* client);
void mqttsn_client_process_data(void* client, const unsigned char** from, unsigned len);
void mqttsn_client_tick(void* client, unsigned ms);
}

int main(int argc, const char** argv)
{
    static_cast<void>(argc);
    static_cast<void>(argv);

    auto* client = mqttsn_client_new();

    static const unsigned char Seq[] = {0x00, 0xf0};
    static const std::size_t SeqSize = std::extent<decltype(Seq)>::value;

    const unsigned char* from = &Seq[0];
    mqttsn_client_process_data(client, &from, SeqSize);
    mqttsn_client_tick(client, 10);
    mqttsn_client_free(client);
    while (true) {};
    return 0;
}
