#pragma once

#include "huma/ast.hpp"
#include <string>

namespace huma {

class WasmCodeGenerator {
public:
    WasmCodeGenerator() = default;

    std::string generateJsBundle(Program& program);
    std::string generateHtmlRunner(Program& program, const std::string& title = "HUMA X Web Runtime");
};

} // namespace huma
