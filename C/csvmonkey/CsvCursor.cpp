#include "CsvCursor.h"

namespace csvmonkey {
    const CsvCell* CsvCursor::with_column_name(
        const std::string& value
    ) const {
        for (size_t i = 0; i < count; i++) {
            if (value == cells[i].as_str()) {
                return &cells[i];
            }
        }
        return nullptr;
    }
}
