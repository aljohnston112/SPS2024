#include "MappedFileCursor.h"

#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

namespace csvmonkey {
    MappedFileCursor::MappedFileCursor()
        : startPtr(nullptr),
          endPtr(nullptr),
          currentPtr(nullptr),
          guardPtr(nullptr) {}

    long MappedFileCursor::get_page_size() {
        return sysconf(_SC_PAGESIZE);
    }

    MappedFileCursor::~MappedFileCursor() {
        if (startPtr) {
            munmap(
                startPtr,
                endPtr - startPtr
            );
        }
        if (guardPtr) {
            munmap(
                guardPtr,
                get_page_size()
            );
        }
    }

    void MappedFileCursor::open(const char* filename) {
        const int fd = ::open(
            filename,
            O_RDONLY
        );

        if (fd == -1) {
            // Failed to open file
            perror("Error opening file");
            std::cerr << "Error opening file: " << strerror(errno) << std::endl;
        }

        struct stat st; // NOLINT(*-pro-type-member-init)
        fstat(fd, &st);

        const unsigned long page_size = get_page_size();
        const unsigned long page_mask = page_size - 1;
        const size_t rounded = (st.st_size + page_mask) & ~(page_mask);

        const auto startp = static_cast<char*>(
            mmap(
                nullptr,
                rounded + page_size,
                PROT_READ,
                MAP_ANON | MAP_PRIVATE,
                0,
                0
            )
        );

        guardPtr = startp + rounded;

        startPtr = static_cast<char*>(
            mmap(
                startp,
                st.st_size,
                PROT_READ,
                MAP_SHARED | MAP_FIXED,
                fd,
                0
            )
        );

        close(fd);

        madvise(
            startPtr,
            st.st_size,
            MADV_SEQUENTIAL
        );
        madvise(
            startPtr,
            st.st_size,
            MADV_WILLNEED
        );
        endPtr = startPtr + st.st_size;
        currentPtr = startPtr;
    }

    const char* MappedFileCursor::buf() const {
        return currentPtr;
    }

    size_t MappedFileCursor::size() const {
        return endPtr - currentPtr;
    }

    void MappedFileCursor::consume(const size_t n) {
        currentPtr += std::min(
            n,
            size()
        );
    }
}
