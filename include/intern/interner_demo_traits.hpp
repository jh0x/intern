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

#include <intern/config.hpp>

#include <cstddef>
#include <functional>
#include <memory>
#ifdef INTERN_HAS_STRING_VIEW
#include <string_view>
#endif
#include <utility>
#include <parallel_hashmap/phmap.h>

namespace intern
{

#ifdef __cpp_exceptions
[[noreturn]] inline void _bad_alloc() {
    throw std::bad_alloc{};
}
#else
[[noreturn]] inline void _bad_alloc() noexcept {
    std::abort();
}
#endif

struct interner_sample_hash
{
    std::size_t operator()(const char* s, std::size_t l) const
    {
#ifdef INTERN_HAS_STRING_VIEW
        auto sv = std::string_view(s, l);
        return std::hash<std::string_view>{}(sv);
#else
        return std::_Hash_impl::hash(s, l);
#endif
    }
};

template<std::size_t N>
struct interner_sample_traits
{
    using hasherT = interner_sample_hash;
    template<typename K, typename V>
    using lookupT = phmap::parallel_flat_hash_map<K, V>;

    static void* allocate(std::size_t s, std::size_t a)
        noexcept(noexcept(_bad_alloc()))
    {
        _off() = ((_off() + (a - 1)) & -a);
        void* ptr = _buf() + _off();
        _off() += s;
        if(_off() > N)
        {
            _bad_alloc();
        }
        return ptr;
    }
    static char* _buf() noexcept
    {
        alignas(8) static char buf[N]{};
        return buf;
    }
    static std::size_t& _off() noexcept
    {
        static std::size_t off;
        return off;
    }

};

}

