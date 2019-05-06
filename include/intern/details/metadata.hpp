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

namespace intern {
namespace details {

template<typename Traits>
struct [[gnu::packed]] metadata
{
    using size_type = typename Traits::size_type;
    constexpr metadata(size_type l) : _len{l} {}
    metadata(const metadata&) = delete;
    metadata& operator=(const metadata&) = delete;

    size_type _len;
    alignas(2) char _data[0];
};


template<typename Traits>
struct lookup_metadata
{
    using size_type = typename Traits::size_type;
    constexpr lookup_metadata(std::size_t h, size_type l, const char* d)
        : _data{d}
        , _hash{h}
        , _len{l}
    {}
    constexpr lookup_metadata(const lookup_metadata&) = default;
    constexpr lookup_metadata& operator=(const lookup_metadata&) = default;

    constexpr bool operator==(const lookup_metadata& o) const noexcept
    {
        return ( (_hash == o._hash) & (_len == o._len) )
            && !Traits::cmp(_data, o._data, _len);
    }

    friend constexpr std::size_t hash_value(const lookup_metadata& m) noexcept
    {
        return m._hash;
    }

    const char* _data;
    std::size_t _hash;
    size_type _len;
};

}
}

