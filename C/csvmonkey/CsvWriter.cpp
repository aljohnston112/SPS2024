#include <ranges>

#include "CsvWriter.h"

void writeCsv(
    const std::string &filePath,
    std::map<CSV::Column,
        std::vector<int>> &data
) {
    const int fd = open(
        filePath.c_str(),
        O_RDWR | O_CREAT,
        S_IRUSR | S_IRGRP | S_IROTH
    );

    if (fd == -1) {
        perror("Error opening file");
    }

    size_t fileSize = 0;

    std::map<
        CSV::Column,
        std::vector<std::string>
    > dataStrings;
    std::string header;
    for (
        const auto &key:
        std::views::keys(data)
    ) {
        dataStrings.emplace(
            key,
            std::vector<std::string>()
        );
        header += ColumnToString(key);
        header += ',';
    }
    header.back() = '\n';
    fileSize += header.size();

    for (
        const auto &[key, value]:
        data
    ) {
        CSV::Column column = key;
        for (
            std::vector<int> series = value;
            const auto &data_point:
            series
        ) {
            auto data_point_as_text = std::to_string(data_point);
            fileSize += data_point_as_text.size() + 1;
            dataStrings[column].push_back(data_point_as_text);
        }
    }

    if (lseek(fd, fileSize - 1, SEEK_SET) == -1) {
        perror("Error seeking in file");
    }

    write(fd, "", 1);

    auto *map = static_cast<char *>(
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
