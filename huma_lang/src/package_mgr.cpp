#include "huma/package_mgr.hpp"
#include <fstream>
#include <filesystem>
#include <iostream>

namespace huma {

namespace fs = std::filesystem;

bool PackageManager::initProject(const std::string& project_dir, const std::string& project_name) {
    try {
        fs::create_directories(project_dir + "/src");
        fs::create_directories(project_dir + "/tests");

        // Write huma.pkg
        std::ofstream pkg_file(project_dir + "/huma.pkg");
        pkg_file << "name: " << project_name << "\n";
        pkg_file << "version: 0.1.0\n";
        pkg_file << "entry: src/main.huma\n";
        pkg_file << "description: HUMA X project\n";
        pkg_file.close();

        // Write src/main.huma
        std::ofstream main_file(project_dir + "/src/main.huma");
        main_file << "# " << project_name << " Entry Point\n\n";
        main_file << "make name = \"" << project_name << "\"\n";
        main_file << "show \"Welcome to \", name\n";
        main_file.close();

        // Write tests/test_main.huma
        std::ofstream test_file(project_dir + "/tests/test_main.huma");
        test_file << "# Unit Tests\n";
        test_file << "make passed = yes\n";
        test_file << "assert passed, \"Basic test passed\"\n";
        test_file << "show \"All tests passed!\"\n";
        test_file.close();

        return true;
    } catch (...) {
        return false;
    }
}

PackageMeta PackageManager::readPackage(const std::string& pkg_file) {
    PackageMeta meta;
    std::ifstream file(pkg_file);
    if (!file.is_open()) return meta;

    std::string line;
    while (std::getline(file, line)) {
        size_t colon = line.find(':');
        if (colon != std::string::npos) {
            std::string key = line.substr(0, colon);
            std::string val = line.substr(colon + 1);
            while (!val.empty() && val.front() == ' ') val.erase(0, 1);
            if (key == "name") meta.name = val;
            else if (key == "version") meta.version = val;
            else if (key == "entry") meta.entry_point = val;
            else if (key == "description") meta.description = val;
        }
    }
    return meta;
}

} // namespace huma
