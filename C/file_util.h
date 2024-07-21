#ifndef MAIN_FILE_UTIL_H
#define MAIN_FILE_UTIL_H

#include <string>
#include <vector>

/**
 * Moves all files in the source folder and all its' children recursively to the destination folder.
 * The files' folder structure in the source folder is not preserved.
 * All files will be directly in the destination folder.
 *
 * @param source_folder The source folder.
 * @param destination_folder The destination folder.
 */
void move_files_recursively_to_single_folder(
    const std::string &source_folder,
    const std::string &destination_folder
);

std::vector<std::string> getAllFilesPaths(
    const std::string &folder
);

#endif //MAIN_FILE_UTIL_H
