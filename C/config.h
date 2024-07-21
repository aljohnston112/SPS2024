#ifndef CONFIG_H
#define CONFIG_H

#include <string>

namespace sps_config {

#ifdef _WIN32
    const std::string raw_data_folder = "C:/Users/aljoh/CLionProjects/Wavelet/data/raw/";
    const std::string intermediate_data_folder = "C:/Users/aljoh/CLionProjects/Wavelet/data/intermediate/";
#else
    const std::string username = std::getenv("USER");
    const std::string base_data_folder = "/home/" + username + "/CLionProjects/SPS2024/data/";
    const std::string raw_data_folder = base_data_folder + "raw/";
    const std::string intermediate_data_folder = base_data_folder + "intermediate/";
    const std::string final_data_folder = base_data_folder + "final/";
    const std::string direction_data_folder = final_data_folder + "direction/";
    const std::string results_data_folder = final_data_folder + "results/";
#endif
}

#endif //CONFIG_H
