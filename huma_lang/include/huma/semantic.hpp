#pragma once

#include "huma/common.hpp"
#include "huma/ast.hpp"
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <set>

namespace huma {

enum class HumaType {
    UNKNOWN,
    ANY,
    NUMBER,
    INTEGER,
    DECIMAL,
    TEXT,
    BOOLEAN,
    NOTHING,
    LIST,
    DICTIONARY,
    FUNCTION,
    CLASS,
    VOID_TYPE
};

std::string humaTypeToString(HumaType type);
HumaType stringToHumaType(const std::string& str);

struct Symbol {
    std::string name;
    HumaType type{HumaType::ANY};
    bool is_fixed{false};
    bool is_function{false};
    bool is_class{false};
    int param_count{0};
    SourceLocation loc;
};

class Scope {
public:
    explicit Scope(std::shared_ptr<Scope> parent = nullptr) : parent_(parent) {}

    bool define(const Symbol& sym) {
        if (symbols_.find(sym.name) != symbols_.end()) {
            return false;
        }
        symbols_[sym.name] = sym;
        return true;
    }

    Symbol* lookup(const std::string& name) {
        auto it = symbols_.find(name);
        if (it != symbols_.end()) return &it->second;
        if (parent_) return parent_->lookup(name);
        return nullptr;
    }

    Symbol* lookupLocal(const std::string& name) {
        auto it = symbols_.find(name);
        if (it != symbols_.end()) return &it->second;
        return nullptr;
    }

    std::vector<std::string> getAllVisibleNames() const {
        std::set<std::string> names;
        for (const auto& pair : symbols_) names.insert(pair.first);
        if (parent_) {
            auto parent_names = parent_->getAllVisibleNames();
            names.insert(parent_names.begin(), parent_names.end());
        }
        return std::vector<std::string>(names.begin(), names.end());
    }

    std::shared_ptr<Scope> getParent() const { return parent_; }

private:
    std::map<std::string, Symbol> symbols_;
    std::shared_ptr<Scope> parent_;
};

class SemanticAnalyzer : public ASTVisitor {
public:
    explicit SemanticAnalyzer(DiagnosticEngine& diag);

    bool analyze(Program& program);

    void visit(LiteralExpr& node) override;
    void visit(IdentifierExpr& node) override;
    void visit(BinaryExpr& node) override;
    void visit(UnaryExpr& node) override;
    void visit(CallExpr& node) override;
    void visit(ListExpr& node) override;
    void visit(DictExpr& node) override;
    void visit(IndexExpr& node) override;
    void visit(MemberExpr& node) override;

    void visit(VarDeclStmt& node) override;
    void visit(ChangeStmt& node) override;
    void visit(ShowStmt& node) override;
    void visit(IfStmt& node) override;
    void visit(RepeatTimesStmt& node) override;
    void visit(WhileStmt& node) override;
    void visit(ForEachStmt& node) override;
    void visit(FuncDeclStmt& node) override;
    void visit(ReturnStmt& node) override;
    void visit(ClassDeclStmt& node) override;
    void visit(TryCatchStmt& node) override;
    void visit(ImportStmt& node) override;
    void visit(BreakStmt& node) override;
    void visit(ContinueStmt& node) override;
    void visit(ExprStmt& node) override;
    void visit(Program& node) override;

private:
    void enterScope();
    void exitScope();
    void registerBuiltins();

    DiagnosticEngine& diag_;
    std::shared_ptr<Scope> current_scope_;
    int loop_depth_{0};
    int function_depth_{0};
    HumaType last_expr_type_{HumaType::ANY};
};

} // namespace huma
