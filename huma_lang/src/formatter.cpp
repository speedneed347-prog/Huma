#include "huma/formatter.hpp"
#include "huma/lexer.hpp"
#include "huma/parser.hpp"

namespace huma {

void Formatter::indent() { indent_level_++; }
void Formatter::dedent() { if (indent_level_ > 0) indent_level_--; }

void Formatter::emit(const std::string& text) {
    current_line_ += text;
}

void Formatter::emitLine(const std::string& text) {
    if (!text.empty()) emit(text);
    if (!current_line_.empty()) {
        for (int i = 0; i < indent_level_; ++i) out_ << "    ";
        out_ << current_line_ << "\n";
        current_line_.clear();
    } else {
        out_ << "\n";
    }
}

std::string Formatter::format(Program& program) {
    out_.str("");
    indent_level_ = 0;
    current_line_.clear();
    program.accept(*this);
    return out_.str();
}

std::string Formatter::formatSource(const std::string& source) {
    DiagnosticEngine diag;
    Lexer lexer(source, "format.huma", &diag);
    auto tokens = lexer.tokenize();
    Parser parser(tokens, diag, source);
    auto program = parser.parse();
    if (!program || diag.hasErrors()) {
        return source; // return original if syntax error
    }
    return format(*program);
}

void Formatter::visit(LiteralExpr& node) {
    switch (node.lit_type) {
        case LiteralType::NUMBER:
            if (node.is_integer) emit(std::to_string(static_cast<int64_t>(node.number_val)));
            else emit(std::to_string(node.number_val));
            break;
        case LiteralType::STRING:
            emit("\"" + node.string_val + "\"");
            break;
        case LiteralType::BOOLEAN:
            emit(node.bool_val ? "yes" : "no");
            break;
        case LiteralType::NOTHING:
            emit("nothing");
            break;
    }
}

void Formatter::visit(IdentifierExpr& node) {
    emit(node.name);
}

void Formatter::visit(BinaryExpr& node) {
    node.left->accept(*this);
    switch (node.op) {
        case BinaryOp::ADD: emit(" + "); break;
        case BinaryOp::SUB: emit(" - "); break;
        case BinaryOp::MUL: emit(" * "); break;
        case BinaryOp::DIV: emit(" / "); break;
        case BinaryOp::MOD: emit(" % "); break;
        case BinaryOp::POW: emit(" ^ "); break;
        case BinaryOp::EQUAL: emit(" is equal to "); break;
        case BinaryOp::NOT_EQUAL: emit(" is not equal to "); break;
        case BinaryOp::GREATER: emit(" is greater than "); break;
        case BinaryOp::LESS: emit(" is less than "); break;
        case BinaryOp::GREATER_EQUAL: emit(" is greater than or equal to "); break;
        case BinaryOp::LESS_EQUAL: emit(" is less than or equal to "); break;
        case BinaryOp::AND: emit(" and "); break;
        case BinaryOp::OR: emit(" or "); break;
    }
    node.right->accept(*this);
}

void Formatter::visit(UnaryExpr& node) {
    if (node.op == UnaryOp::NOT) emit("not ");
    else emit("-");
    node.operand->accept(*this);
}

void Formatter::visit(CallExpr& node) {
    emit("call " + node.callee);
    if (!node.args.empty()) {
        emit(" using ");
        for (size_t i = 0; i < node.args.size(); ++i) {
            if (i > 0) emit(", ");
            node.args[i]->accept(*this);
        }
    }
}

void Formatter::visit(ListExpr& node) {
    emit("[");
    for (size_t i = 0; i < node.elements.size(); ++i) {
        if (i > 0) emit(", ");
        node.elements[i]->accept(*this);
    }
    emit("]");
}

void Formatter::visit(DictExpr& node) {
    emit("{");
    for (size_t i = 0; i < node.entries.size(); ++i) {
        if (i > 0) emit(", ");
        emit("\"" + node.entries[i].first + "\": ");
        node.entries[i].second->accept(*this);
    }
    emit("}");
}

void Formatter::visit(IndexExpr& node) {
    node.collection->accept(*this);
    emit("[");
    node.index->accept(*this);
    emit("]");
}

void Formatter::visit(MemberExpr& node) {
    node.object->accept(*this);
    emit("." + node.member);
}

void Formatter::visit(VarDeclStmt& node) {
    emit(node.is_fixed ? "fixed " : "make ");
    emit(node.name);
    if (!node.type_annotation.empty()) {
        emit(" as " + node.type_annotation);
    }
    emit(" = ");
    node.init_expr->accept(*this);
    emitLine();
}

void Formatter::visit(ChangeStmt& node) {
    emit("change " + node.target_name + " to ");
    node.value_expr->accept(*this);
    emitLine();
}

void Formatter::visit(ShowStmt& node) {
    emit("show ");
    for (size_t i = 0; i < node.expressions.size(); ++i) {
        if (i > 0) emit(", ");
        node.expressions[i]->accept(*this);
    }
    emitLine();
}

void Formatter::visit(IfStmt& node) {
    emit("if ");
    node.condition->accept(*this);
    emit(" then");
    emitLine();
    indent();
    for (auto& s : node.then_branch) s->accept(*this);
    dedent();

    for (auto& elif : node.else_if_branches) {
        emit("else if ");
        elif.condition->accept(*this);
        emit(" then");
        emitLine();
        indent();
        for (auto& s : elif.body) s->accept(*this);
        dedent();
    }

    if (!node.otherwise_branch.empty()) {
        emitLine("otherwise");
        indent();
        for (auto& s : node.otherwise_branch) s->accept(*this);
        dedent();
    }
    emitLine("end");
}

void Formatter::visit(RepeatTimesStmt& node) {
    emit("repeat ");
    node.count_expr->accept(*this);
    emit(" times");
    emitLine();
    indent();
    for (auto& s : node.body) s->accept(*this);
    dedent();
    emitLine("end");
}

void Formatter::visit(WhileStmt& node) {
    emit("repeat while ");
    node.condition->accept(*this);
    emitLine();
    indent();
    for (auto& s : node.body) s->accept(*this);
    dedent();
    emitLine("end");
}

void Formatter::visit(ForEachStmt& node) {
    emit("repeat for each " + node.item_name + " in ");
    node.collection_expr->accept(*this);
    emitLine();
    indent();
    for (auto& s : node.body) s->accept(*this);
    dedent();
    emitLine("end");
}

void Formatter::visit(FuncDeclStmt& node) {
    emit("create function " + node.name);
    if (!node.params.empty()) {
        emit(" using ");
        for (size_t i = 0; i < node.params.size(); ++i) {
            if (i > 0) emit(", ");
            emit(node.params[i]);
        }
    }
    emitLine();
    indent();
    for (auto& s : node.body) s->accept(*this);
    dedent();
    emitLine("end");
}

void Formatter::visit(ReturnStmt& node) {
    emit("give back ");
    if (node.value) node.value->accept(*this);
    emitLine();
}

void Formatter::visit(ClassDeclStmt& node) {
    emitLine("create class " + node.name);
    indent();
    for (const auto& f : node.fields) {
        emitLine("make " + f);
    }
    for (auto& m : node.methods) {
        m->accept(*this);
    }
    dedent();
    emitLine("end");
}

void Formatter::visit(TryCatchStmt& node) {
    emitLine("try");
    indent();
    for (auto& s : node.try_block) s->accept(*this);
    dedent();
    emitLine("catch " + node.error_var);
    indent();
    for (auto& s : node.catch_block) s->accept(*this);
    dedent();
    emitLine("end");
}

void Formatter::visit(ImportStmt& node) {
    emitLine("import " + node.module_name);
}

void Formatter::visit(BreakStmt&) {
    emitLine("break");
}

void Formatter::visit(ContinueStmt&) {
    emitLine("continue");
}

void Formatter::visit(ExprStmt& node) {
    node.expr->accept(*this);
    emitLine();
}

void Formatter::visit(Program& node) {
    for (auto& s : node.statements) {
        s->accept(*this);
    }
}

} // namespace huma
