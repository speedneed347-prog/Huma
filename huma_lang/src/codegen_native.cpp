#include "huma/codegen_native.hpp"
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <iostream>

namespace huma {

std::string NativeCodeGenerator::generateRuntimeBoilerplate() {
    return R"(
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <cmath>
#include <memory>
#include <sstream>
#include <functional>

struct HumaVal {
    enum Type { V_NULL, V_BOOL, V_NUM, V_STR, V_LIST } type{V_NULL};
    bool b_val{false};
    double n_val{0.0};
    std::string s_val{""};
    std::vector<HumaVal> l_val{};

    static HumaVal makeNull() { return HumaVal(); }
    static HumaVal makeBool(bool b) { HumaVal v; v.type = V_BOOL; v.b_val = b; return v; }
    static HumaVal makeNum(double n) { HumaVal v; v.type = V_NUM; v.n_val = n; return v; }
    static HumaVal makeStr(std::string s) { HumaVal v; v.type = V_STR; v.s_val = std::move(s); return v; }
    static HumaVal makeList(std::vector<HumaVal> l) { HumaVal v; v.type = V_LIST; v.l_val = std::move(l); return v; }

    std::string str() const {
        if (type == V_NULL) return "nothing";
        if (type == V_BOOL) return b_val ? "yes" : "no";
        if (type == V_NUM) {
            if (std::floor(n_val) == n_val) return std::to_string((long long)n_val);
            std::ostringstream ss; ss << n_val; return ss.str();
        }
        if (type == V_STR) return s_val;
        if (type == V_LIST) {
            std::string res = "[";
            for (size_t i = 0; i < l_val.size(); ++i) {
                if (i > 0) res += ", ";
                res += l_val[i].str();
            }
            res += "]";
            return res;
        }
        return "";
    }

    bool truthy() const {
        if (type == V_NULL) return false;
        if (type == V_BOOL) return b_val;
        if (type == V_NUM) return n_val != 0;
        if (type == V_STR) return !s_val.empty();
        if (type == V_LIST) return !l_val.empty();
        return true;
    }

    HumaVal operator+(const HumaVal& o) const {
        if (type == V_STR || o.type == V_STR) return makeStr(str() + o.str());
        return makeNum(n_val + o.n_val);
    }
    HumaVal operator-(const HumaVal& o) const { return makeNum(n_val - o.n_val); }
    HumaVal operator*(const HumaVal& o) const { return makeNum(n_val * o.n_val); }
    HumaVal operator/(const HumaVal& o) const { return (o.n_val == 0) ? makeNum(0) : makeNum(n_val / o.n_val); }
    HumaVal operator%(const HumaVal& o) const { return (o.n_val == 0) ? makeNum(0) : makeNum(std::fmod(n_val, o.n_val)); }
    HumaVal op_pow(const HumaVal& o) const { return makeNum(std::pow(n_val, o.n_val)); }
    HumaVal op_eq(const HumaVal& o) const {
        if (type != o.type) return makeBool(false);
        if (type == V_NUM) return makeBool(n_val == o.n_val);
        if (type == V_STR) return makeBool(s_val == o.s_val);
        if (type == V_BOOL) return makeBool(b_val == o.b_val);
        return makeBool(true);
    }
    HumaVal op_gt(const HumaVal& o) const { return makeBool(n_val > o.n_val); }
    HumaVal op_lt(const HumaVal& o) const { return makeBool(n_val < o.n_val); }
    HumaVal op_gte(const HumaVal& o) const { return makeBool(n_val >= o.n_val); }
    HumaVal op_lte(const HumaVal& o) const { return makeBool(n_val <= o.n_val); }
    HumaVal op_not() const { return makeBool(!truthy()); }
    HumaVal op_neg() const { return makeNum(-n_val); }
};

static void huma_show(const HumaVal& v) {
    std::cout << v.str() << "\n";
}

)";
}

class NativeVisitor : public ASTVisitor {
public:
    std::ostringstream main_body;
    std::ostringstream func_decls;
    std::ostringstream func_defs;
    int indent_level{1};

    void indent(std::ostringstream& ss) {
        for (int i = 0; i < indent_level; ++i) ss << "    ";
    }

    std::string genExpr(Expr& expr) {
        std::ostringstream ss;
        if (auto* lit = dynamic_cast<LiteralExpr*>(&expr)) {
            switch (lit->lit_type) {
                case LiteralType::NUMBER:
                    ss << "HumaVal::makeNum(" << lit->number_val << ")";
                    break;
                case LiteralType::STRING:
                    ss << "HumaVal::makeStr(\"" << lit->string_val << "\")";
                    break;
                case LiteralType::BOOLEAN:
                    ss << "HumaVal::makeBool(" << (lit->bool_val ? "true" : "false") << ")";
                    break;
                case LiteralType::NOTHING:
                    ss << "HumaVal::makeNull()";
                    break;
            }
        } else if (auto* id = dynamic_cast<IdentifierExpr*>(&expr)) {
            ss << "var_" << id->name;
        } else if (auto* bin = dynamic_cast<BinaryExpr*>(&expr)) {
            std::string l = genExpr(*bin->left);
            std::string r = genExpr(*bin->right);
            switch (bin->op) {
                case BinaryOp::ADD: ss << "(" << l << " + " << r << ")"; break;
                case BinaryOp::SUB: ss << "(" << l << " - " << r << ")"; break;
                case BinaryOp::MUL: ss << "(" << l << " * " << r << ")"; break;
                case BinaryOp::DIV: ss << "(" << l << " / " << r << ")"; break;
                case BinaryOp::MOD: ss << "(" << l << " % " << r << ")"; break;
                case BinaryOp::POW: ss << "(" << l << ".op_pow(" << r << "))"; break;
                case BinaryOp::EQUAL: ss << "(" << l << ".op_eq(" << r << "))"; break;
                case BinaryOp::NOT_EQUAL: ss << "(" << l << ".op_eq(" << r << ").op_not())"; break;
                case BinaryOp::GREATER: ss << "(" << l << ".op_gt(" << r << "))"; break;
                case BinaryOp::LESS: ss << "(" << l << ".op_lt(" << r << "))"; break;
                case BinaryOp::GREATER_EQUAL: ss << "(" << l << ".op_gte(" << r << "))"; break;
                case BinaryOp::LESS_EQUAL: ss << "(" << l << ".op_lte(" << r << "))"; break;
                case BinaryOp::AND: ss << "HumaVal::makeBool(" << l << ".truthy() && " << r << ".truthy())"; break;
                case BinaryOp::OR: ss << "HumaVal::makeBool(" << l << ".truthy() || " << r << ".truthy())"; break;
            }
        } else if (auto* un = dynamic_cast<UnaryExpr*>(&expr)) {
            std::string opnd = genExpr(*un->operand);
            if (un->op == UnaryOp::NOT) {
                ss << "(" << opnd << ".op_not())";
            } else {
                ss << "(" << opnd << ".op_neg())";
            }
        } else if (auto* call = dynamic_cast<CallExpr*>(&expr)) {
            ss << "fn_" << call->callee << "(";
            for (size_t i = 0; i < call->args.size(); ++i) {
                if (i > 0) ss << ", ";
                ss << genExpr(*call->args[i]);
            }
            ss << ")";
        } else if (auto* list = dynamic_cast<ListExpr*>(&expr)) {
            ss << "HumaVal::makeList({";
            for (size_t i = 0; i < list->elements.size(); ++i) {
                if (i > 0) ss << ", ";
                ss << genExpr(*list->elements[i]);
            }
            ss << "})";
        } else {
            ss << "HumaVal::makeNull()";
        }
        return ss.str();
    }

    void visit(LiteralExpr&) override {}
    void visit(IdentifierExpr&) override {}
    void visit(BinaryExpr&) override {}
    void visit(UnaryExpr&) override {}
    void visit(CallExpr&) override {}
    void visit(ListExpr&) override {}
    void visit(DictExpr&) override {}
    void visit(IndexExpr&) override {}
    void visit(MemberExpr&) override {}

    void visit(VarDeclStmt& node) override {
        indent(main_body);
        main_body << "HumaVal var_" << node.name << " = " << genExpr(*node.init_expr) << ";\n";
    }

    void visit(ChangeStmt& node) override {
        indent(main_body);
        main_body << "var_" << node.target_name << " = " << genExpr(*node.value_expr) << ";\n";
    }

    void visit(ShowStmt& node) override {
        for (auto& expr : node.expressions) {
            indent(main_body);
            main_body << "huma_show(" << genExpr(*expr) << ");\n";
        }
    }

    void visit(IfStmt& node) override {
        indent(main_body);
        main_body << "if (" << genExpr(*node.condition) << ".truthy()) {\n";
        indent_level++;
        for (auto& s : node.then_branch) s->accept(*this);
        indent_level--;
        indent(main_body);
        main_body << "}";

        for (auto& elif : node.else_if_branches) {
            main_body << " else if (" << genExpr(*elif.condition) << ".truthy()) {\n";
            indent_level++;
            for (auto& s : elif.body) s->accept(*this);
            indent_level--;
            indent(main_body);
            main_body << "}";
        }

        if (!node.otherwise_branch.empty()) {
            main_body << " else {\n";
            indent_level++;
            for (auto& s : node.otherwise_branch) s->accept(*this);
            indent_level--;
            indent(main_body);
            main_body << "}";
        }
        main_body << "\n";
    }

    void visit(RepeatTimesStmt& node) override {
        indent(main_body);
        main_body << "{\n";
        indent_level++;
        indent(main_body);
        main_body << "long long __count = (long long)(" << genExpr(*node.count_expr) << ".n_val);\n";
        indent(main_body);
        main_body << "for (long long __i = 0; __i < __count; ++__i) {\n";
        indent_level++;
        for (auto& s : node.body) s->accept(*this);
        indent_level--;
        indent(main_body);
        main_body << "}\n";
        indent_level--;
        indent(main_body);
        main_body << "}\n";
    }

    void visit(WhileStmt& node) override {
        indent(main_body);
        main_body << "while (" << genExpr(*node.condition) << ".truthy()) {\n";
        indent_level++;
        for (auto& s : node.body) s->accept(*this);
        indent_level--;
        indent(main_body);
        main_body << "}\n";
    }

    void visit(ForEachStmt& node) override {
        indent(main_body);
        main_body << "{\n";
        indent_level++;
        indent(main_body);
        main_body << "HumaVal __col = " << genExpr(*node.collection_expr) << ";\n";
        indent(main_body);
        main_body << "for (const auto& var_" << node.item_name << " : __col.l_val) {\n";
        indent_level++;
        for (auto& s : node.body) s->accept(*this);
        indent_level--;
        indent(main_body);
        main_body << "}\n";
        indent_level--;
        indent(main_body);
        main_body << "}\n";
    }

    void visit(FuncDeclStmt& node) override {
        std::ostringstream sig;
        sig << "HumaVal fn_" << node.name << "(";
        for (size_t i = 0; i < node.params.size(); ++i) {
            if (i > 0) sig << ", ";
            sig << "HumaVal var_" << node.params[i];
        }
        sig << ")";

        func_decls << sig.str() << ";\n";
        func_defs << sig.str() << " {\n";

        // Save and swap main_body stream
        auto old_body = std::move(main_body);
        main_body = std::ostringstream();
        int old_indent = indent_level;
        indent_level = 1;

        for (auto& s : node.body) {
            s->accept(*this);
        }

        func_defs << main_body.str();
        func_defs << "    return HumaVal::makeNull();\n";
        func_defs << "}\n\n";

        main_body = std::move(old_body);
        indent_level = old_indent;
    }

    void visit(ReturnStmt& node) override {
        indent(main_body);
        if (node.value) {
            main_body << "return " << genExpr(*node.value) << ";\n";
        } else {
            main_body << "return HumaVal::makeNull();\n";
        }
    }

    void visit(ClassDeclStmt&) override {}
    void visit(TryCatchStmt&) override {}
    void visit(ImportStmt&) override {}
    void visit(BreakStmt&) override { indent(main_body); main_body << "break;\n"; }
    void visit(ContinueStmt&) override { indent(main_body); main_body << "continue;\n"; }
    void visit(ExprStmt& node) override {
        indent(main_body);
        main_body << genExpr(*node.expr) << ";\n";
    }

    void visit(Program& node) override {
        for (auto& s : node.statements) s->accept(*this);
    }
};

std::string NativeCodeGenerator::generateSource(Program& program) {
    NativeVisitor visitor;
    program.accept(visitor);

    std::ostringstream ss;
    ss << "// Generated by HUMA X Native Compiler (x86_64 / arm64 ELF)\n";
    ss << generateRuntimeBoilerplate();
    ss << "\n// Function Declarations\n";
    ss << visitor.func_decls.str();
    ss << "\n// Function Definitions\n";
    ss << visitor.func_defs.str();
    ss << "\nint main(int argc, char** argv) {\n";
    ss << visitor.main_body.str();
    ss << "    return 0;\n";
    ss << "}\n";
    return ss.str();
}

bool NativeCodeGenerator::compileToBinary(Program& program, const std::string& output_binary, std::string& error_out) {
    std::string cpp_source = generateSource(program);
    std::string temp_cpp = "/tmp/huma_native_temp.cpp";

    std::ofstream out(temp_cpp);
    if (!out.is_open()) {
        error_out = "Failed to create temporary C++ file for native compilation.";
        return false;
    }
    out << cpp_source;
    out.close();

    std::string cmd = "g++ -O2 -std=c++20 " + temp_cpp + " -o " + output_binary + " 2>&1";
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) {
        error_out = "Failed to invoke g++ for native binary generation.";
        return false;
    }

    char buffer[256];
    std::string result = "";
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }
    int exit_code = pclose(pipe);

    if (exit_code != 0) {
        error_out = "Native Compilation Failed:\n" + result;
        return false;
    }
    return true;
}

} // namespace huma
