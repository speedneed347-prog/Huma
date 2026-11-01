#include "huma/codegen_wasm.hpp"
#include <sstream>

namespace huma {

class WasmJsVisitor : public ASTVisitor {
public:
    std::ostringstream js_code;
    int indent_level{0};

    void indent() {
        for (int i = 0; i < indent_level; ++i) js_code << "  ";
    }

    std::string genExpr(Expr& expr) {
        std::ostringstream ss;
        if (auto* lit = dynamic_cast<LiteralExpr*>(&expr)) {
            switch (lit->lit_type) {
                case LiteralType::NUMBER:
                    ss << lit->number_val;
                    break;
                case LiteralType::STRING:
                    ss << "\"" << lit->string_val << "\"";
                    break;
                case LiteralType::BOOLEAN:
                    ss << (lit->bool_val ? "true" : "false");
                    break;
                case LiteralType::NOTHING:
                    ss << "null";
                    break;
            }
        } else if (auto* id = dynamic_cast<IdentifierExpr*>(&expr)) {
            ss << id->name;
        } else if (auto* bin = dynamic_cast<BinaryExpr*>(&expr)) {
            std::string l = genExpr(*bin->left);
            std::string r = genExpr(*bin->right);
            switch (bin->op) {
                case BinaryOp::ADD: ss << "(" << l << " + " << r << ")"; break;
                case BinaryOp::SUB: ss << "(" << l << " - " << r << ")"; break;
                case BinaryOp::MUL: ss << "(" << l << " * " << r << ")"; break;
                case BinaryOp::DIV: ss << "(" << l << " / " << r << ")"; break;
                case BinaryOp::MOD: ss << "(" << l << " % " << r << ")"; break;
                case BinaryOp::POW: ss << "Math.pow(" << l << ", " << r << ")"; break;
                case BinaryOp::EQUAL: ss << "(" << l << " === " << r << ")"; break;
                case BinaryOp::NOT_EQUAL: ss << "(" << l << " !== " << r << ")"; break;
                case BinaryOp::GREATER: ss << "(" << l << " > " << r << ")"; break;
                case BinaryOp::LESS: ss << "(" << l << " < " << r << ")"; break;
                case BinaryOp::GREATER_EQUAL: ss << "(" << l << " >= " << r << ")"; break;
                case BinaryOp::LESS_EQUAL: ss << "(" << l << " <= " << r << ")"; break;
                case BinaryOp::AND: ss << "(" << l << " && " << r << ")"; break;
                case BinaryOp::OR: ss << "(" << l << " || " << r << ")"; break;
            }
        } else if (auto* un = dynamic_cast<UnaryExpr*>(&expr)) {
            std::string opnd = genExpr(*un->operand);
            if (un->op == UnaryOp::NOT) {
                ss << "(!" << opnd << ")";
            } else {
                ss << "(-" << opnd << ")";
            }
        } else if (auto* call = dynamic_cast<CallExpr*>(&expr)) {
            ss << call->callee << "(";
            for (size_t i = 0; i < call->args.size(); ++i) {
                if (i > 0) ss << ", ";
                ss << genExpr(*call->args[i]);
            }
            ss << ")";
        } else if (auto* list = dynamic_cast<ListExpr*>(&expr)) {
            ss << "[";
            for (size_t i = 0; i < list->elements.size(); ++i) {
                if (i > 0) ss << ", ";
                ss << genExpr(*list->elements[i]);
            }
            ss << "]";
        } else if (auto* idx = dynamic_cast<IndexExpr*>(&expr)) {
            ss << genExpr(*idx->collection) << "[" << genExpr(*idx->index) << "]";
        } else {
            ss << "null";
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
        indent();
        js_code << (node.is_fixed ? "const " : "let ") << node.name << " = " << genExpr(*node.init_expr) << ";\n";
    }

    void visit(ChangeStmt& node) override {
        indent();
        js_code << node.target_name << " = " << genExpr(*node.value_expr) << ";\n";
    }

    void visit(ShowStmt& node) override {
        for (auto& expr : node.expressions) {
            indent();
            js_code << "huma_runtime.show(" << genExpr(*expr) << ");\n";
        }
    }

    void visit(IfStmt& node) override {
        indent();
        js_code << "if (" << genExpr(*node.condition) << ") {\n";
        indent_level++;
        for (auto& s : node.then_branch) s->accept(*this);
        indent_level--;
        indent();
        js_code << "}";

        for (auto& elif : node.else_if_branches) {
            js_code << " else if (" << genExpr(*elif.condition) << ") {\n";
            indent_level++;
            for (auto& s : elif.body) s->accept(*this);
            indent_level--;
            indent();
            js_code << "}";
        }

        if (!node.otherwise_branch.empty()) {
            js_code << " else {\n";
            indent_level++;
            for (auto& s : node.otherwise_branch) s->accept(*this);
            indent_level--;
            indent();
            js_code << "}";
        }
        js_code << "\n";
    }

    void visit(RepeatTimesStmt& node) override {
        indent();
        js_code << "for (let __i = 0, __max = " << genExpr(*node.count_expr) << "; __i < __max; ++__i) {\n";
        indent_level++;
        for (auto& s : node.body) s->accept(*this);
        indent_level--;
        indent();
        js_code << "}\n";
    }

    void visit(WhileStmt& node) override {
        indent();
        js_code << "while (" << genExpr(*node.condition) << ") {\n";
        indent_level++;
        for (auto& s : node.body) s->accept(*this);
        indent_level--;
        indent();
        js_code << "}\n";
    }

    void visit(ForEachStmt& node) override {
        indent();
        js_code << "for (const " << node.item_name << " of " << genExpr(*node.collection_expr) << ") {\n";
        indent_level++;
        for (auto& s : node.body) s->accept(*this);
        indent_level--;
        indent();
        js_code << "}\n";
    }

    void visit(FuncDeclStmt& node) override {
        indent();
        js_code << "function " << node.name << "(";
        for (size_t i = 0; i < node.params.size(); ++i) {
            if (i > 0) js_code << ", ";
            js_code << node.params[i];
        }
        js_code << ") {\n";
        indent_level++;
        for (auto& s : node.body) s->accept(*this);
        indent_level--;
        indent();
        js_code << "}\n";
    }

    void visit(ReturnStmt& node) override {
        indent();
        if (node.value) {
            js_code << "return " << genExpr(*node.value) << ";\n";
        } else {
            js_code << "return null;\n";
        }
    }

    void visit(ClassDeclStmt&) override {}
    void visit(TryCatchStmt&) override {}
    void visit(ImportStmt&) override {}
    void visit(BreakStmt&) override { indent(); js_code << "break;\n"; }
    void visit(ContinueStmt&) override { indent(); js_code << "continue;\n"; }
    void visit(ExprStmt& node) override {
        indent();
        js_code << genExpr(*node.expr) << ";\n";
    }

    void visit(Program& node) override {
        for (auto& s : node.statements) s->accept(*this);
    }
};

std::string WasmCodeGenerator::generateJsBundle(Program& program) {
    WasmJsVisitor visitor;
    program.accept(visitor);

    std::ostringstream ss;
    ss << R"(// HUMA X WebAssembly / JavaScript Target Bundle
(function() {
  const huma_runtime = {
    stdout: [],
    show(val) {
      const out = val === null ? 'nothing' : (val === true ? 'yes' : (val === false ? 'no' : String(val)));
      this.stdout.push(out);
      if (typeof console !== 'undefined') console.log(out);
    }
  };

  // Standard Library functions
  const length = (x) => (x ? (x.length !== undefined ? x.length : Object.keys(x).length) : 0);
  const sqrt = Math.sqrt;
  const pow = Math.pow;
  const abs = Math.abs;
  const to_string = String;
  const to_number = Number;

  // Program Logic
)" << visitor.js_code.str() << R"(
  if (typeof window !== 'undefined') {
    window.__huma_last_output = huma_runtime.stdout.join('\n');
  }
  return huma_runtime.stdout;
})();
)";
    return ss.str();
}

std::string WasmCodeGenerator::generateHtmlRunner(Program& program, const std::string& title) {
    std::string js = generateJsBundle(program);
    std::ostringstream ss;
    ss << "<!DOCTYPE html>\n<html>\n<head>\n  <title>" << title << "</title>\n</head>\n<body>\n";
    ss << "  <pre id=\"output\"></pre>\n";
    ss << "  <script>\n";
    ss << "  const outputElem = document.getElementById('output');\n";
    ss << js << "\n";
    ss << "  if (window.__huma_last_output) outputElem.textContent = window.__huma_last_output;\n";
    ss << "  </script>\n";
    ss << "</body>\n</html>\n";
    return ss.str();
}

} // namespace huma
