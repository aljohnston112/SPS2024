#include "CsvCell.h"

namespace csvmonkey {

    [[nodiscard]] std::string CsvCell::as_str() const {
        return {
                ptr,
                size
        };
    }

    [[nodiscard]] double CsvCell::as_double() const {
        double number;
        std::from_chars(
                ptr,
                ptr + size,
                number
        );
        return number;
    }

    [[nodiscard]] int CsvCell::as_int() const {
        int number;
        std::from_chars(
                ptr,
                ptr + size,
                number
        );
        return number;
    }

}