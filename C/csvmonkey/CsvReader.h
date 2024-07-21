#ifndef WAVELET_CSVREADER_H
#define WAVELET_CSVREADER_H

#include "CsvCursor.h"
#include "StringSpanner.h"

namespace csvmonkey {
    template<class StreamCursorType>
    class alignas(16) CsvReader {
    public:
        explicit CsvReader(
            StreamCursorType &stream,
            const char delimiter = ','
        ) : endPtr(stream.buf() + stream.size()),
            ptr(stream.buf()),
            delimiter(delimiter),
            stream(stream),
            unquoted_cell_spanner(
                delimiter,
                '\r',
                '\n'
            ) {
        }

        bool read_row() {
            bool canContinue = false;
            const char *startP = ptr;

            if (try_parse()) {
                stream.consume(ptr - startP);
                canContinue = true;
            }
            return canContinue;
        }

        CsvCursor &row() {
            return cursor;
        }

    private:
        const char *endPtr;
        const char *ptr;
        char delimiter;
        StreamCursorType &stream;
        StringSpanner unquoted_cell_spanner;
        CsvCursor cursor;

        bool try_parse() {
            const char *currentCharacterPtr = ptr;

            CsvCell *cell = &cursor.cells[0];
            cursor.count = 0;

            while (*currentCharacterPtr == '\r' || *currentCharacterPtr == '\n') {
                ++currentCharacterPtr;
            }

            while (currentCharacterPtr < endPtr) {
                if (*currentCharacterPtr == '\r' || *currentCharacterPtr == '\n') {
                    cell->ptr = nullptr;
                    cell->size = 0;

                    // These do not make sense, the file has ended
                    cursor.count++;
                    ptr = currentCharacterPtr + 1;
                    return true;
                } else {
                    const char *cell_start = currentCharacterPtr;

                    // TODO This should be a loop
                    // It is a hard limit of 32 characters
                    if (
                        const int rc = unquoted_cell_spanner(currentCharacterPtr);
                        rc != 16
                    ) {
                        currentCharacterPtr += rc;
                    } else {
                        const int rc2 = unquoted_cell_spanner(currentCharacterPtr + 16);
                        currentCharacterPtr += rc2 + 16;
                    }

                    cell->ptr = cell_start;
                    cell->size = currentCharacterPtr - cell_start;
                    ++cursor.count;

                    if (*currentCharacterPtr == delimiter) {
                        ++cell;
                        ++currentCharacterPtr;
                    } else {
                        ptr = currentCharacterPtr + 1;
                        return true;
                    }
                }
            }
            return false;
        }
    };
}

#endif //WAVELET_CSVREADER_H
