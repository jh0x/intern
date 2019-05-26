# intern
Simple Interner for Immutable Strings

### Tiny Small String

- `sizeof(T) == sizeof(char*)`
- `sso_size == sizeof(T) - 1`
- last byte used to store adjusted size and a flag indicating whether SSO is being used.
- non-SSO case requires that the far data is at least `alignas(2)` because we use the least significant bit for storing non-SSO flag.
- non-SSO case stores the data pointer in Big Endian notation thus needed a `bswap` on the way in and out.
- trivial to copy
- checking whether `small()` always required

#### Examples:
##### SSO
`░ := storage`, `▓ := sso_size - length`:

```
┌76543210┐┌76543210┐┌76543210┐┌76543210┐┌76543210┐┌76543210┐┌76543210┐┌76543210┐
│░░░░░░░░││░░░░░░░░││░░░░░░░░││░░░░░░░░││░░░░░░░░││░░░░░░░░││░░░░░░░░││▓▓▓▓▓▓▓0│
└────────┘└────────┘└────────┘└────────┘└────────┘└────────┘└────────┘└────────┘

String = 012345

┌76543210┐┌76543210┐┌76543210┐┌76543210┐┌76543210┐┌76543210┐┌76543210┐┌76543210┐
│00110000││00110000││00110000││00110000││00110000││00110000││00000000││00000010│
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
