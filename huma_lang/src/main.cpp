#include "huma/common.hpp"
#include "huma/lexer.hpp"
#include "huma/parser.hpp"
#include "huma/semantic.hpp"
#include "huma/ir.hpp"
#include "huma/bytecode.hpp"
#include "huma/vm.hpp"
#include "huma/codegen_native.hpp"
#include "huma/codegen_wasm.hpp"
#include "huma/formatter.hpp"
#include "huma/repl.hpp"
#include "huma/docgen.hpp"
#include "huma/package_mgr.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <chrono>

namespace fs = std::filesystem;

static const char* const HUMA_VERSION = "1.0.0";
static const char* const HUMA_BUILD_DATE = "2026-11-01";

static void printUsage() {
    std::cout << "\033[1;36mHUMA X\033[0m — Human-Oriented Universal Machine Language (v" << HUMA_VERSION << ")\n\n";
    std::cout << "Usage: huma <command> [options] [arguments]\n\n";
    std::cout << "Commands:\n";
    std::cout << "  version              Display compiler & runtime versions\n";
    std::cout << "  run <file>           Execute a HUMA program (VM or native)\n";
    std::cout << "  check <file>         Validate syntax & type semantics\n";
    std::cout << "  build <file>         Build native binary executable\n";
    std::cout << "  compile <file>       Compile to specified target (native, wasm, bytecode)\n";
    std::cout << "  test [dir]           Execute automated test suite\n";
    std::cout << "  format <file>        Format HUMA source code\n";
    std::cout << "  repl                 Interactive HUMA shell\n";
    std::cout << "  package [init|read]  Manage HUMA packages & dependencies\n";
    std::cout << "  docs <file>          Generate markdown & HTML documentation\n";
    std::cout << "  dump-ast <file>      Display parsed Abstract Syntax Tree\n";
    std::cout << "  dump-ir <file>       Display Intermediate Representation (TAC)\n";
    std::cout << "  dump-bytecode <file> Display disassembled bytecode instructions\n\n";
    std::cout << "Options:\n";
    std::cout << "  -o, --output <file>  Specify output file path\n";
    std::cout << "  --target <target>    Target: 'vm', 'native', 'wasm' (default: vm)\n";
    std::cout << "  --json               Output machine-readable JSON (for HUMA Studio IDE)\n";
    std::cout << "  --help               Display this help text\n";
}

static std::string readFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file '" << path << "'\n";
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

static std::string escapeJson(const std::string& s) {
    std::ostringstream o;
    for (char c : s) {
        if (c == '"') o << "\\\"";
        else if (c == '\\') o << "\\\\";
        else if (c == '\b') o << "\\b";
        else if (c == '\f') o << "\\f";
        else if (c == '\n') o << "\\n";
        else if (c == '\r') o << "\\r";
        else if (c == '\t') o << "\\t";
        else if (static_cast<unsigned char>(c) <= '\x1f') {
            o << "\\u00" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(c);
        } else {
            o << c;
        }
    }
    return o.str();
}

int main(int argc, char** argv) {
    if (argc < 2) {
        printUsage();
        return 1;
    }

    std::string cmd = argv[1];

    if (cmd == "--help" || cmd == "-h" || cmd == "help") {
        printUsage();
        return 0;
    }

    if (cmd == "version" || cmd == "--version" || cmd == "-v") {
        std::cout << "HUMA X Compiler & Runtime Ecosystem\n";
        std::cout << "Version: " << HUMA_VERSION << " (Build: " << HUMA_BUILD_DATE << ")\n";
        std::cout << "Target architecture: Linux x86-64 / GCC 12 backend\n";
        std::cout << "Language Standard: HUMA X Specification 2026\n";
        return 0;
    }

    if (cmd == "repl") {
        huma::REPL repl;
        repl.run();
        return 0;
    }

    // Parse options
    std::string file_path = "";
    std::string target = "vm";
    std::string output_path = "";
    bool json_mode = false;

    for (int i = 2; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--json") {
            json_mode = true;
        } else if (arg == "--target" && i + 1 < argc) {
            target = argv[++i];
        } else if ((arg == "-o" || arg == "--output") && i + 1 < argc) {
            output_path = argv[++i];
        } else if (arg.rfind("-", 0) != 0 && file_path.empty()) {
            file_path = arg;
        }
    }

    if (cmd == "package") {
        std::string sub = (argc >= 3) ? argv[2] : "init";
        if (sub == "init") {
            std::string name = (argc >= 4) ? argv[3] : "my_huma_project";
            if (huma::PackageManager::initProject(".", name)) {
                std::cout << "Successfully initialized HUMA X package: " << name << "\n";
                return 0;
            } else {
                std::cerr << "Failed to initialize HUMA X package.\n";
                return 1;
            }
        }
        return 0;
    }

    if (file_path.empty()) {
        std::cerr << "Error: No input file specified for command '" << cmd << "'.\n";
        return 1;
    }

    std::string source = readFile(file_path);
    if (source.empty() && !fs::exists(file_path)) {
        return 1;
    }

    huma::DiagnosticEngine diag;
    huma::Lexer lexer(source, file_path, &diag);
    auto tokens = lexer.tokenize();

    huma::Parser parser(tokens, diag, source);
    auto program = parser.parse();

    huma::SemanticAnalyzer sema(diag);
    if (program) {
        sema.analyze(*program);
    }

    if (cmd == "check") {
        if (json_mode) {
            std::cout << "{\n  \"success\": " << (diag.hasErrors() ? "false" : "true") << ",\n  \"diagnostics\": [\n";
            const auto& diags = diag.getDiagnostics();
            for (size_t i = 0; i < diags.size(); ++i) {
                const auto& d = diags[i];
                std::cout << "    {\n";
                std::cout << "      \"level\": \"" << (d.level == huma::DiagnosticLevel::ERROR ? "error" : "warning") << "\",\n";
                std::cout << "      \"message\": \"" << escapeJson(d.message) << "\",\n";
                std::cout << "      \"line\": " << d.loc.line << ",\n";
                std::cout << "      \"column\": " << d.loc.column << ",\n";
                std::cout << "      \"suggestion\": \"" << escapeJson(d.suggestion) << "\"\n";
                std::cout << "    }" << (i + 1 < diags.size() ? "," : "") << "\n";
            }
            std::cout << "  ]\n}\n";
        } else {
            if (diag.hasErrors()) {
                for (const auto& d : diag.getDiagnostics()) {
                    std::cerr << d.toString() << "\n";
                }
                return 1;
            } else {
                std::cout << "\033[1;32m✓ Syntax & Semantics check passed!\033[0m No errors found in " << file_path << "\n";
            }
        }
        return diag.hasErrors() ? 1 : 0;
    }

    if (cmd == "format") {
        huma::Formatter formatter;
        std::string formatted = formatter.formatSource(source);
        if (!output_path.empty()) {
            std::ofstream out(output_path);
            out << formatted;
            std::cout << "Formatted code saved to: " << output_path << "\n";
        } else {
            std::cout << formatted;
        }
        return 0;
    }

    if (cmd == "dump-ast") {
        std::cout << "=== HUMA X Abstract Syntax Tree ===\n";
        if (program) {
            std::cout << "Program with " << program->statements.size() << " top-level statements parsed successfully.\n";
            huma::Formatter fmt;
            std::cout << fmt.format(*program);
        }
        return 0;
    }

    if (cmd == "dump-ir") {
        if (diag.hasErrors()) {
            for (const auto& d : diag.getDiagnostics()) std::cerr << d.toString() << "\n";
            return 1;
        }
        huma::IRGenerator ir_gen;
        auto ir_prog = ir_gen.generate(*program);
        std::cout << ir_prog.dump();
        return 0;
    }

    if (cmd == "dump-bytecode") {
        if (diag.hasErrors()) {
            for (const auto& d : diag.getDiagnostics()) std::cerr << d.toString() << "\n";
            return 1;
        }
        huma::BytecodeCompiler compiler(diag);
        auto fn = compiler.compile(*program);
        if (fn) {
            std::cout << fn->chunk.disassemble(file_path);
        }
        return 0;
    }

    if (cmd == "docs") {
        if (!program) return 1;
        std::string doc = huma::DocGenerator::generateMarkdown(*program, fs::path(file_path).stem().string());
        if (!output_path.empty()) {
            std::ofstream out(output_path);
            out << doc;
            std::cout << "Documentation saved to: " << output_path << "\n";
        } else {
            std::cout << doc;
        }
        return 0;
    }

    if (cmd == "compile" || cmd == "build") {
        if (diag.hasErrors()) {
            for (const auto& d : diag.getDiagnostics()) std::cerr << d.toString() << "\n";
            return 1;
        }

        std::string effective_target = (cmd == "build") ? "native" : target;
        if (output_path.empty()) {
            output_path = fs::path(file_path).stem().string();
            if (effective_target == "native") output_path += ".bin";
            else if (effective_target == "wasm") output_path += ".js";
            else if (effective_target == "bytecode") output_path += ".humac";
        }

        if (effective_target == "native") {
            std::cout << "Compiling " << file_path << " to native executable (" << output_path << ")...\n";
            huma::NativeCodeGenerator native_gen;
            std::string err;
            if (native_gen.compileToBinary(*program, output_path, err)) {
                std::cout << "\033[1;32m✓ Native compilation successful!\033[0m Executable created: " << output_path << "\n";
                return 0;
            } else {
                std::cerr << err << "\n";
                return 1;
            }
        } else if (effective_target == "wasm") {
            huma::WasmCodeGenerator wasm_gen;
            std::string js = wasm_gen.generateJsBundle(*program);
            std::ofstream out(output_path);
            out << js;
            std::cout << "\033[1;32m✓ WebAssembly/JS compilation successful!\033[0m Saved to " << output_path << "\n";
            return 0;
        } else {
            // Bytecode
            huma::BytecodeCompiler compiler(diag);
            auto fn = compiler.compile(*program);
            std::ofstream out(output_path);
            out << fn->chunk.disassemble(file_path);
            std::cout << "\033[1;32m✓ Bytecode compilation successful!\033[0m Saved to " << output_path << "\n";
            return 0;
        }
    }

    if (cmd == "run") {
        if (target == "native") {
            // Compile and run native binary
            huma::NativeCodeGenerator native_gen;
            std::string temp_bin = "/tmp/huma_run_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
            std::string err;
            if (!native_gen.compileToBinary(*program, temp_bin, err)) {
                std::cerr << err << "\n";
                return 1;
            }
            int exit_code = std::system(temp_bin.c_str());
            fs::remove(temp_bin);
            return exit_code;
        }

        // Default: VM execution
        huma::VM vm;
        auto res = vm.interpretSource(source, file_path);

        if (json_mode) {
            std::cout << "{\n";
            std::cout << "  \"status\": \"" << (res == huma::InterpretResult::OK ? "success" : "error") << "\",\n";
            std::cout << "  \"stdout\": \"" << escapeJson(vm.getStdout()) << "\",\n";
            std::cout << "  \"stderr\": \"" << escapeJson(vm.getStderr()) << "\"\n";
            std::cout << "}\n";
        } else {
            if (!vm.getStdout().empty()) {
                std::cout << vm.getStdout();
            }
            if (!vm.getStderr().empty()) {
                std::cerr << vm.getStderr();
            }
        }
        return (res == huma::InterpretResult::OK) ? 0 : 1;
    }

    std::cerr << "Unknown command: '" << cmd << "'. Run 'huma --help' for usage.\n";
    return 1;
}
