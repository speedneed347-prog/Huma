#pragma once

#include "huma/common.hpp"
#include <string>
#include <vector>
#include <memory>

namespace huma {

// Forward declarations
class ASTVisitor;

class ASTNode {
public:
    SourceLocation loc;
    virtual ~ASTNode() = default;
    virtual void accept(ASTVisitor& visitor) = 0;
};

// Expressions
class Expr : public ASTNode {};

enum class LiteralType {
    NUMBER,
    STRING,
    BOOLEAN,
    NOTHING
};

class LiteralExpr : public Expr {
public:
    LiteralType lit_type;
    double number_val{0.0};
    bool is_integer{false};
    std::string string_val;
    bool bool_val{false};

    LiteralExpr(double val, bool is_int, const SourceLocation& l)
        : lit_type(LiteralType::NUMBER), number_val(val), is_integer(is_int) { loc = l; }

    LiteralExpr(const std::string& val, const SourceLocation& l)
        : lit_type(LiteralType::STRING), string_val(val) { loc = l; }

    LiteralExpr(bool val, const SourceLocation& l)
        : lit_type(LiteralType::BOOLEAN), bool_val(val) { loc = l; }

    explicit LiteralExpr(const SourceLocation& l)
        : lit_type(LiteralType::NOTHING) { loc = l; }

    void accept(ASTVisitor& visitor) override;
};

class IdentifierExpr : public Expr {
public:
    std::string name;
    IdentifierExpr(std::string n, const SourceLocation& l) : name(std::move(n)) { loc = l; }
    void accept(ASTVisitor& visitor) override;
};

enum class BinaryOp {
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
    AND,
    OR
};

class BinaryExpr : public Expr {
public:
    BinaryOp op;
    std::unique_ptr<Expr> left;
    std::unique_ptr<Expr> right;

    BinaryExpr(BinaryOp o, std::unique_ptr<Expr> l, std::unique_ptr<Expr> r, const SourceLocation& loc_)
        : op(o), left(std::move(l)), right(std::move(r)) { loc = loc_; }
    void accept(ASTVisitor& visitor) override;
};

enum class UnaryOp {
    NOT,
    NEGATE
};

class UnaryExpr : public Expr {
public:
    UnaryOp op;
    std::unique_ptr<Expr> operand;

    UnaryExpr(UnaryOp o, std::unique_ptr<Expr> opnd, const SourceLocation& loc_)
        : op(o), operand(std::move(opnd)) { loc = loc_; }
    void accept(ASTVisitor& visitor) override;
};

class CallExpr : public Expr {
public:
    std::string callee;
    std::unique_ptr<Expr> callee_expr;
    std::vector<std::unique_ptr<Expr>> args;

    CallExpr(std::string name, std::vector<std::unique_ptr<Expr>> arguments, const SourceLocation& loc_)
        : callee(std::move(name)), args(std::move(arguments)) { loc = loc_; }
    void accept(ASTVisitor& visitor) override;
};

class ListExpr : public Expr {
public:
    std::vector<std::unique_ptr<Expr>> elements;
    ListExpr(std::vector<std::unique_ptr<Expr>> elems, const SourceLocation& loc_)
        : elements(std::move(elems)) { loc = loc_; }
    void accept(ASTVisitor& visitor) override;
};

class DictExpr : public Expr {
public:
    std::vector<std::pair<std::string, std::unique_ptr<Expr>>> entries;
    DictExpr(std::vector<std::pair<std::string, std::unique_ptr<Expr>>> ents, const SourceLocation& loc_)
        : entries(std::move(ents)) { loc = loc_; }
    void accept(ASTVisitor& visitor) override;
};

class IndexExpr : public Expr {
public:
    std::unique_ptr<Expr> collection;
    std::unique_ptr<Expr> index;
    IndexExpr(std::unique_ptr<Expr> col, std::unique_ptr<Expr> idx, const SourceLocation& loc_)
        : collection(std::move(col)), index(std::move(idx)) { loc = loc_; }
    void accept(ASTVisitor& visitor) override;
};

class MemberExpr : public Expr {
public:
    std::unique_ptr<Expr> object;
    std::string member;
    MemberExpr(std::unique_ptr<Expr> obj, std::string mem, const SourceLocation& loc_)
        : object(std::move(obj)), member(std::move(mem)) { loc = loc_; }
    void accept(ASTVisitor& visitor) override;
};

// Statements
class Stmt : public ASTNode {};

class VarDeclStmt : public Stmt {
public:
    std::string name;
    bool is_fixed{false}; // fixed vs make
    std::string type_annotation;
    std::unique_ptr<Expr> init_expr;

    VarDeclStmt(std::string n, bool fixed, std::unique_ptr<Expr> init, const SourceLocation& loc_, std::string type_ann = "")
        : name(std::move(n)), is_fixed(fixed), type_annotation(std::move(type_ann)), init_expr(std::move(init)) { loc = loc_; }
    void accept(ASTVisitor& visitor) override;
};

class ChangeStmt : public Stmt {
public:
    std::string target_name;
    std::unique_ptr<Expr> target_expr; // For member / index assignment
    std::unique_ptr<Expr> value_expr;

    ChangeStmt(std::string name, std::unique_ptr<Expr> val, const SourceLocation& loc_)
        : target_name(std::move(name)), value_expr(std::move(val)) { loc = loc_; }
    void accept(ASTVisitor& visitor) override;
};

class ShowStmt : public Stmt {
public:
    std::vector<std::unique_ptr<Expr>> expressions;
    ShowStmt(std::vector<std::unique_ptr<Expr>> exprs, const SourceLocation& loc_)
        : expressions(std::move(exprs)) { loc = loc_; }
    void accept(ASTVisitor& visitor) override;
};

struct ElseIfBranch {
    std::unique_ptr<Expr> condition;
    std::vector<std::unique_ptr<Stmt>> body;
    SourceLocation loc;
};

class IfStmt : public Stmt {
public:
    std::unique_ptr<Expr> condition;
    std::vector<std::unique_ptr<Stmt>> then_branch;
    std::vector<ElseIfBranch> else_if_branches;
    std::vector<std::unique_ptr<Stmt>> otherwise_branch;

    IfStmt(std::unique_ptr<Expr> cond, std::vector<std::unique_ptr<Stmt>> then_b, const SourceLocation& loc_)
        : condition(std::move(cond)), then_branch(std::move(then_b)) { loc = loc_; }
    void accept(ASTVisitor& visitor) override;
};

class RepeatTimesStmt : public Stmt {
public:
    std::unique_ptr<Expr> count_expr;
    std::vector<std::unique_ptr<Stmt>> body;

    RepeatTimesStmt(std::unique_ptr<Expr> count, std::vector<std::unique_ptr<Stmt>> b, const SourceLocation& loc_)
        : count_expr(std::move(count)), body(std::move(b)) { loc = loc_; }
    void accept(ASTVisitor& visitor) override;
};

class WhileStmt : public Stmt {
public:
    std::unique_ptr<Expr> condition;
    std::vector<std::unique_ptr<Stmt>> body;

    WhileStmt(std::unique_ptr<Expr> cond, std::vector<std::unique_ptr<Stmt>> b, const SourceLocation& loc_)
        : condition(std::move(cond)), body(std::move(b)) { loc = loc_; }
    void accept(ASTVisitor& visitor) override;
};

class ForEachStmt : public Stmt {
public:
    std::string item_name;
    std::unique_ptr<Expr> collection_expr;
    std::vector<std::unique_ptr<Stmt>> body;

    ForEachStmt(std::string item, std::unique_ptr<Expr> col, std::vector<std::unique_ptr<Stmt>> b, const SourceLocation& loc_)
        : item_name(std::move(item)), collection_expr(std::move(col)), body(std::move(b)) { loc = loc_; }
    void accept(ASTVisitor& visitor) override;
};

class FuncDeclStmt : public Stmt {
public:
    std::string name;
    std::vector<std::string> params;
    std::vector<std::unique_ptr<Stmt>> body;
    std::string return_type;

    FuncDeclStmt(std::string n, std::vector<std::string> p, std::vector<std::unique_ptr<Stmt>> b, const SourceLocation& loc_, std::string ret = "")
        : name(std::move(n)), params(std::move(p)), body(std::move(b)), return_type(std::move(ret)) { loc = loc_; }
    void accept(ASTVisitor& visitor) override;
};

class ReturnStmt : public Stmt {
public:
    std::unique_ptr<Expr> value;
    ReturnStmt(std::unique_ptr<Expr> val, const SourceLocation& loc_)
        : value(std::move(val)) { loc = loc_; }
    void accept(ASTVisitor& visitor) override;
};

class ClassDeclStmt : public Stmt {
public:
    std::string name;
    std::string parent_class;
    std::vector<std::string> fields;
    std::vector<std::unique_ptr<FuncDeclStmt>> methods;

    ClassDeclStmt(std::string n, std::vector<std::string> f, std::vector<std::unique_ptr<FuncDeclStmt>> m, const SourceLocation& loc_)
        : name(std::move(n)), fields(std::move(f)), methods(std::move(m)) { loc = loc_; }
    void accept(ASTVisitor& visitor) override;
};

class TryCatchStmt : public Stmt {
public:
    std::vector<std::unique_ptr<Stmt>> try_block;
    std::string error_var;
    std::vector<std::unique_ptr<Stmt>> catch_block;

    TryCatchStmt(std::vector<std::unique_ptr<Stmt>> tb, std::string err_v, std::vector<std::unique_ptr<Stmt>> cb, const SourceLocation& loc_)
        : try_block(std::move(tb)), error_var(std::move(err_v)), catch_block(std::move(cb)) { loc = loc_; }
    void accept(ASTVisitor& visitor) override;
};

class ImportStmt : public Stmt {
public:
    std::string module_name;
    ImportStmt(std::string mod, const SourceLocation& loc_)
        : module_name(std::move(mod)) { loc = loc_; }
    void accept(ASTVisitor& visitor) override;
};

class BreakStmt : public Stmt {
public:
    explicit BreakStmt(const SourceLocation& loc_) { loc = loc_; }
    void accept(ASTVisitor& visitor) override;
};

class ContinueStmt : public Stmt {
public:
    explicit ContinueStmt(const SourceLocation& loc_) { loc = loc_; }
    void accept(ASTVisitor& visitor) override;
};

class ExprStmt : public Stmt {
public:
    std::unique_ptr<Expr> expr;
    ExprStmt(std::unique_ptr<Expr> e, const SourceLocation& loc_) : expr(std::move(e)) { loc = loc_; }
    void accept(ASTVisitor& visitor) override;
};

class Program : public ASTNode {
public:
    std::vector<std::unique_ptr<Stmt>> statements;
    void accept(ASTVisitor& visitor) override;
};

class ASTVisitor {
public:
    virtual ~ASTVisitor() = default;
    virtual void visit(LiteralExpr& node) = 0;
    virtual void visit(IdentifierExpr& node) = 0;
    virtual void visit(BinaryExpr& node) = 0;
    virtual void visit(UnaryExpr& node) = 0;
    virtual void visit(CallExpr& node) = 0;
    virtual void visit(ListExpr& node) = 0;
    virtual void visit(DictExpr& node) = 0;
    virtual void visit(IndexExpr& node) = 0;
    virtual void visit(MemberExpr& node) = 0;

    virtual void visit(VarDeclStmt& node) = 0;
    virtual void visit(ChangeStmt& node) = 0;
    virtual void visit(ShowStmt& node) = 0;
    virtual void visit(IfStmt& node) = 0;
    virtual void visit(RepeatTimesStmt& node) = 0;
    virtual void visit(WhileStmt& node) = 0;
    virtual void visit(ForEachStmt& node) = 0;
    virtual void visit(FuncDeclStmt& node) = 0;
    virtual void visit(ReturnStmt& node) = 0;
    virtual void visit(ClassDeclStmt& node) = 0;
    virtual void visit(TryCatchStmt& node) = 0;
    virtual void visit(ImportStmt& node) = 0;
    virtual void visit(BreakStmt& node) = 0;
    virtual void visit(ContinueStmt& node) = 0;
    virtual void visit(ExprStmt& node) = 0;
    virtual void visit(Program& node) = 0;
};

} // namespace huma
