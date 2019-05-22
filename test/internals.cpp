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
#include <sstream>
#include <cmath>
#include <cstdint>
#include <string>
#include <typeinfo>

#include <intern/string_io.hpp>

#ifdef __GNUG__
#include <cxxabi.h>
#include <memory>
static std::string demangle(const char* s)
{
    int status = 0;
    auto realname = std::unique_ptr<char,
         decltype(free)*>(abi::__cxa_demangle(s, 0, 0, &status), std::free);
    return realname ? realname.get() : s;
}
#else
static std::string demangle(const char* s)
{
    return s;
}
#endif

static void dump_memory(const void* ptr, std::size_t sz, std::ostream& os)
{
    std::ostringstream o;
    auto h = [&o](auto x, auto width)
    {
        o << std::setfill('0') << std::setw(width) << std::hex
            << static_cast<std::uint64_t>(x);
    };

    constexpr auto full_margin = 20;
    constexpr auto margin = full_margin - 2 * sizeof(void*) - 3;
    const auto banner = std::string(full_margin + 16*4 + 7, '~');

    o << banner << '\n';

    // Headers
    o << std::string(full_margin, ' ');
    for(std::size_t i = 0; i != 16; ++i)
    {
        if(i % 4 == 0) o << ' ';
        o << ' ';
        h(i, 2);
    }
    o << " | ";
    for(std::size_t i = 0; i != 16; ++i)
    {
        o << std::setw(1) << i;
    }
    o << '\n' << banner;

    auto payload = [&o, first = true, fo = 0](auto p, auto i) mutable
    {
        // Print empty spaces for the hex values
        for(size_t j = p % 16; j != 16; ++j)
        {
            if(!j) break;
            o << "   ";
            if(j % 4 == 0) o << ' ';
        }
        o << " | ";
        // Pad elements that do not belong to the printed area 
        const auto pad = first ? (p - i) % 16 : 0;
        for(size_t j = 0; j != pad; ++j)
        {
            o << ' ';
        }
        int off = (i-fo) % 16 ? (i-fo) % 16 : 16;
        for(; off > 0; --off)
        {
            const auto c = *reinterpret_cast<const char*>(p - off);
            o << (::isprint(c) ? c : '.');
        }
        first = false;
        fo = i;
    };

    // Get to the right offset:
    auto p = reinterpret_cast<std::uintptr_t>(ptr);
    if(p % 16)
    {
        o << '\n';
        o << std::string(margin, ' ');
        o << "0x"; h(p & ~15, 16); o << ':';
        std::size_t i;
        for(i = 0; i < p % 16; ++i)
        {
            o << "   "; if(i % 4 == 0) o << ' ';
        }
    }
    // Dump the data + pretty print
    std::size_t i;
    for(i = 0; i != sz; ++i, ++p)
    {
        if(p % 16 == 0)
        {
            if(i) payload(p, i);
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
    payload(p, i);
    o << '\n' << banner << '\n';
    os << o.str();
}


namespace x = intern;

int main()
{
    raze();
    x::interner<interner_traits> i;

    auto print = [](auto generator)
    {
        const auto banner = std::string(120, 'O');
        auto s = generator();
        std::cout << '\n' << banner << '\n';
        std::cout << "My typeinfo: " << demangle(typeid(s).name());
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
    (void)print;

    print([&i]{return i.tiny("0123456");});
    print([&i]{return i.sso1<16>("0123456789ABCDE");});
    print([&i]{return i.sso2<16>("01234");});

    constexpr static auto txt =
        "This is a very very long string. "
        "And it will not fit in the Small String Optimised String. "
        "So we should expect to see it interned!";

    print([&]{return i.far(txt);});
    print([&]{return i.tiny(txt);});
    print([&]{return i.sso1<16>(txt);});
    print([&]{return i.sso2<16>(txt);});

//    char data[] = "QWERTYUIOP";
//    std::cout << (void*) data << std::endl;
//    dump_memory(data, 3, std::cout);
//    dump_memory(data+1, 3, std::cout);
}
