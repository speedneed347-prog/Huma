#pragma once

#include "huma/ast.hpp"
#include <string>

namespace huma {

class Formatter : public ASTVisitor {
public:
    Formatter() = default;

    std::string format(Program& program);
    std::string formatSource(const std::string& source);

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
    void indent();
    void dedent();
    void emitLine(const std::string& text = "");
    void emit(const std::string& text);

    std::ostringstream out_;
    int indent_level_{0};
    std::string current_line_;
};

} // namespace huma
