#pragma once

#include "huma/common.hpp"
#include "huma/bytecode.hpp"
#include "huma/ast.hpp"
#include <vector>
#include <map>
#include <memory>
#include <string>
#include <sstream>

namespace huma {

struct CallFrame {
    std::shared_ptr<ObjFunction> function;
    size_t ip{0};
    size_t stack_base{0};
};

enum class InterpretResult {
    OK,
    COMPILE_ERROR,
    RUNTIME_ERROR
};

class BytecodeCompiler : public ASTVisitor {
public:
    BytecodeCompiler(DiagnosticEngine& diag);

    std::shared_ptr<ObjFunction> compile(Program& program);

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
    struct Local {
        std::string name;
        int depth{0};
        bool is_fixed{false};
    };

    struct CompilerScope {
        std::shared_ptr<ObjFunction> function;
        std::vector<Local> locals;
        int scope_depth{0};
        std::vector<std::vector<int>> break_jumps;
        std::vector<int> continue_targets;
    };

    void emitByte(uint8_t byte, int line = 1);
    void emitOp(OpCode op, int line = 1);
    void emitConstant(Value val, int line = 1);
    int emitJump(OpCode instruction, int line = 1);
    void patchJump(int offset);
    void emitLoop(int loop_start, int line = 1);

    int resolveLocal(const std::string& name);
    void addLocal(const std::string& name, bool is_fixed, int line);
    void beginScope();
    void endScope();

    DiagnosticEngine& diag_;
    std::vector<CompilerScope> scopes_;
    CompilerScope* currentScope() { return &scopes_.back(); }
    Chunk* currentChunk() { return &scopes_.back().function->chunk; }
};

class VM {
public:
    VM();
    ~VM() = default;

    InterpretResult interpret(std::shared_ptr<ObjFunction> function);
    InterpretResult interpretSource(const std::string& source, const std::string& filename = "main.huma");

    void defineNative(const std::string& name, NativeFn fn);
    void defineGlobal(const std::string& name, Value value);
    Value* getGlobal(const std::string& name);

    void push(Value val);
    Value pop();
    Value peek(int distance = 0) const;

    void runtimeError(const std::string& message);
    std::string getStdout() const { return stdout_stream_.str(); }
    std::string getStderr() const { return stderr_stream_.str(); }
    void clearOutput() { stdout_stream_.str(""); stderr_stream_.str(""); }

    const std::map<std::string, Value>& getGlobals() const { return globals_; }

private:
    void initStandardLibrary();

    std::vector<Value> stack_;
    std::vector<CallFrame> frames_;
    std::map<std::string, Value> globals_;
    std::ostringstream stdout_stream_;
    std::ostringstream stderr_stream_;
    bool has_runtime_error_{false};
};

} // namespace huma
