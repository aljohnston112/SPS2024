#include "CsvCell.h"

namespace csvmonkey {
    [[nodiscard]] std::string CsvCell::as_str() const {
        return {
            ptr,
            size
        };
    }
}
