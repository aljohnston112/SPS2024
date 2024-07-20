#include "CsvWriter.h"

void writeCsv(
        const std::string &filePath,
        const std::map<CSV::Column, std::vector<int>> &data
) {
    int fd = open(
            filePath.c_str(),
            O_RDWR | O_CREAT,
            S_IRUSR | S_IRGRP | S_IROTH
    );

    if (fd == -1) {
        perror("Error opening file");
    }

    size_t fileSize = 0;

    std::map<CSV::Column, std::vector<std::string>> dataStrings;
    std::string header;
    for (const auto &d: data) {
        dataStrings.emplace(
                d.first,
                std::vector<std::string>()
        );
        header += CSV::ColumnToString(d.first);
        header += ',';
    }
    header.back() = '\n';
    fileSize += header.size();

    for (const auto &d: data) {
        CSV::Column column = d.first;
        std::vector<int> series = d.second;

        for (const auto &value: series) {
            auto s = std::to_string(value);
            fileSize += s.size() + 1;
            dataStrings.at(column).push_back(s);
        }
    }

    if (lseek(fd, fileSize - 1, SEEK_SET) == -1) {}
    if (write(fd, "", 1) == -1) {}

    char *map = (char *) mmap(
            nullptr,
            fileSize,
            PROT_WRITE,
            MAP_SHARED,
            fd,
            0
    );
    if (map == MAP_FAILED) {}

// Copy data to memory map
    size_t offset = 0;
    std::memcpy(
            map,
            header.c_str(),
            header.size()
    );
    offset += header.size();

    for (int i = 0; i < dataStrings.begin()->second.size(); i++) {
        for (const auto &d: data) {
            CSV::Column column = d.first;
            auto str = dataStrings[column][i];
            std::memcpy(
                    map + offset,
                    str.c_str(),
                    str.size()
            );
            offset += str.size();
            map[offset++] = ',';
        }
        offset--;
        map[offset++] = '\n';
    }

    if (msync(map, fileSize, MS_SYNC) == -1) {
        perror("Error syncing file");
    }
    if (munmap(map, fileSize) == -1) {
        perror("Error unmapping file");
    }
    close(fd);
}