#pragma once

#include "huma/vm.hpp"
#include <string>

namespace huma {

class REPL {
public:
    REPL() = default;
    void run();
    std::string evaluateLine(const std::string& input, VM& vm);
};

} // namespace huma
