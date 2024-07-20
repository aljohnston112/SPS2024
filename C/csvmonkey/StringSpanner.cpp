#include "StringSpanner.h"

namespace csvmonkey {

    StringSpannerFallback::StringSpannerFallback( // NOLINT(*-pro-type-member-init)
            char c1,
            char c2,
            char c3,
            char c4
    ) {
        ::memset(
                charset_,
                0,
                sizeof charset_
        );
        charset_[(unsigned char) c1] = 1;
        charset_[(unsigned char) c2] = 1;
        charset_[(unsigned char) c3] = 1;
        charset_[(unsigned char) c4] = 1;
        charset_[0] = 1;
    }

    size_t StringSpannerFallback::operator()(const char *string) {

        auto ptr = (const unsigned char *) string;
        auto endPtr = ptr + 16;

        bool found = false;

        ptr -= 4;
        while (!found && ptr < endPtr) {
            ptr += 4;
            if (charset_[ptr[0]]) {
                found = true;
            } else if (charset_[ptr[1]]) {
                ptr++;
                found = true;
            } else if (charset_[ptr[2]]) {
                ptr += 2;
                found = true;
            } else if (charset_[ptr[3]]) {
                ptr += 3;
                found = true;
            }
        }

        if (!*ptr) {
            return 16; // PCMPISTRI reports NUL encountered as no match.
        }

        return ptr - (const unsigned char *) string;
    }


        StringSpannerSse42::StringSpannerSse42(
                char c1,
                char c2,
                char c3,
                char c4
        ) {
            // assert(!((reinterpret_cast<intptr_t>(&v_) & 15)));

            __v16qi vq = {c1, c2, c3, c4};
            v_ = (__m128i) vq;
        }

        size_t StringSpannerSse42::operator()(const char *buf) const {
            return _mm_cmpistri(
                    v_,
                    _mm_loadu_si128((__m128i *) buf),
                    _SIDD_UBYTE_OPS |
                    _SIDD_CMP_EQUAL_ANY |
                    _SIDD_POSITIVE_POLARITY
            );
        }

}