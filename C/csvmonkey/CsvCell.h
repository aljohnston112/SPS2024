#ifndef WAVELET_CSVCELL_H
#define WAVELET_CSVCELL_H

#include <charconv>
#include <cstddef>
#include <string>

namespace csvmonkey {
    struct CsvCell {
        const char* ptr;
        size_t size;

        [[nodiscard]] std::string as_str() const;

        template <class T>
        [[nodiscard]] T from_chars_as_type() const {
            T type{};
            std::from_chars(
                ptr,
                ptr + size,
                type
            );
            return type;
        }
    };
}

#endif //WAVELET_CSVCELL_H
