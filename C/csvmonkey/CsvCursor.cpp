#include "CsvCursor.h"

namespace csvmonkey {
    CsvCell *CsvCursor::by_value(
        const std::string &value
    ) {
        for (size_t i = 0; i < count; i++) {
            if (value == cells[i].as_str()) {
                return &cells[i];
            }
        }
        return nullptr;
    }
}
