#ifndef WAVELET_CSVCURSOR_H
#define WAVELET_CSVCURSOR_H

#include <cstddef>
#include <string>
#include <vector>

#include "CsvCell.h"

namespace csvmonkey {
    class CsvCursor {
    public:
        std::vector<CsvCell> cells;

        size_t count;

        CsvCursor()
            : cells(32),
              count(0) {}

        [[nodiscard]] const CsvCell* with_column_name(
            const std::string& value
        ) const ;
    };
}

#endif //WAVELET_CSVCURSOR_H
