#ifndef WAVELET_CSVWRITER_H
#define WAVELET_CSVWRITER_H

#include <vector>
#include <string>
#include <future>

void writeCsv(
        const std::string& filePath,
        std::vector<std::vector<int>>& data
);

#endif //WAVELET_CSVWRITER_H
