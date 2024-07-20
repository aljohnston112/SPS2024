#ifndef WAVELET_STRINGSPANNER_H
#define WAVELET_STRINGSPANNER_H

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <cassert>
#include "csvmonkey.hpp"

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

        size_t operator()(const char *string) __attribute__((__always_inline__));

    };


#ifndef CAN_USE_SSE42
#warning Using non-SSE4.2 fallback implementation.
    using StringSpanner = StringSpannerFallback;
#   define CSM_ATTR_SSE42
#endif // !CAN_USE_SSE42


#ifdef CAN_USE_SSE42

    struct alignas(16) StringSpannerSse42 {

        __m128i v_;

        explicit StringSpannerSse42(
                char c1 = 0,
                char c2 = 0,
                char c3 = 0,
                char c4 = 0
        );

        size_t operator()(const char *buf) const;

    };

    using StringSpanner = StringSpannerSse42;
#   define CSM_ATTR_SSE42 __attribute__((target("sse4.2")))
#endif // CAN_USE_SSE42

}

#endif //WAVELET_STRINGSPANNER_H
