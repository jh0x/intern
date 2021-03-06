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

#include <doctest/doctest.h>

#include "common.h"

#include <algorithm>
#include <fstream>
#include <vector>
#include <string>

#include <cstring>

#include <intern/string_io.hpp>
#include <intern/string_ops.hpp>
#include <intern/interner.hpp>

namespace x = intern;

///////////////////////////////////////////////////////////////////////

static std::vector<std::string> get_words()
{
    std::vector<std::string> normal;
    normal.reserve(13000);
    std::ifstream f("test/words.txt", std::fstream::in);
    std::string s;
    while(std::getline(f, s))
    {
        normal.push_back(s);
    }
    return normal;
}
static const std::vector<std::string> words = get_words();

///////////////////////////////////////////////////////////////////////
// Test template
TEST_CASE_TEMPLATE_DEFINE("interner", T, test_id)
{
    using interner_traits = typename T::interner_traits;
    interner_traits::raze();

    x::interner<interner_traits, typename T::string_traits> i;

    using stringT = decltype(T::intern(i, ""));
    constexpr bool can_be_small = stringT::sso_size;
    {
        for(auto& s: words)
        {
            auto is = T::intern(i, s);
            REQUIRE(s.size() == is.size());
            REQUIRE(s.length() == is.length());
            REQUIRE(s.empty() == is.empty());
            REQUIRE(is.small()
                    == (can_be_small && is.size() <= stringT::sso_size));
            REQUIRE(0 == std::strncmp(s.c_str(), is.c_str(), s.size()));
            for(std::size_t q = 0; q != s.size(); ++q)
            {
                REQUIRE(s[q] == is[q]);
            }
            REQUIRE(std::equal(
                        s.begin(), s.end(),
                        is.begin(), is.end()));
            REQUIRE(std::equal(
                        s.begin(), s.end(),
                        is.cbegin(), is.cend()));
            REQUIRE(std::equal(
                        s.rbegin(), s.rend(),
                        is.rbegin(), is.rend()));
            REQUIRE(std::equal(
                        s.rbegin(), s.rend(),
                        is.crbegin(), is.crend()));
        }
    }
    {
        std::vector<stringT> interned;
        interned.reserve(13000);
        for(auto& s: words)
        {
            interned.push_back(T::intern(i, s));
            REQUIRE(interned.back() == s);
        }

        REQUIRE(std::equal(
                    words.begin(), words.end(),
                    interned.begin(), interned.end(),
                    [](const auto& a, const auto& b) { return a == b; }
                    ));

        auto tmp = words;
        std::sort(tmp.begin(), tmp.end());
        std::sort(interned.begin(), interned.end());

        REQUIRE(std::equal(
                    tmp.begin(), tmp.end(),
                    interned.begin(), interned.end(),
                    [](const auto& a, const auto& b) { return a == b; }
                    ));

    }
    {
        phmap::parallel_flat_hash_map<std::string, int> nmap;
        phmap::parallel_flat_hash_map<stringT, int> imap;
        for(auto& s : words)
        {
            auto is = T::intern(i, s);
            ++nmap[s]; ++imap[is];
            REQUIRE(nmap[s] == imap[is]);
        }
    }
    {
        auto s1 = T::intern(i, "SHORT1");
        auto s2 = T::intern(i, "SHORT2");
        REQUIRE(s1 == "SHORT1");
        REQUIRE(s2 == "SHORT2");
        s1.swap(s1);
        s2.swap(s2);
        REQUIRE(s1 == "SHORT1");
        REQUIRE(s2 == "SHORT2");
        s1 = s1;
        s2 = s2;
        REQUIRE(s1 == "SHORT1");
        REQUIRE(s2 == "SHORT2");
        s1.swap(s2);
        REQUIRE(s1 == "SHORT2");
        REQUIRE(s2 == "SHORT1");
        s1.swap(s2);
        REQUIRE(s1 == "SHORT1");
        REQUIRE(s2 == "SHORT2");
    }
    {
        constexpr static char txt[] = "RATHER LONG STRING RATHER LONG STRING";
        auto s1 = T::intern(i, txt);
        auto s2 = T::intern(i, "SHORT2");
        REQUIRE(s1 == txt);
        REQUIRE(s2 == "SHORT2");
        s1.swap(s1);
        s2.swap(s2);
        REQUIRE(s1 == txt);
        REQUIRE(s2 == "SHORT2");
        s1 = s1;
        s2 = s2;
        REQUIRE(s1 == txt);
        REQUIRE(s2 == "SHORT2");
        s1.swap(s2);
        REQUIRE(s1 == "SHORT2");
        REQUIRE(s2 == txt);
        s1.swap(s2);
        REQUIRE(s1 == txt);
        REQUIRE(s2 == "SHORT2");
    }
    {
        constexpr static char txt1[] = "rather long string rather long string";
        constexpr static char txt2[] = "RATHER LONG STRING RATHER LONG STRING";
        auto s1 = T::intern(i, txt1);
        auto s2 = T::intern(i, txt2);
        REQUIRE(s1 == txt1);
        REQUIRE(s2 == txt2);
        s1.swap(s1);
        s2.swap(s2);
        REQUIRE(s1 == txt1);
        REQUIRE(s2 == txt2);
        s1 = s1;
        s2 = s2;
        REQUIRE(s1 == txt1);
        REQUIRE(s2 == txt2);
        s1.swap(s2);
        REQUIRE(s1 == txt2);
        REQUIRE(s2 == txt1);
        s1.swap(s2);
        REQUIRE(s1 == txt1);
        REQUIRE(s2 == txt2);
    }
}

///////////////////////////////////////////////////////////////////////
// Test parameters

template<typename InternerTraits, typename StringTraits>
struct test_far_string
{
    using interner_traits = InternerTraits;
    using string_traits = StringTraits;
    template<typename I, typename... Args>
    static auto intern(I& i, Args... args)
    {
        return i.far(std::forward<Args>(args)...);
    }
};

template<typename InternerTraits, typename StringTraits>
struct test_sso_tiny
{
    using interner_traits = InternerTraits;
    using string_traits = StringTraits;
    template<typename I, typename... Args>
    static auto intern(I& i, Args... args)
    {
        return i.tiny(std::forward<Args>(args)...);
    }
};

template<typename InternerTraits, std::size_t S, typename StringTraits>
struct test_sso_v1_string
{
    using interner_traits = InternerTraits;
    using string_traits = StringTraits;
    template<typename I, typename... Args>
    static auto intern(I& i, Args... args)
    {
        return i.template sso1<S>(std::forward<Args>(args)...);
    }
};

template<typename InternerTraits, std::size_t S, typename StringTraits>
struct test_sso_v2_string
{
    using interner_traits = InternerTraits;
    using string_traits = StringTraits;
    template<typename I, typename... Args>
    static auto intern(I& i, Args... args)
    {
        return i.template sso1<S>(std::forward<Args>(args)...);
    }
};

///////////////////////////////////////////////////////////////////////
// Parameters for test parameters...

using Default = intern::default_string_traits;

struct DefaultFarDeep : Default
{
    constexpr static auto string_far_use_ptr_equality = false;
};

struct DefaultStrOps : Default
{
    inline static int cmp(const char* a, const char* b, std::size_t sz)
    {
        return std::strncmp(a, b, sz);
    }
    inline static void* copy(
            char* INTERN__RESTRICT dst,
            const char* INTERN__RESTRICT src,
            std::size_t sz)
    {
        return std::strncpy(dst, src, sz);
    }
};

struct Default8 : Default
{
    using size_type = std::uint64_t;
};
struct Default32 : Default
{
    using size_type = std::uint32_t;
};
struct Default64 : Default
{
    using size_type = std::uint64_t;
};

///////////////////////////////////////////////////////////////////////
// Test invocation

#define TEST_IT(traits)                                           \
TYPE_TO_STRING(test_far_string<interner_traits1, traits>);        \
TYPE_TO_STRING(test_sso_tiny<interner_traits1, traits>);          \
TYPE_TO_STRING(test_sso_v1_string<interner_traits1, 16, traits>); \
TYPE_TO_STRING(test_sso_v1_string<interner_traits1, 24, traits>); \
TYPE_TO_STRING(test_sso_v1_string<interner_traits1, 32, traits>); \
TYPE_TO_STRING(test_sso_v2_string<interner_traits1, 16, traits>); \
TYPE_TO_STRING(test_sso_v2_string<interner_traits1, 24, traits>); \
TYPE_TO_STRING(test_sso_v2_string<interner_traits1, 32, traits>); \
TYPE_TO_STRING(test_far_string<interner_traits2, traits>);        \
TYPE_TO_STRING(test_sso_tiny<interner_traits2, traits>);          \
TYPE_TO_STRING(test_sso_v1_string<interner_traits2, 16, traits>); \
TYPE_TO_STRING(test_sso_v1_string<interner_traits2, 24, traits>); \
TYPE_TO_STRING(test_sso_v1_string<interner_traits2, 32, traits>); \
TYPE_TO_STRING(test_sso_v2_string<interner_traits2, 16, traits>); \
TYPE_TO_STRING(test_sso_v2_string<interner_traits2, 24, traits>); \
TYPE_TO_STRING(test_sso_v2_string<interner_traits2, 32, traits>); \
TEST_CASE_TEMPLATE_INVOKE(                                        \
        test_id                                                   \
        , test_far_string<interner_traits1, traits>               \
        , test_sso_tiny<interner_traits1, traits>                 \
        , test_sso_v1_string<interner_traits1, 16, traits>        \
        , test_sso_v1_string<interner_traits1, 24, traits>        \
        , test_sso_v1_string<interner_traits1, 32, traits>        \
        , test_sso_v2_string<interner_traits1, 16, traits>        \
        , test_sso_v2_string<interner_traits1, 24, traits>        \
        , test_sso_v2_string<interner_traits1, 32, traits>        \
        , test_far_string<interner_traits2, traits>               \
        , test_sso_tiny<interner_traits2, traits>                 \
        , test_sso_v1_string<interner_traits2, 16, traits>        \
        , test_sso_v1_string<interner_traits2, 24, traits>        \
        , test_sso_v1_string<interner_traits2, 32, traits>        \
        , test_sso_v2_string<interner_traits2, 16, traits>        \
        , test_sso_v2_string<interner_traits2, 24, traits>        \
        , test_sso_v2_string<interner_traits2, 32, traits>        \
        );

