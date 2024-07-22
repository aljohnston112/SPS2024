#include <filesystem>

#include "../config.h"
#include "data_sanitizer.h"
#include "file_util.h"

void move_stock_data_to_single_folder() {
    std::filesystem::create_directory(sps_config::intermediate_data_folder);
    move_files_recursively_to_single_folder(
        sps_config::raw_data_folder,
        sps_config::intermediate_data_folder
    );
}
