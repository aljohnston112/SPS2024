#ifndef WAVELET_CSVCURSOR_H
#define WAVELET_CSVCURSOR_H

#include <vector>
#include <cstddef>
#include <string>
#include "CsvCell.h"

namespace csvmonkey {

    class CsvCursor {

    public:
        std::vector<CsvCell> cells;

        size_t count;

        CsvCursor() : cells(32),
                      count(0) {}

        CsvCell *by_value(const std::string &value);

    };

}

#endif //WAVELET_CSVCURSOR_H
