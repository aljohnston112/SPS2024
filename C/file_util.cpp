#include <filesystem>
#include "file_util.h"

namespace fs = std::filesystem;

void aggregate_files_recursively_to_single_folder(
        const std::string &source_folder,
        const std::string &destination_folder
) {
    const fs::path source_path(source_folder);
    const fs::path destination_path(destination_folder);

    // recursive_directory_iterator gets all the files
    fs::create_directory(destination_path);
    for (const auto &directory_entry: fs::recursive_directory_iterator(source_path)) {
        const auto destination_file = destination_folder / directory_entry.path().filename();
        if(!fs::exists(destination_file) && directory_entry.is_regular_file() && fs::file_size(directory_entry.path()) > 0) {
            fs::rename(
                    directory_entry,
                    destination_file
            );
        }
    }
}

std::vector<std::string> getAllFilesPaths(const std::string& folder){
    std::vector<std::string> file_paths;
    for (
        const auto &entry:
            std::filesystem::directory_iterator(folder)
            ) {
        if (std::filesystem::is_regular_file(entry.path())) {
            file_paths.push_back(entry.path().string());
        }
    }
    return file_paths;
}