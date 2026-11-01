#pragma once

#include <string>
#include <vector>

namespace huma {

struct PackageMeta {
    std::string name;
    std::string version{"0.1.0"};
    std::string author;
    std::string description;
    std::string entry_point{"src/main.huma"};
    std::vector<std::string> dependencies;
};

class PackageManager {
public:
    static bool initProject(const std::string& project_dir, const std::string& project_name);
    static bool buildProject(const std::string& project_dir, const std::string& target = "vm");
    static bool runProject(const std::string& project_dir);
    static bool testProject(const std::string& project_dir);
    static PackageMeta readPackage(const std::string& pkg_file);
};

} // namespace huma
