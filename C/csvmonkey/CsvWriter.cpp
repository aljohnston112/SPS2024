#include <cstring>
#include <fcntl.h>
#include <ranges>
#include <sys/mman.h>

#include "../csv//csv_util.h"
#include "CsvWriter.h"

void writeCsv(
    const std::string& filePath,
    std::vector<std::vector<int>>& data
) {
    const int fd = open(
        filePath.c_str(),
        O_RDWR | O_CREAT, S_IRUSR | S_IRGRP | S_IROTH
    );

    if (fd == -1) {
        perror("Error opening file");
    }

    size_t fileSize = 0;
    std::vector<std::vector<std::string>> dataStrings;
    dataStrings.reserve(data.size());
    const auto it = dataStrings.begin();
    for (size_t key = 0; key < data.size(); key++) {
        dataStrings.emplace(
            it + static_cast<int>(key)
        );
    }

    // Create the header
    std::string header;
    for (size_t key = 0; key < data.size(); key++) {
        header += ColumnToString(
            CSV::Column{static_cast<int>(key)}
        );
        header += ',';
    }
    header.back() = '\n';
    fileSize += header.size();

    // Create the data rows
    for (size_t key = 0; key < data.size(); key++) {
        for (
            std::vector<int>& series = data[key];
            const auto& data_point : series
        ) {
            auto data_point_as_text = std::to_string(data_point);
            fileSize += data_point_as_text.size() + 1;
            dataStrings[CSV::Column{static_cast<int>(key)}].push_back(data_point_as_text);
        }
    }

    // Fill the file with nothing
    if (lseek(fd, static_cast<long>(fileSize - 1), SEEK_SET) == -1) {
        perror("Error seeking in file");
    }
    write(fd, reinterpret_cast<const void*>(""), 1);

    // Map into memory
    auto* map = static_cast<char*>(
        mmap(
            nullptr,
            fileSize,
            PROT_WRITE,
            MAP_SHARED,
            fd,
            0
        )
    );

    if (map == MAP_FAILED) {
        perror("Error memory mapping file");
    }

    // Copy data to memory map
    // The header
    size_t offset = 0;
    std::memcpy(
        map,
        header.c_str(),
        header.size()
    );
    offset += header.size();

    // The data rows
    for (size_t i = 0; i < dataStrings.begin()->size(); i++) {
        for (size_t key = 0; key < data.size(); key++) {
            auto str = dataStrings[CSV::Column{static_cast<int>(key)}][i];
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
