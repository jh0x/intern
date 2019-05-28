# intern
Simple Interner for Immutable Strings

## Table of Contents
- [Interner](#interner)
    - [Simple Example](#simple-example)
- [Small Strings](#small-strings)
    - [Tiny Small String](#tiny-small-string)
    - [Small String V1](#small-string-v1)
    - [Small String V2](#small-string-v2)
- [Internals Demo](#internals-demo)

## Interner
### Simple Example

```cpp
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
```

And sample output:

```
(void*)far1.data()=0x4051da, far1=Lorem ipsum dolor sit amet, consectetur adipiscing elit
(void*)far2.data()=0x4051da, far2=Lorem ipsum dolor sit amet, consectetur adipiscing elit
(void*)far3.data()=0x4051da, far3=Lorem ipsum dolor sit amet, consectetur adipiscing elit

  (void*)tiny.data()=0x7ffea02056c0,   &tiny=0x7ffea02056c0,   tiny=SPY
(void*)small1.data()=0x7ffea02056d0, &small1=0x7ffea02056d0, small1=SPY
(void*)small2.data()=0x7ffea02056ea, &small2=0x7ffea02056e0, small2=SPY

  (void*)tiny.data()=0x4051da,   &tiny=0x7ffea0205698,   tiny=Lorem ipsum dolor sit amet, consectetur adipiscing elit
(void*)small1.data()=0x4051da, &small1=0x7ffea02056a0, small1=Lorem ipsum dolor sit amet, consectetur adipiscing elit
(void*)small2.data()=0x4051da, &small2=0x7ffea02056c0, small2=Lorem ipsum dolor sit amet, consectetur adipiscing elit
```

For more information about the internals of the different Small Strings
see [Small Strings](#small-strings).

## Small Strings
### Tiny Small String

- `sizeof(T) == sizeof(char*)`
- `sso_size == sizeof(T) - 1`
- Last byte used to store adjusted size (SSO) and a flag indicating whether SSO is being used.
- Non-SSO case requires that the far data is at least `alignas(2)` because we use the least significant bit for storing non-SSO flag.
- Non-SSO case stores the data pointer in Big Endian notation thus needed a `bswap` on the way in and out.
- Trivial to copy,
- Checking whether `small()` always required.

#### Examples:
##### SSO
`░ := storage`, `▓ := sso_size - length`:

```
┌76543210┐┌76543210┐┌76543210┐┌76543210┐┌76543210┐┌76543210┐┌76543210┐┌76543210┐
│░░░░░░░░││░░░░░░░░││░░░░░░░░││░░░░░░░░││░░░░░░░░││░░░░░░░░││░░░░░░░░││▓▓▓▓▓▓▓0│
└────────┘└────────┘└────────┘└────────┘└────────┘└────────┘└────────┘└────────┘

String = 012345, sz = 6, sso_size - sz = 7 - 6 = 1

┌76543210┐┌76543210┐┌76543210┐┌76543210┐┌76543210┐┌76543210┐┌76543210┐┌76543210┐
│00110000││00110001││00110010││00110011││00110100││00110101││00000000││00000010│
├────────┤├────────┤├────────┤├────────┤├────────┤├────────┤├────────┤├────────┤
│0x30='0'││0x31='1'││0x32='2'││0x33='3'││0x34='4'││0x35='5'││0x0='\0'││(7-6)<<1│
└────────┘└────────┘└────────┘└────────┘└────────┘└────────┘└────────┘└────────┘
```

##### Non-SSO
`▒ := data pointer` (stored as Big Endian number):

```
┌76543210┐┌76543210┐┌76543210┐┌76543210┐┌76543210┐┌76543210┐┌76543210┐┌76543210┐
│▒▒▒▒▒▒▒▒││▒▒▒▒▒▒▒▒││▒▒▒▒▒▒▒▒││▒▒▒▒▒▒▒▒││▒▒▒▒▒▒▒▒││▒▒▒▒▒▒▒▒││▒▒▒▒▒▒▒▒││▒▒▒▒▒▒▒1│
└────────┘└────────┘└────────┘└────────┘└────────┘└────────┘└────────┘└────────┘

const char* ptr = 0x000000000040724a (this is natural notation)
                  0x4a72400000000000 (this is little endian representation)

Little Endian representation does not work with our SSO so we need to do a bswap:

   Here we use the "extra storage" due to the string being at least alignas(2)┐
                                                                              │
┌76543210┐┌76543210┐┌76543210┐┌76543210┐┌76543210┐┌76543210┐┌76543210┐┌76543210┐
│00000000││00000000││00000000││00000000││00000000││01000000││01110010││01001011│
├────────┤├────────┤├────────┤├────────┤├────────┤├────────┤├────────┤├────────┤
│   00   ││   00   ││   00   ││   00   ││   00   ││   40   ││   72   ││   4b   │
└────────┘└────────┘└────────┘└────────┘└────────┘└────────┘└────────┘└────────┘
```


### Small String V1

- `sizeof(T) == S`, `S >= 16`, `S % 8 == 0`
- `sso_size == S - 1`
- Parameter `S` introduces extra SSO space into the structure.
- Last byte used to store adjusted size (SSO) and a flag indicating whether SSO is being used.
- Mon-SSO case stores the data pointer in natural representation. We also store string length locally.
- Trivial to copy.
- Checking whether `small()` always required.

#### Examples:
##### SSO
`░ := storage`, `▓ := sso_size - length`:

```
┌76543210┐┌76543210┐┌76543210┐┌76543210┐┌76543210┐┌76543210┐┌76543210┐┌76543210┐
│░░░░░░░░││░░░░░░░░││░░░░░░░░││░░░░░░░░││░░░░░░░░││░░░░░░░░││░░░░░░░░││░░░░░░░░│
└────────┘└────────┘└────────┘└────────┘└────────┘└────────┘└────────┘└────────┘
┌76543210┐┌76543210┐┌76543210┐┌76543210┐┌76543210┐┌76543210┐┌76543210┐┌76543210┐
│░░░░░░░░││░░░░░░░░││░░░░░░░░││░░░░░░░░││░░░░░░░░││░░░░░░░░││░░░░░░░░││▓▓▓▓▓▓▓0│
└────────┘└────────┘└────────┘└────────┘└────────┘└────────┘└────────┘└────────┘

String = 0123456789ABCDE, sz = 15, sso_size - sz = 15 - 15 = 0

┌76543210┐┌76543210┐┌76543210┐┌76543210┐┌76543210┐┌76543210┐┌76543210┐┌76543210┐
│00110000││00110001││00110010││00110011││00110100││00110101││00110110││00110111│
├────────┤├────────┤├────────┤├────────┤├────────┤├────────┤├────────┤├────────┤
│0x30='0'││0x31='1'││0x32='2'││0x33='3'││0x34='4'││0x35='5'││0x36='6'││0x37='7'│
└────────┘└────────┘└────────┘└────────┘└────────┘└────────┘└────────┘└────────┘
┌76543210┐┌76543210┐┌76543210┐┌76543210┐┌76543210┐┌76543210┐┌76543210┐┌76543210┐
│00111000││00111001││01000001││01000010││01000011││01000100││01000101││00000000│
├────────┤├────────┤├────────┤├────────┤├────────┤├────────┤├────────┤├────────┤
│0x38='8'││0x39='9'││0x41='A'││0x42='B'││0x43='C'││0x44='D'││0x45='E'││(15-15) << 1│
└────────┘└────────┘└────────┘└────────┘└────────┘└────────┘└────────┘└────────┘
```

##### Non-SSO
`▒ := data pointer`, `▓ := size` :

```
┌76543210┐┌76543210┐┌76543210┐┌76543210┐┌76543210┐┌76543210┐┌76543210┐┌76543210┐
│░░░░░░░░││░░░░░░░░││░░░░░░░░││░░░░░░░░││░░░░░░░░││░░░░░░░░││░░░░░░░░││░░░░░░░░│
└────────┘└────────┘└────────┘└────────┘└────────┘└────────┘└────────┘└────────┘
┌76543210┐┌76543210┐┌76543210┐┌76543210┐┌76543210┐┌76543210┐┌76543210┐┌76543210┐
│▓▓▓▓▓▓▓▓││▓▓▓▓▓▓▓▓││▓▓▓▓▓▓▓▓││▓▓▓▓▓▓▓▓││▓▓▓▓▓▓▓▓││▓▓▓▓▓▓▓▓││▓▓▓▓▓▓▓▓││▓▓▓▓▓▓▓▓│
└────────┘└────────┘└────────┘└────────┘└────────┘└────────┘└────────┘└────────┘

const char* ptr = 0x000000000040724a, sz = 130 (0x82)

┌76543210┐┌76543210┐┌76543210┐┌76543210┐┌76543210┐┌76543210┐┌76543210┐┌76543210┐
│01001010││01110010││01000000││00000000││00000000││00000000││00000000││00000000│
├────────┤├────────┤├────────┤├────────┤├────────┤├────────┤├────────┤├────────┤
│   4a   ││   72   ││   40   ││   00   ││   00   ││   00   ││   00   ││   00   │
└────────┘└────────┘└────────┘└────────┘└────────┘└────────┘└────────┘└────────┘
┌76543210┐┌76543210┐┌76543210┐┌76543210┐┌76543210┐┌76543210┐┌76543210┐┌76543210┐
│10000010││00000000││00000000││00000000││00000000││00000000││00000000││00000001│
├────────┤├────────┤├────────┤├────────┤├────────┤├────────┤├────────┤├───────╫┤
│   82   ││   00   ││   00   ││   00   ││   00   ││   00   ││   00   ││   01  ║│
└────────┘└────────┘└────────┘└────────┘└────────┘└────────┘└────────┘└───────╫┘
                                                                              ║
                  Here we steal one bit of storage from size to indicate mode ╝
```


### Small String V2
- `sizeof(T) == S`, `S >= 16`, `S % 8 == 0`
- `sso_size == S - 8 - sizeof(size_type)`
- Parameter `S` introduces extra SSO space into the structure.
- Not trivial to copy.
- Data always directly under the pointer.
- Check whether `small()` **NOT** always required.

#### Examples:
##### SSO
`▒ := data pointer`, `▓ := size`, `█ - payload`
```
            ┏┳━━━━━━━━━━━━━━━━━━━┓
 ┏━━━━━━━━━━┻┻━━━━━━━━━━━┓       ┃
┌00┬01┬02┬03┐┌04┬05┬06┬07┐┌08┬09┬0a┬0b┐┌0c┬0d┬0e┬0f┐
│▒▒│▒▒│▒▒│▒▒││▒▒│▒▒│▒▒│▒▒││▓▓│▓▓│██│██││██│██│██│00│
└──┴──┴──┴──┘└──┴──┴──┴──┘└──┴──┴──┴──┘└──┴──┴──┴──┘
```

##### Non-SSO
`▒ := data pointer`, `▓ := size`
```
            ┏┳━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━> far location
 ┏━━━━━━━━━━┻┻━━━━━━━━━━━┓
┌00┬01┬02┬03┐┌04┬05┬06┬07┐┌08┬09┬0a┬0b┐┌0c┬0d┬0e┬0f┐
│▒▒│▒▒│▒▒│▒▒││▒▒│▒▒│▒▒│▒▒││▓▓│▓▓│  │  ││  │  │  │  │
└──┴──┴──┴──┘└──┴──┴──┴──┘└──┴──┴──┴──┘└──┴──┴──┴──┘
```

## Internals Demo

`test/internals.cpp` can be used to inspect internal representation of
different types of small strings. In case when it was not possible to
make use of SSO the far string from the interner is also dumped.

```bash
$ ./internals
```

Sample output for `string_sso_tiny` when SSO is used:
```
OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO
My typeinfo: intern::string_sso_tiny<intern::default_string_traits>
OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO
String: '0123456'
	small=1, sso_size=7
	size=7
	sizeof(s)=8
	&s=0x7fff3936d888
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                      00 01 02 03  04 05 06 07  08 09 0a 0b  0c 0d 0e 0f | 0123456789abcdef
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 0x00007fff3936d880:                            30 31 32 33  34 35 36 00 |         0123456.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO
```

Sample output for `string_sso_v2<24>` when SSO is not used:
```
OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO
My typeinfo: intern::string_sso_v2<24ul, intern::default_string_traits>
OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO
String: 'This is a very very long string. And it will not fit in the Small String Optimised String. So we should expect to see it interned!'
	small=0, sso_size=13
	size=130
	sizeof(s)=24
	&s=0x7ffebeb53640
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                      00 01 02 03  04 05 06 07  08 09 0a 0b  0c 0d 0e 0f | 0123456789abcdef
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 0x00007ffebeb53640:  4a 72 40 00  00 00 00 00  82 00 93 1c  00 00 00 00 | Jr@.............
 0x00007ffebeb53650:  60 f3 9b 8c  89 7f 00 00                           | `.......
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Far address=0x40724a
	Far data:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                      00 01 02 03  04 05 06 07  08 09 0a 0b  0c 0d 0e 0f | 0123456789abcdef
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 0x0000000000407240:                            82 00 54 68  69 73 20 69 |         ..This i
 0x0000000000407250:  73 20 61 20  76 65 72 79  20 76 65 72  79 20 6c 6f | s a very very lo
 0x0000000000407260:  6e 67 20 73  74 72 69 6e  67 2e 20 41  6e 64 20 69 | ng string. And i
 0x0000000000407270:  74 20 77 69  6c 6c 20 6e  6f 74 20 66  69 74 20 69 | t will not fit i
 0x0000000000407280:  6e 20 74 68  65 20 53 6d  61 6c 6c 20  53 74 72 69 | n the Small Stri
 0x0000000000407290:  6e 67 20 4f  70 74 69 6d  69 73 65 64  20 53 74 72 | ng Optimised Str
 0x00000000004072a0:  69 6e 67 2e  20 53 6f 20  77 65 20 73  68 6f 75 6c | ing. So we shoul
 0x00000000004072b0:  64 20 65 78  70 65 63 74  20 74 6f 20  73 65 65 20 | d expect to see 
 0x00000000004072c0:  69 74 20 69  6e 74 65 72  6e 65 64 21              | it interned!
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO
```
