#include <filesystem>

#include "file_util.h"

void move_files_recursively_to_single_folder(
    const std::string& source_folder,
    const std::string& destination_folder
) {
    // recursive_directory_iterator gets all the files
    std::filesystem::create_directory(destination_folder);
    for (
        const auto& directory_entry :
        std::filesystem::recursive_directory_iterator(source_folder)
    ) {
        if (
            const auto destination_file =
                destination_folder / directory_entry.path().filename();
            !exists(destination_file) &&
            directory_entry.is_regular_file() &&
            file_size(directory_entry.path()) > 0
        ) {
            std::filesystem::rename(
                directory_entry,
                destination_file
            );
        }
    }
}

std::vector<std::string> getAllFilesPaths(
    const std::string& folder
) {
    std::vector<std::string> file_paths;
    for (
        const auto& entry :
        std::filesystem::directory_iterator(folder)
    ) {
        if (is_regular_file(entry.path())) {
            file_paths.push_back(entry.path().string());
        }
    }
    return file_paths;
}
