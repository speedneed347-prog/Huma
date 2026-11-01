#pragma once

#include "huma/ast.hpp"
#include <string>

namespace huma {

class DocGenerator {
public:
    static std::string generateMarkdown(Program& program, const std::string& project_name = "HUMA Project");
    static std::string generateHtml(Program& program, const std::string& project_name = "HUMA Project");
};

} // namespace huma
