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

#include <intern/details/metadata.hpp>
#include <intern/details/utils.hpp>
#include <intern/default_string_traits.hpp>
#include <intern/string_far.hpp>
#include <intern/string_sso_tiny.hpp>
#include <intern/string_sso_v1.hpp>
#include <intern/string_sso_v2.hpp>

namespace intern
{

template<typename T> class string_far;
template<size_t S, typename T> class string_sso_v1;

template<typename ITraits, typename Traits = default_string_traits>
class interner
{
public:
    using hasherT = typename ITraits::hasherT;

    using stringF = string_far<Traits>;
    using stringST = string_sso_tiny<Traits>;
    static_assert(sizeof(stringF) == sizeof(char*), "Size problem");
    template<size_t S> using stringS1 = string_sso_v1<S, Traits>;
    template<size_t S> using stringS2 = string_sso_v2<S, Traits>;


    stringF far(const char* s, typename Traits::size_type sz);
    stringF far(const std::string& s)
    {
        return far(s.data(), s.size());
    }
    template<size_t N>
    stringF far(const char (&s)[N]) { return far(s, N - 1); }

    stringST tiny(const char* s, typename Traits::size_type sz);
    stringST tiny(const std::string& s)
    {
        return tiny(s.data(), s.size());
    }
    template<size_t N>
    stringST tiny(const char (&s)[N]) { return tiny(s, N - 1); }

    template<size_t S>
    stringS1<S> sso1(const char* s, typename Traits::size_type sz);
    template<size_t S>
    stringS1<S> sso1(const std::string& s)
    {
        return sso1<S>(s.data(), s.size());
    }
    template<size_t S, size_t N>
    stringS1<S> sso1(const char (&s)[N]) { return sso1<S>(s, N - 1); }


    template<size_t S>
    stringS2<S> sso2(const char* s, typename Traits::size_type sz);
    template<size_t S>
    stringS2<S> sso2(const std::string& s)
    {
        return sso2<S>(s.data(), s.size());
    }
    template<size_t S, size_t N>
    stringS2<S> sso2(const char (&s)[N])
    {
        return sso2<S>(s, N - 1);
    }

private:
    using lookupT = typename ITraits::template lookupT<
        details::lookup_metadata<Traits>, string_far<Traits>>;
    lookupT _lookup;
};

template<typename ITraits, typename Traits>
string_far<Traits> interner<ITraits, Traits>::far(
        const char* s, typename Traits::size_type sz)
{
    // Do we already have it?
    using metadata = details::metadata<Traits>;
    using lookup_metadata = details::lookup_metadata<Traits>;
    lookup_metadata lm{hasherT{}(s, sz), sz, s};
    auto it = _lookup.find(lm);
    if( INTERN__LIKELY( it != _lookup.end() ) )
    {
        return it->second;
    }

    // Add it to the store:
    void* mem = ITraits::allocate(
            sizeof(metadata) + sz + 1, alignof(metadata));
    metadata* m = new(mem) metadata{lm._len};
    Traits::copy(m->_data, s, sz);
    m->_data[sz] = '\0'; // <-- FIXME: do not do if zeroed out

    // Lookup metadata should point to the store
    lm._data = m->_data;

    // And store the link:
    string_far<Traits> res{m->_data};
    _lookup.insert(std::make_pair(lm, res));
    return res;
}

template<typename ITraits, typename Traits>
string_sso_tiny<Traits> interner<ITraits, Traits>::tiny(
        const char* s, typename Traits::size_type sz)
{
    if(sz <= stringST::sso_size)
    {
        return stringST(s, sz);
    }
    return stringST(far(s, sz));

}

template<typename ITraits, typename Traits>
template<size_t S>
string_sso_v1<S, Traits> interner<ITraits, Traits>::sso1(
        const char* s, typename Traits::size_type sz)
{
    static_assert(sizeof(stringS1<S>) == S, "Size problem");
    if(sz <= stringS1<S>::sso_size)
    {
        return stringS1<S>(s, sz);
    }
    return stringS1<S>(far(s, sz).data(), sz);
}

template<typename ITraits, typename Traits>
template<size_t S>
string_sso_v2<S, Traits> interner<ITraits, Traits>::sso2(
        const char* s, typename Traits::size_type sz)
{
    static_assert(sizeof(stringS2<S>) == S, "Size problem");
    if(sz <= stringS2<S>::sso_size)
    {
        return stringS2<S>(s, sz);
    }
    return stringS2<S>(far(s, sz).data(), sz);
}

}

