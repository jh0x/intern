#pragma once
// The MIT License (MIT)
//
// Copyright (c) 2019 jh0x
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <intern/interner.hpp>
#include <intern/interner_demo_traits.hpp>

#include <string>
#include <cstring>
#include <parallel_hashmap/phmap.h>


///////////////////////////////////////////////////////////////////////

using hash_sv = intern::interner_sample_hash;

// And we want string_common to conform...
namespace intern
{
template<typename T, typename Traits>
std::size_t hash_value(const intern::details::string_common<T, Traits>& s)
{
    return hash_sv{}(s.data(), s.size());
}
}

constexpr static auto kBufferSize = 1 << 20;

// Minimal interner configuration
struct interner_traits1
{
    using hasherT = hash_sv;
    template<typename K, typename V>
    using lookupT = phmap::parallel_flat_hash_map<K, V>;

    static void* allocate(std::size_t s, std::size_t a)
        noexcept(noexcept(intern::_bad_alloc()))
    {
        _off() = ((_off() + (a - 1)) & -a);
        void* ptr = _buf() + _off();
        _off() += s;
        if(_off() > kBufferSize)
        {
            intern::_bad_alloc();
        }
        return ptr;
    }
    static void raze() noexcept
    {
        std::memset(_buf(), '\0', kBufferSize);
        _off() = 0;
    }
    static char* _buf() noexcept
    {
        alignas(8) static char buf[kBufferSize]{};
        return buf;
    }
    static std::size_t& _off() noexcept
    {
        static std::size_t off;
        return off;
    }
};

struct interner_traits2
{
    using hasherT = hash_sv;
    template<typename K, typename V>
    using lookupT = phmap::parallel_flat_hash_map<K, V>;

    static void* allocate(std::size_t s, std::size_t a)
        noexcept(noexcept(intern::_bad_alloc()))
    {
        _off() = ((_off() + (a - 1)) & -a);
        void* ptr = _buf() + _off();
        _off() += s;
        if(_off() > kBufferSize)
        {
            intern::_bad_alloc();
        }
        return ptr;
    }
    static void raze() noexcept
    {
        std::memset(_buf(), '\0', kBufferSize);
        _off() = 0;
    }
    static char* _buf()
    {
        static std::unique_ptr<char[]> buf(new char[kBufferSize]);
        return buf.get();
    }
    static std::size_t& _off() noexcept
    {
        static std::size_t off;
        return off;
    }
};

