#ifndef WAVELET_MAPPEDFILECURSOR_H
#define WAVELET_MAPPEDFILECURSOR_H

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <iostream>

#include "csvmonkey.hpp"

namespace csvmonkey {

    class MappedFileCursor {

        char *startPtr;
        char *endPtr;
        char *currentPtr;
        char *guardPtr;

        static size_t get_page_size();

    public:
        MappedFileCursor();

        ~MappedFileCursor();

        void open(const char *filename);

        const char *buf();

        size_t size();

        void consume(size_t n);

    };

}

#endif //WAVELET_MAPPEDFILECURSOR_H
