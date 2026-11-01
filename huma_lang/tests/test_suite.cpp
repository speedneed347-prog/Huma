#include "huma/common.hpp"
#include "huma/lexer.hpp"
#include "huma/parser.hpp"
#include "huma/semantic.hpp"
#include "huma/ir.hpp"
#include "huma/bytecode.hpp"
#include "huma/vm.hpp"
#include "huma/codegen_native.hpp"
#include <iostream>
#include <cassert>
#include <cmath>

static int total_tests = 0;
static int passed_tests = 0;

#define TEST_ASSERT(cond, name) do { \
    total_tests++; \
    if (cond) { \
        passed_tests++; \
        std::cout << "  \033[1;32m✓\033[0m " << name << "\n"; \
    } else { \
        std::cerr << "  \033[1;31m✗\033[0m " << name << " (FAILED at " << __FILE__ << ":" << __LINE__ << ")\n"; \
    } \
} while(0)

void test_lexer() {
    std::cout << "\n[TEST SUITE] Lexer Tests\n";
    std::string src = "make x = 42\nshow \"Hello\", x\nchange x to x + 1\n";
    huma::Lexer lexer(src, "test.huma");
    auto tokens = lexer.tokenize();

    TEST_ASSERT(!tokens.empty(), "Tokens not empty");
    TEST_ASSERT(tokens[0].type == huma::TokenType::MAKE, "First token is MAKE");
    TEST_ASSERT(tokens[1].type == huma::TokenType::IDENTIFIER && tokens[1].text == "x", "Variable name token is 'x'");
    TEST_ASSERT(tokens[2].type == huma::TokenType::EQUAL_SIGN, "Equal token");
    TEST_ASSERT(tokens[3].type == huma::TokenType::NUMBER && tokens[3].number_val == 42, "Number token 42");
}

void test_parser() {
    std::cout << "\n[TEST SUITE] Parser Tests\n";
    std::string src = "make a = 10\nmake b = 20\nmake c = a + b\n";
    huma::DiagnosticEngine diag;
    huma::Lexer lexer(src, "test.huma", &diag);
    auto tokens = lexer.tokenize();
    huma::Parser parser(tokens, diag, src);
    auto program = parser.parse();

    TEST_ASSERT(program != nullptr, "Parsed program not null");
    TEST_ASSERT(!diag.hasErrors(), "No parse diagnostics errors");
    TEST_ASSERT(program->statements.size() == 3, "Parsed exactly 3 variable declarations");
}

void test_semantic_typo_suggestion() {
    std::cout << "\n[TEST SUITE] Semantic Analyzer & Typo Diagnostics Tests\n";
    std::string src = "make amount = 100\nshow amout\n"; // Typo in 'amout'
    huma::DiagnosticEngine diag;
    huma::Lexer lexer(src, "test.huma", &diag);
    auto tokens = lexer.tokenize();
    huma::Parser parser(tokens, diag, src);
    auto program = parser.parse();

    huma::SemanticAnalyzer sema(diag);
    sema.analyze(*program);

    TEST_ASSERT(diag.hasErrors(), "Semantic error detected for undefined variable");
    const auto& diags = diag.getDiagnostics();
    bool found_suggestion = false;
    for (const auto& d : diags) {
        if (d.suggestion.find("amount") != std::string::npos) {
            found_suggestion = true;
        }
    }
    TEST_ASSERT(found_suggestion, "Suggestion correctly identifies 'Did you mean 'amount'?' via Levenshtein distance");
}

void test_ir_and_optimization() {
    std::cout << "\n[TEST SUITE] Intermediate Representation & Optimizer Tests\n";
    std::string src = "make x = 10 + 20\n";
    huma::DiagnosticEngine diag;
    huma::Lexer lexer(src, "test.huma", &diag);
    auto tokens = lexer.tokenize();
    huma::Parser parser(tokens, diag, src);
    auto program = parser.parse();

    huma::IRGenerator ir_gen;
    auto ir_prog = ir_gen.generate(*program);
    std::string dumped = ir_prog.dump();

    TEST_ASSERT(!ir_prog.instructions.empty(), "IR instructions generated");
    TEST_ASSERT(dumped.find("TAC") != std::string::npos, "IR dumped successfully");
}

void test_vm_execution() {
    std::cout << "\n[TEST SUITE] Virtual Machine Execution Tests\n";

    // 1. Hello World smoke test
    {
        huma::VM vm;
        std::string src = "show \"Hello, HUMA X!\"\n";
        auto res = vm.interpretSource(src);
        TEST_ASSERT(res == huma::InterpretResult::OK, "Smoke test: Hello World interprets OK");
        TEST_ASSERT(vm.getStdout() == "Hello, HUMA X!\n", "Smoke test: Hello World output matches");
    }

    // 2. Variables and Arithmetic
    {
        huma::VM vm;
        std::string src = "make a = 5\nmake b = 10\nmake c = a * b + 2\nshow c\n";
        auto res = vm.interpretSource(src);
        TEST_ASSERT(res == huma::InterpretResult::OK, "Variables and arithmetic execute OK");
        TEST_ASSERT(vm.getStdout() == "52\n", "Arithmetic result is 52");
    }

    // 3. Conditions (if / otherwise)
    {
        huma::VM vm;
        std::string src = "make age = 19\nif age is greater than 18 then\nshow \"Adult\"\notherwise\nshow \"Underage\"\nend\n";
        auto res = vm.interpretSource(src);
        TEST_ASSERT(res == huma::InterpretResult::OK, "Condition statement executes OK");
        TEST_ASSERT(vm.getStdout() == "Adult\n", "If-branch selected: 'Adult'");
    }

    // 4. Loops (repeat N times)
    {
        huma::VM vm;
        std::string src = "make count = 0\nrepeat 3 times\nchange count to count + 1\nend\nshow count\n";
        auto res = vm.interpretSource(src);
        TEST_ASSERT(res == huma::InterpretResult::OK, "Repeat N times loop executes OK");
        TEST_ASSERT(vm.getStdout() == "3\n", "Loop counter executed 3 times");
    }

    // 5. Functions & Recursion
    {
        huma::VM vm;
        std::string src = R"(
create function add using a, b
give back a + b
end

make res = call add using 15, 25
show res
)";
        auto res = vm.interpretSource(src);
        TEST_ASSERT(res == huma::InterpretResult::OK, "Function declaration & call execute OK");
        TEST_ASSERT(vm.getStdout() == "40\n", "Function result is 40");
    }

    // 6. Collections (Lists and Indexing)
    {
        huma::VM vm;
        std::string src = "make items = [10, 20, 30]\nshow items[1]\n";
        auto res = vm.interpretSource(src);
        TEST_ASSERT(res == huma::InterpretResult::OK, "List indexing executes OK");
        TEST_ASSERT(vm.getStdout() == "20\n", "List index 1 is 20");
    }

    // 7. Standard Library (Math & Text)
    {
        huma::VM vm;
        std::string src = "make root = sqrt(16)\nmake greeting = to_upper(\"huma\")\nshow root\nshow greeting\n";
        auto res = vm.interpretSource(src);
        TEST_ASSERT(res == huma::InterpretResult::OK, "Standard library math & text execute OK");
        TEST_ASSERT(vm.getStdout() == "4\nHUMA\n", "sqrt(16) is 4 and to_upper is HUMA");
    }
}

void test_native_compilation() {
    std::cout << "\n[TEST SUITE] Native Machine-Code Compilation Tests\n";
    std::string src = R"(
make name = "Manojit"
make age = 18
show "Native Hello, " + name
change age to age + 1
if age is greater than 18 then
show "Adult"
otherwise
show "Underage"
end
)";
    huma::DiagnosticEngine diag;
    huma::Lexer lexer(src, "native_test.huma", &diag);
    auto tokens = lexer.tokenize();
    huma::Parser parser(tokens, diag, src);
    auto program = parser.parse();

    huma::NativeCodeGenerator native_gen;
    std::string temp_bin = "/tmp/huma_native_unit_test.bin";
    std::string err;
    bool ok = native_gen.compileToBinary(*program, temp_bin, err);

    TEST_ASSERT(ok, "Native compilation with g++ -O2 produced executable ELF binary");

    if (ok) {
        FILE* pipe = popen(temp_bin.c_str(), "r");
        TEST_ASSERT(pipe != nullptr, "Native binary can be executed");
        if (pipe) {
            char buf[128];
            std::string output = "";
            while (fgets(buf, sizeof(buf), pipe) != nullptr) {
                output += buf;
            }
            pclose(pipe);
            TEST_ASSERT(output.find("Native Hello, Manojit") != std::string::npos, "Native execution output matches expected greeting");
            TEST_ASSERT(output.find("Adult") != std::string::npos, "Native execution conditional logic works");
        }
        std::remove(temp_bin.c_str());
    }
}

int main() {
    std::cout << "========================================\n";
    std::cout << "  HUMA X Automated Test Suite Runner\n";
    std::cout << "========================================\n";

    test_lexer();
    test_parser();
    test_semantic_typo_suggestion();
    test_ir_and_optimization();
    test_vm_execution();
    test_native_compilation();

    std::cout << "\n========================================\n";
    std::cout << "Test Results: " << passed_tests << " / " << total_tests << " passed.\n";
    if (passed_tests == total_tests) {
        std::cout << "\033[1;32mALL TESTS PASSED SUCCESSFULLY!\033[0m\n";
        std::cout << "========================================\n";
        return 0;
    } else {
        std::cerr << "\033[1;31mSOME TESTS FAILED!\033[0m\n";
        std::cout << "========================================\n";
        return 1;
    }
}
