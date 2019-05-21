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

#include <string>
#include <cstring>
#include <string_view>
#include <parallel_hashmap/phmap.h>

#include <intern/interner.hpp>

///////////////////////////////////////////////////////////////////////

// We need some form of a hash - nothing fancy here:
struct hash_sv
{
    std::size_t operator()(const char* s, std::size_t l) const
    {
        auto sv = std::string_view(s, l);
        return std::hash<std::string_view>{}(sv);
    }
};

// And we want string_common to conform...
namespace intern
{
template<typename T, typename Traits>
std::size_t hash_value(const intern::details::string_common<T, Traits>& s)
{
    return hash_sv{}(s.data(), s.size());
}
}

// Chunk of memory for "allocating" strings
constexpr static auto kBufferSize = 131072;
alignas(8) static char gBuffer[kBufferSize]{};
static std::size_t gOffset = 0;
static void raze()
{
    std::memset(gBuffer, '\0', gOffset);
    gOffset = 0;
}

#ifdef __cpp_exceptions
[[noreturn]] static void _bad_alloc() {
    throw std::bad_alloc{};
}
#else
[[noreturn]] static void _bad_alloc() noexcept {
    std::abort();
}
#endif

// Minimal interner configuration
struct interner_traits
{
    using hasherT = hash_sv;
    template<typename K, typename V>
    using lookupT = phmap::parallel_flat_hash_map<K, V>;

    static void* allocate(std::size_t s, std::size_t a)
        noexcept(noexcept(_bad_alloc()))
    {
        gOffset = ((gOffset + (a - 1)) & -a);
        void* ptr = gBuffer + gOffset;
        gOffset += s;
        if(gOffset > kBufferSize)
        {
            _bad_alloc();
        }
        return ptr;
    }
};

