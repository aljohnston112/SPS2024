#ifndef WAVELET_STRINGSPANNER_H
#define WAVELET_STRINGSPANNER_H

#include <cstdint>
#include <cstddef>

#ifdef __SSE4_2__
#include <emmintrin.h>
#include <smmintrin.h>
#endif

namespace csvmonkey {

    /**
     * Callable that matches a set of up to 5 bytes (including NUL) in a 16 byte
     * string. The index 0..15 of the first occurrence is returned, otherwise 16 is
     * returned if no match is found or NUL is encountered.
     */
    struct StringSpannerFallback {
        uint8_t charset_[256];

        explicit StringSpannerFallback( // NOLINT(*-pro-type-member-init)
            char c1 = 0,
            char c2 = 0,
            char c3 = 0,
            char c4 = 0
        );

        size_t operator()(const char* string) const __attribute__((__always_inline__));
    };


#ifndef __SSE4_2__
#warning Using non-SSE4.2 fallback implementation.
    using StringSpanner = StringSpannerFallback;
#else

    struct alignas(16) StringSpannerSse42 {
        __m128i v_;

        explicit StringSpannerSse42(
            const char c1 = 0,
            const char c2 = 0,
            const char c3 = 0,
            const char c4 = 0
        ) {
            // assert(!((reinterpret_cast<intptr_t>(&v_) & 15)));
            v_ = _mm_set_epi8(
                0, 0, 0, 0,
                0, 0, 0, 0,
                0, 0, 0, 0,
                c4, c3, c2, c1
            );
        }

        size_t operator()(const char* buf) const __attribute__((__always_inline__)) {
            return _mm_cmpistri(
                v_,
                _mm_loadu_si128(reinterpret_cast<__m128i *>(const_cast<char *>(buf))),
                _SIDD_UBYTE_OPS |
                _SIDD_CMP_EQUAL_ANY |
                _SIDD_POSITIVE_POLARITY
            );
        }
    };

    using StringSpanner = StringSpannerSse42;
#endif
}

#endif //WAVELET_STRINGSPANNER_H
