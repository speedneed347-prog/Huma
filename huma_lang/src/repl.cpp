#include "huma/repl.hpp"
#include <iostream>

namespace huma {

void REPL::run() {
    VM vm;
    std::cout << "HUMA X Interactive REPL (v1.0.0)\n";
    std::cout << "Human-Oriented Universal Machine Language\n";
    std::cout << "Type 'exit' to quit, 'help' for instructions.\n\n";

    std::string line;
    std::string buffer;
    while (true) {
        if (buffer.empty()) {
            std::cout << "huma> ";
        } else {
            std::cout << " ...> ";
        }

        if (!std::getline(std::cin, line)) {
            break;
        }

        if (buffer.empty() && (line == "exit" || line == "quit")) {
            std::cout << "Goodbye!\n";
            break;
        }

        if (buffer.empty() && line == "help") {
            std::cout << "Quick HUMA X syntax:\n";
            std::cout << "  make x = 42\n";
            std::cout << "  show x\n";
            std::cout << "  repeat 3 times show \"Hi\" end\n\n";
            continue;
        }

        buffer += line + "\n";

        // Check if multi-line block is open
        if (line.rfind("if ", 0) == 0 || line.rfind("repeat ", 0) == 0 ||
            line.rfind("create ", 0) == 0 || line.rfind("try", 0) == 0) {
            continue; // Wait for 'end'
        }

        std::string result = evaluateLine(buffer, vm);
        if (!result.empty()) {
            std::cout << result;
        }
        buffer.clear();
    }
}

std::string REPL::evaluateLine(const std::string& input, VM& vm) {
    auto res = vm.interpretSource(input, "<repl>");
    if (res == InterpretResult::OK) {
        return vm.getStdout();
    } else {
        return vm.getStderr();
    }
}

} // namespace huma
