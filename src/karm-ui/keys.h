#pragma once

#include <karm-base/string.h>

namespace Karm::Ui {

#define FOREACH_KEY(KEY)              \
    KEY(INVALID, 0x0)                 \
    KEY(ESC, 0x01)                    \
    KEY(NUM1, 0x02)                   \
    KEY(NUM2, 0x03)                   \
    KEY(NUM3, 0x04)                   \
    KEY(NUM4, 0x05)                   \
    KEY(NUM5, 0x06)                   \
    KEY(NUM6, 0x07)                   \
    KEY(NUM7, 0x08)                   \
    KEY(NUM8, 0x09)                   \
    KEY(NUM9, 0x0A)                   \
    KEY(NUM0, 0x0B)                   \
    KEY(SYM1, 0x0C)                   \
    KEY(SYM2, 0x0D)                   \
    KEY(BKSPC, 0x0E)                  \
    KEY(TAB, 0x0F)                    \
    KEY(Q, 0x10)                      \
    KEY(W, 0x11)                      \
    KEY(E, 0x12)                      \
    KEY(R, 0x13)                      \
    KEY(T, 0x14)                      \
    KEY(Y, 0x15)                      \
    KEY(U, 0x16)                      \
    KEY(I, 0x17)                      \
    KEY(O, 0x18)                      \
    KEY(P, 0x19)                      \
    KEY(SYM3, 0x1A)                   \
    KEY(SYM4, 0x1B)                   \
    KEY(ENTER, 0x1C)                  \
    KEY(LCTRL, 0x1D)                  \
    KEY(A, 0x1E)                      \
    KEY(S, 0x1F)                      \
    KEY(D, 0x20)                      \
    KEY(F, 0x21)                      \
    KEY(G, 0x22)                      \
    KEY(H, 0x23)                      \
    KEY(J, 0x24)                      \
    KEY(K, 0x25)                      \
    KEY(L, 0x26)                      \
    KEY(SYM5, 0x27)                   \
    KEY(SYM6, 0x28)                   \
    KEY(SYM7, 0x29)                   \
    KEY(LSHIFT, 0x2A)                 \
    KEY(SYM8, 0x2B)                   \
    KEY(Z, 0x2C)                      \
    KEY(X, 0x2D)                      \
    KEY(C, 0x2E)                      \
    KEY(V, 0x2F)                      \
    KEY(B, 0x30)                      \
    KEY(N, 0x31)                      \
    KEY(M, 0x32)                      \
    KEY(SYM9, 0x33)                   \
    KEY(SYM10, 0x34)                  \
    KEY(SYM11, 0x35)                  \
    KEY(RSHIFT, 0x36)                 \
    KEY(SYM12, 0x37)                  \
    KEY(LALT, 0x38)                   \
    KEY(SPACE, 0x39)                  \
    KEY(CAPSLOCK, 0x3A)               \
    KEY(F1, 0x3B)                     \
    KEY(F2, 0x3C)                     \
    KEY(F3, 0x3D)                     \
    KEY(F4, 0x3E)                     \
    KEY(F5, 0x3F)                     \
    KEY(F6, 0x40)                     \
    KEY(F7, 0x41)                     \
    KEY(F8, 0x42)                     \
    KEY(F9, 0x43)                     \
    KEY(F10, 0x44)                    \
    KEY(NUMLOCK, 0x45)                \
    KEY(SCROLLLOCK, 0x46)             \
    KEY(KPAD7, 0x47)                  \
    KEY(KPAD8, 0x48)                  \
    KEY(KPAD9, 0x49)                  \
    KEY(SYM13, 0x4A)                  \
    KEY(KPAD4, 0x4B)                  \
    KEY(KPAD5, 0x4C)                  \
    KEY(KPAD6, 0x4D)                  \
    KEY(SYM14, 0x4E)                  \
    KEY(KPAD1, 0x4F)                  \
    KEY(KPAD2, 0x50)                  \
    KEY(KPAD3, 0x51)                  \
    KEY(KPAD0, 0x52)                  \
    KEY(SYM15, 0x53)                  \
    KEY(ALTSYSRQ, 0x54)               \
    KEY(NO_STANDARD_MEANING_1, 0x55)  \
    KEY(NO_STANDARD_MEANING_2, 0x56)  \
    KEY(F11, 0x57)                    \
    KEY(F12, 0x58)                    \
    KEY(KPADENTER, (0x80 + 0x1C))     \
    KEY(RCTRL, (0x80 + 0x1D))         \
    KEY(FAKELSHIFT, (0x80 + 0x2A))    \
    KEY(SYM16, (0x80 + 0x35))         \
    KEY(FAKERSHIFT, (0x80 + 0x36))    \
    KEY(CTRLPRINTSCRN, (0x80 + 0x37)) \
    KEY(RALT, (0x80 + 0x38))          \
    KEY(CTRLBREAK, (0x80 + 0x46))     \
    KEY(HOME, (0x80 + 0x47))          \
    KEY(UP, (0x80 + 0x48))            \
    KEY(PGUP, (0x80 + 0x49))          \
    KEY(LEFT, (0x80 + 0x4B))          \
    KEY(RIGHT, (0x80 + 0x4D))         \
    KEY(END, (0x80 + 0x4F))           \
    KEY(DOWN, (0x80 + 0x50))          \
    KEY(PGDOWN, (0x80 + 0x51))        \
    KEY(INSERT, (0x80 + 0x52))        \
    KEY(DELETE, (0x80 + 0x53))        \
    KEY(LSUPER, (0x80 + 0x5B))        \
    KEY(RSUPER, (0x80 + 0x5C))        \
    KEY(MENU, (0x80 + 0x5D))

struct Key {
    enum struct Code {
#define ITER(name, code) name = code,
        FOREACH_KEY(ITER)
#undef ITER
    };

    using enum Code;

    Code _code;

    Key() = default;
    Key(Code code) : _code(code) {}

    Str name() const {
        switch (_code) {
#define ITER(name, code) \
    case Code::name:     \
        return #name;
            FOREACH_KEY(ITER)
#undef ITER
        }
        return "INVALID";
    }
};

} // namespace Karm::Ui
