#ifndef WAVELET_MAPPEDFILECURSOR_H
#define WAVELET_MAPPEDFILECURSOR_H

#include <cstddef>

namespace csvmonkey {
    class MappedFileCursor {
        char* startPtr;
        char* endPtr;
        char* currentPtr;
        char* guardPtr;

        static long get_page_size();

    public:
        MappedFileCursor();

        ~MappedFileCursor();

        void open(const char* filename);

        [[nodiscard]] const char* buf() const;

        [[nodiscard]] size_t size() const;

        void consume(size_t n);
    };
}

#endif //WAVELET_MAPPEDFILECURSOR_H
