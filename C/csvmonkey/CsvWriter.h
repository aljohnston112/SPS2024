#ifndef WAVELET_CSVWRITER_H
#define WAVELET_CSVWRITER_H

#include <vector>
#include <string>
#include <future>
#include <sys/mman.h>
#include <cstring>
#include <fcntl.h>
#include "../csv_util.h"

void writeCsv(
        const std::string &filePath,
        std::map<CSV::Column, std::vector<int>> &data
);

#endif //WAVELET_CSVWRITER_H
