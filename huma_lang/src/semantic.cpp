#include "huma/semantic.hpp"
#include <algorithm>

namespace huma {

std::string humaTypeToString(HumaType type) {
    switch (type) {
        case HumaType::NUMBER: return "number";
        case HumaType::INTEGER: return "integer";
        case HumaType::DECIMAL: return "decimal";
        case HumaType::TEXT: return "text";
        case HumaType::BOOLEAN: return "boolean";
        case HumaType::NOTHING: return "nothing";
        case HumaType::LIST: return "list";
        case HumaType::DICTIONARY: return "dictionary";
        case HumaType::FUNCTION: return "function";
        case HumaType::CLASS: return "class";
        case HumaType::VOID_TYPE: return "void";
        case HumaType::ANY: return "any";
        default: return "unknown";
    }
}

HumaType stringToHumaType(const std::string& str) {
    if (str == "number") return HumaType::NUMBER;
    if (str == "integer") return HumaType::INTEGER;
    if (str == "decimal") return HumaType::DECIMAL;
    if (str == "text" || str == "string") return HumaType::TEXT;
    if (str == "boolean" || str == "bool") return HumaType::BOOLEAN;
    if (str == "list") return HumaType::LIST;
    if (str == "dictionary" || str == "dict") return HumaType::DICTIONARY;
    return HumaType::ANY;
}

SemanticAnalyzer::SemanticAnalyzer(DiagnosticEngine& diag)
    : diag_(diag) {
    current_scope_ = std::make_shared<Scope>();
    registerBuiltins();
}

void SemanticAnalyzer::registerBuiltins() {
    static const std::vector<std::string> builtins = {
        "show", "print", "length", "type_of", "to_string", "to_number",
        "range", "assert", "exit", "sqrt", "pow", "abs", "sin", "cos",
        "tan", "floor", "ceil", "round", "min", "max", "random",
        "push", "pop", "insert", "remove", "keys", "values", "contains",
        "sort", "filter", "map", "split", "join", "replace", "trim",
        "to_upper", "to_lower", "now", "sleep", "timestamp", "read_file",
        "write_file", "file_exists", "json_parse", "json_stringify",
        "sha256", "md5", "base64_encode", "base64_decode"
    };

    for (const auto& name : builtins) {
        Symbol sym;
        sym.name = name;
        sym.type = HumaType::FUNCTION;
        sym.is_function = true;
        sym.is_fixed = true;
        current_scope_->define(sym);
    }
}

void SemanticAnalyzer::enterScope() {
    current_scope_ = std::make_shared<Scope>(current_scope_);
}

void SemanticAnalyzer::exitScope() {
    if (current_scope_->getParent()) {
        current_scope_ = current_scope_->getParent();
    }
}

bool SemanticAnalyzer::analyze(Program& program) {
    program.accept(*this);
    return !diag_.hasErrors();
}

void SemanticAnalyzer::visit(LiteralExpr& node) {
    switch (node.lit_type) {
        case LiteralType::NUMBER:
            last_expr_type_ = node.is_integer ? HumaType::INTEGER : HumaType::DECIMAL;
            break;
        case LiteralType::STRING:
            last_expr_type_ = HumaType::TEXT;
            break;
        case LiteralType::BOOLEAN:
            last_expr_type_ = HumaType::BOOLEAN;
            break;
        case LiteralType::NOTHING:
            last_expr_type_ = HumaType::NOTHING;
            break;
    }
}

void SemanticAnalyzer::visit(IdentifierExpr& node) {
    Symbol* sym = current_scope_->lookup(node.name);
    if (!sym) {
        auto candidates = current_scope_->getAllVisibleNames();
        std::string suggestion = findClosestMatch(node.name, candidates, 3);
        std::string sug_msg = "";
        if (!suggestion.empty()) {
            sug_msg = "Did you mean '" + suggestion + "'?";
        }
        diag_.error("Unknown variable '" + node.name + "'.", node.loc, sug_msg);
        last_expr_type_ = HumaType::UNKNOWN;
    } else {
        last_expr_type_ = sym->type;
    }
}

void SemanticAnalyzer::visit(BinaryExpr& node) {
    node.left->accept(*this);
    HumaType left_type = last_expr_type_;
    node.right->accept(*this);
    HumaType right_type = last_expr_type_;

    switch (node.op) {
        case BinaryOp::ADD:
            if (left_type == HumaType::TEXT || right_type == HumaType::TEXT) {
                last_expr_type_ = HumaType::TEXT;
            } else {
                last_expr_type_ = HumaType::NUMBER;
            }
            break;
        case BinaryOp::SUB:
        case BinaryOp::MUL:
        case BinaryOp::DIV:
        case BinaryOp::MOD:
        case BinaryOp::POW:
            last_expr_type_ = HumaType::NUMBER;
            break;
        case BinaryOp::EQUAL:
        case BinaryOp::NOT_EQUAL:
        case BinaryOp::GREATER:
        case BinaryOp::LESS:
        case BinaryOp::GREATER_EQUAL:
        case BinaryOp::LESS_EQUAL:
        case BinaryOp::AND:
        case BinaryOp::OR:
            last_expr_type_ = HumaType::BOOLEAN;
            break;
    }
}

void SemanticAnalyzer::visit(UnaryExpr& node) {
    node.operand->accept(*this);
    if (node.op == UnaryOp::NOT) {
        last_expr_type_ = HumaType::BOOLEAN;
    } else {
        last_expr_type_ = HumaType::NUMBER;
    }
}

void SemanticAnalyzer::visit(CallExpr& node) {
    Symbol* sym = current_scope_->lookup(node.callee);
    if (!sym) {
        auto candidates = current_scope_->getAllVisibleNames();
        std::string suggestion = findClosestMatch(node.callee, candidates, 3);
        std::string sug_msg = "";
        if (!suggestion.empty()) {
            sug_msg = "Did you mean '" + suggestion + "'?";
        }
        diag_.error("Undefined function '" + node.callee + "'.", node.loc, sug_msg);
    }

    for (auto& arg : node.args) {
        arg->accept(*this);
    }
    last_expr_type_ = HumaType::ANY;
}

void SemanticAnalyzer::visit(ListExpr& node) {
    for (auto& el : node.elements) {
        el->accept(*this);
    }
    last_expr_type_ = HumaType::LIST;
}

void SemanticAnalyzer::visit(DictExpr& node) {
    for (auto& entry : node.entries) {
        entry.second->accept(*this);
    }
    last_expr_type_ = HumaType::DICTIONARY;
}

void SemanticAnalyzer::visit(IndexExpr& node) {
    node.collection->accept(*this);
    node.index->accept(*this);
    last_expr_type_ = HumaType::ANY;
}

void SemanticAnalyzer::visit(MemberExpr& node) {
    node.object->accept(*this);
    last_expr_type_ = HumaType::ANY;
}

void SemanticAnalyzer::visit(VarDeclStmt& node) {
    node.init_expr->accept(*this);
    HumaType inferred_type = last_expr_type_;

    if (!node.type_annotation.empty()) {
        HumaType explicit_type = stringToHumaType(node.type_annotation);
        if (explicit_type != HumaType::ANY && inferred_type != HumaType::ANY &&
            explicit_type != inferred_type && inferred_type != HumaType::UNKNOWN) {
            diag_.warning("Type annotation '" + node.type_annotation + "' does not match inferred type '" +
                          humaTypeToString(inferred_type) + "'.", node.loc);
        }
        inferred_type = explicit_type;
    }

    Symbol sym;
    sym.name = node.name;
    sym.type = inferred_type;
    sym.is_fixed = node.is_fixed;
    sym.loc = node.loc;

    if (!current_scope_->define(sym)) {
        diag_.warning("Variable '" + node.name + "' shadows an existing declaration in the same scope.", node.loc);
    }
}

void SemanticAnalyzer::visit(ChangeStmt& node) {
    Symbol* sym = current_scope_->lookup(node.target_name);
    if (!sym) {
        auto candidates = current_scope_->getAllVisibleNames();
        std::string suggestion = findClosestMatch(node.target_name, candidates, 3);
        std::string sug_msg = "";
        if (!suggestion.empty()) {
            sug_msg = "Did you mean '" + suggestion + "'?";
        }
        diag_.error("Cannot change unknown variable '" + node.target_name + "'.", node.loc, sug_msg);
    } else if (sym->is_fixed) {
        diag_.error("Cannot reassign fixed variable '" + node.target_name + "'.", node.loc,
                    "Variable was declared with 'fixed'. Remove 'fixed' if it should be mutable.");
    }

    node.value_expr->accept(*this);
}

void SemanticAnalyzer::visit(ShowStmt& node) {
    for (auto& expr : node.expressions) {
        expr->accept(*this);
    }
}

void SemanticAnalyzer::visit(IfStmt& node) {
    node.condition->accept(*this);
    enterScope();
    for (auto& stmt : node.then_branch) {
        stmt->accept(*this);
    }
    exitScope();

    for (auto& elif : node.else_if_branches) {
        elif.condition->accept(*this);
        enterScope();
        for (auto& stmt : elif.body) {
            stmt->accept(*this);
        }
        exitScope();
    }

    if (!node.otherwise_branch.empty()) {
        enterScope();
        for (auto& stmt : node.otherwise_branch) {
            stmt->accept(*this);
        }
        exitScope();
    }
}

void SemanticAnalyzer::visit(RepeatTimesStmt& node) {
    node.count_expr->accept(*this);
    loop_depth_++;
    enterScope();
    for (auto& stmt : node.body) {
        stmt->accept(*this);
    }
    exitScope();
    loop_depth_--;
}

void SemanticAnalyzer::visit(WhileStmt& node) {
    node.condition->accept(*this);
    loop_depth_++;
    enterScope();
    for (auto& stmt : node.body) {
        stmt->accept(*this);
    }
    exitScope();
    loop_depth_--;
}

void SemanticAnalyzer::visit(ForEachStmt& node) {
    node.collection_expr->accept(*this);
    loop_depth_++;
    enterScope();
    Symbol item_sym;
    item_sym.name = node.item_name;
    item_sym.type = HumaType::ANY;
    item_sym.loc = node.loc;
    current_scope_->define(item_sym);

    for (auto& stmt : node.body) {
        stmt->accept(*this);
    }
    exitScope();
    loop_depth_--;
}

void SemanticAnalyzer::visit(FuncDeclStmt& node) {
    Symbol fn_sym;
    fn_sym.name = node.name;
    fn_sym.type = HumaType::FUNCTION;
    fn_sym.is_function = true;
    fn_sym.is_fixed = true;
    fn_sym.param_count = static_cast<int>(node.params.size());
    fn_sym.loc = node.loc;
    current_scope_->define(fn_sym);

    function_depth_++;
    enterScope();
    for (const auto& param : node.params) {
        Symbol param_sym;
        param_sym.name = param;
        param_sym.type = HumaType::ANY;
        param_sym.loc = node.loc;
        current_scope_->define(param_sym);
    }

    for (auto& stmt : node.body) {
        stmt->accept(*this);
    }
    exitScope();
    function_depth_--;
}

void SemanticAnalyzer::visit(ReturnStmt& node) {
    if (function_depth_ == 0) {
        diag_.error("'give back' or 'return' used outside of a function body.", node.loc);
    }
    if (node.value) {
        node.value->accept(*this);
    }
}

void SemanticAnalyzer::visit(ClassDeclStmt& node) {
    Symbol cls_sym;
    cls_sym.name = node.name;
    cls_sym.type = HumaType::CLASS;
    cls_sym.is_class = true;
    cls_sym.is_fixed = true;
    cls_sym.loc = node.loc;
    current_scope_->define(cls_sym);

    enterScope();
    for (const auto& field : node.fields) {
        Symbol field_sym;
        field_sym.name = field;
        field_sym.type = HumaType::ANY;
        field_sym.loc = node.loc;
        current_scope_->define(field_sym);
    }

    for (auto& method : node.methods) {
        method->accept(*this);
    }
    exitScope();
}

void SemanticAnalyzer::visit(TryCatchStmt& node) {
    enterScope();
    for (auto& stmt : node.try_block) {
        stmt->accept(*this);
    }
    exitScope();

    enterScope();
    Symbol err_sym;
    err_sym.name = node.error_var;
    err_sym.type = HumaType::TEXT;
    err_sym.loc = node.loc;
    current_scope_->define(err_sym);

    for (auto& stmt : node.catch_block) {
        stmt->accept(*this);
    }
    exitScope();
}

void SemanticAnalyzer::visit(ImportStmt& node) {
    // Module import semantic hook
}

void SemanticAnalyzer::visit(BreakStmt& node) {
    if (loop_depth_ == 0) {
        diag_.error("'break' statement cannot be used outside of a loop.", node.loc);
    }
}

void SemanticAnalyzer::visit(ContinueStmt& node) {
    if (loop_depth_ == 0) {
        diag_.error("'continue' statement cannot be used outside of a loop.", node.loc);
    }
}

void SemanticAnalyzer::visit(ExprStmt& node) {
    node.expr->accept(*this);
}

void SemanticAnalyzer::visit(Program& node) {
    for (auto& stmt : node.statements) {
        stmt->accept(*this);
    }
}

} // namespace huma
