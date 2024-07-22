#include <cstring>

#include "StringSpanner.h"

namespace csvmonkey {
    StringSpannerFallback::StringSpannerFallback( // NOLINT(*-pro-type-member-init)
        const char c1,
        const char c2,
        const char c3,
        const char c4
    ) {
        ::memset(
            charset_,
            0,
            sizeof charset_
        );
        charset_[static_cast<unsigned char>(c1)] = 1;
        charset_[static_cast<unsigned char>(c2)] = 1;
        charset_[static_cast<unsigned char>(c3)] = 1;
        charset_[static_cast<unsigned char>(c4)] = 1;
        charset_[static_cast<unsigned char>(0)] = 1;
    }

    inline size_t StringSpannerFallback::operator()(const char* string) const {
        auto ptr = string;
        const auto endPtr = ptr + 16;

        bool found = false;

        ptr -= 4;
        while (!found && ptr < endPtr) {
            ptr += 4;
            if (charset_[static_cast<unsigned char>(ptr[0])]) {
                found = true;
            }
            else if (charset_[static_cast<unsigned char>(ptr[1])]) {
                ptr++;
                found = true;
            }
            else if (charset_[static_cast<unsigned char>(ptr[2])]) {
                ptr += 2;
                found = true;
            }
            else if (charset_[static_cast<unsigned char>(ptr[3])]) {
                ptr += 3;
                found = true;
            }
        }

        if (!*ptr) {
            return 16; // PCMPISTRI reports NUL encountered as no match.
        }

        return ptr - string;
    }
}
