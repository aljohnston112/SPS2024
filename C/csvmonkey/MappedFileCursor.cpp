#include "MappedFileCursor.h"

namespace csvmonkey {

    MappedFileCursor::MappedFileCursor()
            : startPtr(nullptr),
              endPtr(nullptr),
              currentPtr(nullptr),
              guardPtr(nullptr) {}

    size_t MappedFileCursor::get_page_size() {
        return (size_t) sysconf(_SC_PAGESIZE);
    }

    MappedFileCursor::~MappedFileCursor() {
        if (startPtr) {
            ::munmap(
                    startPtr,
                    endPtr - startPtr
            );
        }
        if (guardPtr) {
            ::munmap(
                    guardPtr,
                    get_page_size()
            );
        }
    }

    void MappedFileCursor::open(const char *filename) {
        int fd = ::open(
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

        unsigned long page_size = get_page_size();
        unsigned long page_mask = page_size - 1;
        size_t rounded = (st.st_size + page_mask) & ~(page_mask);

        auto startp = (char *) mmap(
                nullptr,
                rounded + page_size,
                PROT_READ,
                MAP_ANON | MAP_PRIVATE,
                0,
                0
        );

        guardPtr = startp + rounded;

        startPtr = (char *) mmap(
                startp,
                st.st_size,
                PROT_READ,
                MAP_SHARED | MAP_FIXED,
                fd,
                0
        );

        ::close(fd);

        ::madvise(
                startPtr,
                st.st_size,
                MADV_SEQUENTIAL
        );
        ::madvise(
                startPtr,
                st.st_size,
                MADV_WILLNEED
        );
        endPtr = startPtr + st.st_size;
        currentPtr = startPtr;
    }

    const char *MappedFileCursor::buf() {
        return currentPtr;
    }

    size_t MappedFileCursor::size() {
        return endPtr - currentPtr;
    }

    void MappedFileCursor::consume(size_t n) {
        currentPtr += std::min(
                n,
                size()
        );

    }

}