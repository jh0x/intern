#include <intern/interner.hpp>
#include <intern/interner_demo_traits.hpp>
#include <intern/string_io.hpp>
#include <iostream>

namespace x = intern;

constexpr static auto long_string =
    "Lorem ipsum dolor sit amet, consectetur adipiscing elit";

int main()
{
    x::interner<x::interner_sample_traits<(1<<16)>> interner;

    {
        // Far string - will be interned
        auto far1 = interner.far(long_string);
        auto far2 = interner.far(long_string);
        auto far3 = far2;

        std::cout << "(void*)far1.data()=" << (void*)far1.data()
            << ", far1=" << far1 << '\n';
        std::cout << "(void*)far2.data()=" << (void*)far2.data()
            << ", far2=" << far2 << '\n';
        std::cout << "(void*)far3.data()=" << (void*)far3.data()
            << ", far3=" << far3 << '\n';
    }
    std::cout << "\n";
    {
        // Different flavours of SSO
        // Will not intern if string fits in the SSO
        auto tiny = interner.tiny("SPY");
        auto small1 = interner.sso1<16>("SPY");
        auto small2 = interner.sso2<16>("SPY");

        std::cout << "  (void*)tiny.data()=" << (void*)tiny.data()
            << ",   &tiny=" << &tiny << ",   tiny=" << tiny << '\n';
        std::cout << "(void*)small1.data()=" << (void*)small1.data()
            << ", &small1=" << &small1 << ", small1=" << small1 << '\n';
        std::cout << "(void*)small2.data()=" << (void*)small2.data()
            << ", &small2=" << &small2 << ", small2=" << small2 << '\n';
    }
    std::cout << "\n";
    {
        // But falls back to using interner with longer strings
        auto tiny = interner.tiny(long_string);
        auto small1 = interner.sso1<16>(long_string);
        auto small2 = interner.sso2<16>(long_string);

        std::cout << "  (void*)tiny.data()=" << (void*)tiny.data()
            << ",   &tiny=" << &tiny << ",   tiny=" << tiny << '\n';
        std::cout << "(void*)small1.data()=" << (void*)small1.data()
            << ", &small1=" << &small1 << ", small1=" << small1 << '\n';
        std::cout << "(void*)small2.data()=" << (void*)small2.data()
            << ", &small2=" << &small2 << ", small2=" << small2 << '\n';
    }
}

