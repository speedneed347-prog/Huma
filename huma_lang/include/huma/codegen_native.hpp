#pragma once

#include "huma/ast.hpp"
#include <string>

namespace huma {

class NativeCodeGenerator {
public:
    NativeCodeGenerator() = default;

    std::string generateSource(Program& program);
    bool compileToBinary(Program& program, const std::string& output_binary, std::string& error_out);

private:
    std::string generateRuntimeBoilerplate();
};

} // namespace huma
