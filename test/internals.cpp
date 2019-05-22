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

#include "common.h"
#include <climits>

#include <iostream>
#include <iomanip>
#include <cmath>
#include <cstdint>
#include <string>
#include <typeinfo>

#include <intern/string_io.hpp>

static void dump_memory(const void* ptr, std::size_t sz, std::ostream& o)
{
    auto h = [&o](auto x, auto width)
    {
        auto f(o.flags());
        o << std::setfill('0') << std::setw(width) << std::hex
            << static_cast<std::uint64_t>(x);
        o.flags(f);
    };

    constexpr auto full_margin = 20;
    constexpr auto margin = full_margin - 2 * sizeof(void*) - 3;
    const auto banner = std::string(full_margin + 16*3 + 4, '~');

    o << banner << '\n';

    // Headers
    o << std::string(full_margin, ' ');
    for(std::size_t i = 0; i != 16; ++i)
    {
        if(i % 4 == 0) o << ' ';
        o << ' ';
        h(i, 2);
    }
    o << '\n' << banner;
    // Get to the right offset:
    auto p = reinterpret_cast<std::uintptr_t>(ptr);
    if(p % 16)
    {
        o << '\n';
        o << std::string(margin, ' ');
        o << "0x"; h(p & ~15, 16); o << ':';
        for(std::size_t i = 0; i < p % 16; ++i)
        {
            o << "   "; if(i % 4 == 0) o << ' ';
        }
    }
    // Dump the data + pretty print
    for(std::size_t i = 0; i != sz; ++i, ++p)
    {
        if(p % 16 == 0)
        {
            o << '\n';
            o << std::string(margin, ' ');
            o << "0x"; h(p, 16); o << ':';
        }
        if(p % 4 == 0)
        {
            o << ' ';
        }
        o << ' ';

        h(*reinterpret_cast<unsigned char*>(p), 2);
    }
    o << '\n' << banner << '\n';
}

namespace x = intern;

int main()
{
    raze();
    x::interner<interner_traits> i;

    auto print = [](auto generator)
    {
        const auto banner = std::string(72, 'O');
        auto s = generator();
        std::cout << '\n' << banner << '\n';
        std::cout << "My typeinfo: " << typeid(s).name();
        std::cout << '\n' << banner << '\n';
        std::cout
            << "String: '" << s << "'"
            << "\n\tsmall=" << s.small() << ", sso_size=" << decltype(s)::sso_size
            << "\n\tsize=" << s.size()
            << "\n\tsizeof(s)=" << sizeof(s)
            << "\n\t&s=" << &s
            << '\n';
        dump_memory(&s, sizeof(s), std::cout);
        if(!s.small())
        {
            std::cout << "\tFar address=" << (void*)s.data() << '\n';
            std::cout << "\tFar data:\n";
            const auto meta_sz =
                sizeof(x::details::metadata<decltype(i)::StringTraits>);
            const char* meta_begin = s.data() - meta_sz;
            dump_memory(meta_begin, s.size() + meta_sz, std::cout);
        }
        std::cout << banner << "\n\n\n";
    };

    print([&i]{return i.tiny("0123456");});
    print([&i]{return i.sso1<16>("0123456789ABCDE");});
    print([&i]{return i.sso2<16>("01234");});

    constexpr static auto txt = "This is a very very long string";
    print([&]{return i.far(txt);});
    print([&]{return i.tiny(txt);});
    print([&]{return i.sso1<16>(txt);});
    print([&]{return i.sso2<16>(txt);});
}
