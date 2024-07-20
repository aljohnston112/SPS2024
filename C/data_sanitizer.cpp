#include <string>
#include "data_sanitizer.h"
#include "file_util.h"

void aggregate_stock_data_to_single_folder() {

#ifdef _WIN32
    const std::string raw_data_folder = "C:/Users/aljoh/CLionProjects/Wavelet/data/raw/";
    const std::string intermediate_data_folder = "C:/Users/aljoh/CLionProjects/Wavelet/data/intermediate/";
#else
    const std::string raw_data_folder = "/home/alexanderjohnston/CLionProjects/SPS2024/data/raw/";
    const std::string intermediate_data_folder = "/home/alexanderjohnston/CLionProjects/SPS2024/data/intermediate/";
#endif
    aggregate_files_recursively_to_single_folder(
            raw_data_folder,
            intermediate_data_folder
    );
}