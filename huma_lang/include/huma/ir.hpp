#pragma once

#include "huma/common.hpp"
#include "huma/ast.hpp"
#include <string>
#include <vector>
#include <memory>
#include <sstream>

namespace huma {

enum class IROp {
    LOAD_CONST,
    ASSIGN,
    ADD,
    SUB,
    MUL,
    DIV,
    MOD,
    POW,
    EQUAL,
    NOT_EQUAL,
    GREATER,
    LESS,
    GREATER_EQUAL,
    LESS_EQUAL,
    LOGIC_AND,
    LOGIC_OR,
    LOGIC_NOT,
    NEGATE,
    LABEL,
    JUMP,
    JUMP_IF_FALSE,
    CALL,
    RETURN,
    PRINT,
    MAKE_LIST,
    MAKE_DICT,
    GET_INDEX,
    SET_INDEX,
    GET_MEMBER,
    SET_MEMBER,
    NOP
};

struct IRInstruction {
    IROp op;
    std::string result;
    std::string arg1;
    std::string arg2;
    std::vector<std::string> var_args;
    SourceLocation loc;

    std::string toString() const;
};

class IRBasicBlock {
public:
    std::string label;
    std::vector<IRInstruction> instructions;

    explicit IRBasicBlock(std::string lbl) : label(std::move(lbl)) {}
    void addInstruction(IRInstruction instr) { instructions.push_back(std::move(instr)); }
};

class IRFunction {
public:
    std::string name;
    std::vector<std::string> params;
    std::vector<IRBasicBlock> blocks;

    explicit IRFunction(std::string n) : name(std::move(n)) {}
};

class IRProgram {
public:
    std::vector<IRInstruction> instructions;
    std::vector<IRFunction> functions;

    std::string dump() const;
};

class IRGenerator : public ASTVisitor {
public:
    IRGenerator();

    IRProgram generate(Program& program);

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
    std::string newTemp();
    std::string newLabel(const std::string& prefix = "L");
    void emit(IROp op, const std::string& res = "", const std::string& a1 = "", const std::string& a2 = "");

    IRProgram ir_prog_;
    std::string last_temp_;
    int temp_counter_{0};
    int label_counter_{0};
    std::vector<std::string> break_labels_;
    std::vector<std::string> continue_labels_;
};

class IROptimizer {
public:
    static void optimize(IRProgram& program);
private:
    static bool constantFolding(std::vector<IRInstruction>& instrs);
    static bool deadCodeElimination(std::vector<IRInstruction>& instrs);
};

} // namespace huma
