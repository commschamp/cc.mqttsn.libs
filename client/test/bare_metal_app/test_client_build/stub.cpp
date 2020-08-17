//
// Copyright 2014 - 2020 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <cstdint>

// This function may be required by _mainCRTSturtup when using a particular compiler
extern "C"
void exit()
{
    while (true) {}
}

void operator delete(void *)
{
}

// Required when there is pure virtual function
extern "C" void __cxa_pure_virtual()
{
    while (true) {}
}

extern "C" int __aeabi_atexit(
    void *object,
    void (*destructor)(void *),
    void *dso_handle)
{
    static_cast<void>(object);
    static_cast<void>(destructor);
    static_cast<void>(dso_handle);
    return 0;
}

// The pointer to this variable will be passed to __aeabi_atexit
void* __dso_handle = nullptr;

extern "C"
void* memset(void* dest, int ch, std::size_t count)
{
    static_cast<void>(dest);
    static_cast<void>(ch);
    static_cast<void>(count);
    return nullptr;
}

extern "C"
void* memmove( void* dest, const void* src, std::size_t count )
{
    static_cast<void>(dest);
    static_cast<void>(src);
    static_cast<void>(count);
    return nullptr;
}

extern "C"
void* memcpy( void* dest, const void* src, std::size_t count )
{
    static_cast<void>(dest);
    static_cast<void>(src);
    static_cast<void>(count);
    return nullptr;
}

extern "C"
int memcmp(const void*, const void*, std::size_t)
{
    return 0;
}

extern "C"
std::size_t strlen(const char*)
{
    return 0U;
}

extern "C"
void  __aeabi_uidiv()
{
}



