#ifndef WAVELET_CSVCELL_H
#define WAVELET_CSVCELL_H

#include <cstddef>
#include <string>
#include <charconv>

namespace csvmonkey {

    struct CsvCell {

        const char *ptr;
        size_t size;

        [[nodiscard]] std::string as_str() const;

        [[nodiscard]] double as_double() const;

        [[nodiscard]] int as_int() const;

    };

}

#endif //WAVELET_CSVCELL_H
