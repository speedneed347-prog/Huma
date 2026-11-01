#include "huma/vm.hpp"
#include "huma/lexer.hpp"
#include "huma/parser.hpp"
#include "huma/semantic.hpp"
#include "huma/stdlib.hpp"
#include <cmath>
#include <iostream>

namespace huma {

// ==================== Bytecode Compiler ====================

BytecodeCompiler::BytecodeCompiler(DiagnosticEngine& diag)
    : diag_(diag) {
    CompilerScope root_scope;
    root_scope.function = std::make_shared<ObjFunction>("main");
    scopes_.push_back(root_scope);
}

void BytecodeCompiler::emitByte(uint8_t byte, int line) {
    currentChunk()->write(byte, line);
}

void BytecodeCompiler::emitOp(OpCode op, int line) {
    currentChunk()->writeOp(op, line);
}

void BytecodeCompiler::emitConstant(Value val, int line) {
    size_t idx = currentChunk()->addConstant(val);
    emitOp(OpCode::OP_CONSTANT, line);
    emitByte(static_cast<uint8_t>(idx), line);
}

int BytecodeCompiler::emitJump(OpCode instruction, int line) {
    emitOp(instruction, line);
    emitByte(0xff, line);
    emitByte(0xff, line);
    return static_cast<int>(currentChunk()->code.size() - 2);
}

void BytecodeCompiler::patchJump(int offset) {
    int jump = static_cast<int>(currentChunk()->code.size()) - offset - 2;
    currentChunk()->code[offset] = static_cast<uint8_t>((jump >> 8) & 0xff);
    currentChunk()->code[offset + 1] = static_cast<uint8_t>(jump & 0xff);
}

void BytecodeCompiler::emitLoop(int loop_start, int line) {
    emitOp(OpCode::OP_LOOP, line);
    int jump = static_cast<int>(currentChunk()->code.size()) - loop_start + 2;
    emitByte(static_cast<uint8_t>((jump >> 8) & 0xff), line);
    emitByte(static_cast<uint8_t>(jump & 0xff), line);
}

void BytecodeCompiler::beginScope() {
    currentScope()->scope_depth++;
}

void BytecodeCompiler::endScope() {
    currentScope()->scope_depth--;
    while (!currentScope()->locals.empty() &&
           currentScope()->locals.back().depth > currentScope()->scope_depth) {
        emitOp(OpCode::OP_POP);
        currentScope()->locals.pop_back();
    }
}

int BytecodeCompiler::resolveLocal(const std::string& name) {
    auto& locals = currentScope()->locals;
    for (int i = static_cast<int>(locals.size()) - 1; i >= 0; --i) {
        if (locals[i].name == name) {
            return i;
        }
    }
    return -1;
}

void BytecodeCompiler::addLocal(const std::string& name, bool is_fixed, int line) {
    currentScope()->locals.push_back({name, currentScope()->scope_depth, is_fixed});
}

std::shared_ptr<ObjFunction> BytecodeCompiler::compile(Program& program) {
    program.accept(*this);
    emitOp(OpCode::OP_NULL);
    emitOp(OpCode::OP_RETURN);
    return scopes_.front().function;
}

void BytecodeCompiler::visit(LiteralExpr& node) {
    switch (node.lit_type) {
        case LiteralType::NUMBER:
            emitConstant(Value::makeNumber(node.number_val, node.is_integer), node.loc.line);
            break;
        case LiteralType::STRING:
            emitConstant(Value::makeString(node.string_val), node.loc.line);
            break;
        case LiteralType::BOOLEAN:
            emitOp(node.bool_val ? OpCode::OP_TRUE : OpCode::OP_FALSE, node.loc.line);
            break;
        case LiteralType::NOTHING:
            emitOp(OpCode::OP_NULL, node.loc.line);
            break;
    }
}

void BytecodeCompiler::visit(IdentifierExpr& node) {
    int local = resolveLocal(node.name);
    if (local != -1) {
        emitOp(OpCode::OP_GET_LOCAL, node.loc.line);
        emitByte(static_cast<uint8_t>(local), node.loc.line);
    } else {
        size_t name_idx = currentChunk()->addConstant(Value::makeString(node.name));
        emitOp(OpCode::OP_GET_GLOBAL, node.loc.line);
        emitByte(static_cast<uint8_t>(name_idx), node.loc.line);
    }
}

void BytecodeCompiler::visit(BinaryExpr& node) {
    node.left->accept(*this);
    node.right->accept(*this);

    int line = node.loc.line;
    switch (node.op) {
        case BinaryOp::ADD: emitOp(OpCode::OP_ADD, line); break;
        case BinaryOp::SUB: emitOp(OpCode::OP_SUBTRACT, line); break;
        case BinaryOp::MUL: emitOp(OpCode::OP_MULTIPLY, line); break;
        case BinaryOp::DIV: emitOp(OpCode::OP_DIVIDE, line); break;
        case BinaryOp::MOD: emitOp(OpCode::OP_MODULO, line); break;
        case BinaryOp::POW: emitOp(OpCode::OP_POWER, line); break;
        case BinaryOp::EQUAL: emitOp(OpCode::OP_EQUAL, line); break;
        case BinaryOp::NOT_EQUAL:
            emitOp(OpCode::OP_EQUAL, line);
            emitOp(OpCode::OP_NOT, line);
            break;
        case BinaryOp::GREATER: emitOp(OpCode::OP_GREATER, line); break;
        case BinaryOp::LESS: emitOp(OpCode::OP_LESS, line); break;
        case BinaryOp::GREATER_EQUAL:
            emitOp(OpCode::OP_LESS, line);
            emitOp(OpCode::OP_NOT, line);
            break;
        case BinaryOp::LESS_EQUAL:
            emitOp(OpCode::OP_GREATER, line);
            emitOp(OpCode::OP_NOT, line);
            break;
        case BinaryOp::AND:
        case BinaryOp::OR:
            // Handled as boolean logic
            break;
    }
}

void BytecodeCompiler::visit(UnaryExpr& node) {
    node.operand->accept(*this);
    if (node.op == UnaryOp::NOT) {
        emitOp(OpCode::OP_NOT, node.loc.line);
    } else {
        emitOp(OpCode::OP_NEGATE, node.loc.line);
    }
}

void BytecodeCompiler::visit(CallExpr& node) {
    // Look up function
    int local = resolveLocal(node.callee);
    if (local != -1) {
        emitOp(OpCode::OP_GET_LOCAL, node.loc.line);
        emitByte(static_cast<uint8_t>(local), node.loc.line);
    } else {
        size_t name_idx = currentChunk()->addConstant(Value::makeString(node.callee));
        emitOp(OpCode::OP_GET_GLOBAL, node.loc.line);
        emitByte(static_cast<uint8_t>(name_idx), node.loc.line);
    }

    for (auto& arg : node.args) {
        arg->accept(*this);
    }
    emitOp(OpCode::OP_CALL, node.loc.line);
    emitByte(static_cast<uint8_t>(node.args.size()), node.loc.line);
}

void BytecodeCompiler::visit(ListExpr& node) {
    for (auto& elem : node.elements) {
        elem->accept(*this);
    }
    emitOp(OpCode::OP_BUILD_LIST, node.loc.line);
    emitByte(static_cast<uint8_t>(node.elements.size()), node.loc.line);
}

void BytecodeCompiler::visit(DictExpr& node) {
    for (auto& entry : node.entries) {
        emitConstant(Value::makeString(entry.first), node.loc.line);
        entry.second->accept(*this);
    }
    emitOp(OpCode::OP_BUILD_DICT, node.loc.line);
    emitByte(static_cast<uint8_t>(node.entries.size()), node.loc.line);
}

void BytecodeCompiler::visit(IndexExpr& node) {
    node.collection->accept(*this);
    node.index->accept(*this);
    emitOp(OpCode::OP_GET_INDEX, node.loc.line);
}

void BytecodeCompiler::visit(MemberExpr& node) {
    node.object->accept(*this);
    size_t name_idx = currentChunk()->addConstant(Value::makeString(node.member));
    emitOp(OpCode::OP_GET_PROPERTY, node.loc.line);
    emitByte(static_cast<uint8_t>(name_idx), node.loc.line);
}

void BytecodeCompiler::visit(VarDeclStmt& node) {
    node.init_expr->accept(*this);

    if (currentScope()->scope_depth > 0) {
        addLocal(node.name, node.is_fixed, node.loc.line);
    } else {
        size_t name_idx = currentChunk()->addConstant(Value::makeString(node.name));
        emitOp(OpCode::OP_DEFINE_GLOBAL, node.loc.line);
        emitByte(static_cast<uint8_t>(name_idx), node.loc.line);
    }
}

void BytecodeCompiler::visit(ChangeStmt& node) {
    node.value_expr->accept(*this);

    int local = resolveLocal(node.target_name);
    if (local != -1) {
        emitOp(OpCode::OP_SET_LOCAL, node.loc.line);
        emitByte(static_cast<uint8_t>(local), node.loc.line);
    } else {
        size_t name_idx = currentChunk()->addConstant(Value::makeString(node.target_name));
        emitOp(OpCode::OP_SET_GLOBAL, node.loc.line);
        emitByte(static_cast<uint8_t>(name_idx), node.loc.line);
    }
    emitOp(OpCode::OP_POP, node.loc.line);
}

void BytecodeCompiler::visit(ShowStmt& node) {
    for (auto& expr : node.expressions) {
        expr->accept(*this);
    }
    emitOp(OpCode::OP_PRINT, node.loc.line);
    emitByte(static_cast<uint8_t>(node.expressions.size()), node.loc.line);
}

void BytecodeCompiler::visit(IfStmt& node) {
    node.condition->accept(*this);
    int then_jump = emitJump(OpCode::OP_JUMP_IF_FALSE, node.loc.line);
    emitOp(OpCode::OP_POP, node.loc.line); // Pop condition

    beginScope();
    for (auto& stmt : node.then_branch) {
        stmt->accept(*this);
    }
    endScope();

    int else_jump = emitJump(OpCode::OP_JUMP, node.loc.line);
    patchJump(then_jump);
    emitOp(OpCode::OP_POP, node.loc.line); // Pop condition on false path

    // Else if branches
    std::vector<int> end_jumps;
    end_jumps.push_back(else_jump);

    for (auto& elif : node.else_if_branches) {
        elif.condition->accept(*this);
        int elif_jump = emitJump(OpCode::OP_JUMP_IF_FALSE, elif.loc.line);
        emitOp(OpCode::OP_POP, elif.loc.line);

        beginScope();
        for (auto& stmt : elif.body) {
            stmt->accept(*this);
        }
        endScope();

        end_jumps.push_back(emitJump(OpCode::OP_JUMP, elif.loc.line));
        patchJump(elif_jump);
        emitOp(OpCode::OP_POP, elif.loc.line);
    }

    beginScope();
    for (auto& stmt : node.otherwise_branch) {
        stmt->accept(*this);
    }
    endScope();

    for (int j : end_jumps) {
        patchJump(j);
    }
}

void BytecodeCompiler::visit(RepeatTimesStmt& node) {
    // Loop counter: slot 0 = count, slot 1 = current
    beginScope();
    node.count_expr->accept(*this);
    addLocal("__count_" + std::to_string(node.loc.line), true, node.loc.line);

    emitConstant(Value::makeNumber(0, true), node.loc.line);
    addLocal("__idx_" + std::to_string(node.loc.line), false, node.loc.line);

    int count_slot = resolveLocal("__count_" + std::to_string(node.loc.line));
    int idx_slot = resolveLocal("__idx_" + std::to_string(node.loc.line));

    int loop_start = static_cast<int>(currentChunk()->code.size());
    currentScope()->continue_targets.push_back(loop_start);
    currentScope()->break_jumps.emplace_back();

    // Check idx < count
    emitOp(OpCode::OP_GET_LOCAL, node.loc.line);
    emitByte(static_cast<uint8_t>(idx_slot), node.loc.line);
    emitOp(OpCode::OP_GET_LOCAL, node.loc.line);
    emitByte(static_cast<uint8_t>(count_slot), node.loc.line);
    emitOp(OpCode::OP_LESS, node.loc.line);

    int exit_jump = emitJump(OpCode::OP_JUMP_IF_FALSE, node.loc.line);
    emitOp(OpCode::OP_POP, node.loc.line);

    beginScope();
    for (auto& stmt : node.body) {
        stmt->accept(*this);
    }
    endScope();

    // Increment idx
    emitOp(OpCode::OP_GET_LOCAL, node.loc.line);
    emitByte(static_cast<uint8_t>(idx_slot), node.loc.line);
    emitConstant(Value::makeNumber(1, true), node.loc.line);
    emitOp(OpCode::OP_ADD, node.loc.line);
    emitOp(OpCode::OP_SET_LOCAL, node.loc.line);
    emitByte(static_cast<uint8_t>(idx_slot), node.loc.line);
    emitOp(OpCode::OP_POP, node.loc.line);

    emitLoop(loop_start, node.loc.line);
    patchJump(exit_jump);
    emitOp(OpCode::OP_POP, node.loc.line);

    for (int bj : currentScope()->break_jumps.back()) {
        patchJump(bj);
    }
    currentScope()->break_jumps.pop_back();
    currentScope()->continue_targets.pop_back();

    endScope();
}

void BytecodeCompiler::visit(WhileStmt& node) {
    int loop_start = static_cast<int>(currentChunk()->code.size());
    currentScope()->continue_targets.push_back(loop_start);
    currentScope()->break_jumps.emplace_back();

    node.condition->accept(*this);
    int exit_jump = emitJump(OpCode::OP_JUMP_IF_FALSE, node.loc.line);
    emitOp(OpCode::OP_POP, node.loc.line);

    beginScope();
    for (auto& stmt : node.body) {
        stmt->accept(*this);
    }
    endScope();

    emitLoop(loop_start, node.loc.line);
    patchJump(exit_jump);
    emitOp(OpCode::OP_POP, node.loc.line);

    for (int bj : currentScope()->break_jumps.back()) {
        patchJump(bj);
    }
    currentScope()->break_jumps.pop_back();
    currentScope()->continue_targets.pop_back();
}

void BytecodeCompiler::visit(ForEachStmt& node) {
    beginScope();
    node.collection_expr->accept(*this);
    addLocal("__col_" + std::to_string(node.loc.line), true, node.loc.line);
    emitConstant(Value::makeNumber(0, true), node.loc.line);
    addLocal("__idx_" + std::to_string(node.loc.line), false, node.loc.line);

    int col_slot = resolveLocal("__col_" + std::to_string(node.loc.line));
    int idx_slot = resolveLocal("__idx_" + std::to_string(node.loc.line));

    int loop_start = static_cast<int>(currentChunk()->code.size());
    currentScope()->continue_targets.push_back(loop_start);
    currentScope()->break_jumps.emplace_back();

    beginScope();
    // Fetch element: col[idx]
    emitOp(OpCode::OP_GET_LOCAL, node.loc.line);
    emitByte(static_cast<uint8_t>(col_slot), node.loc.line);
    emitOp(OpCode::OP_GET_LOCAL, node.loc.line);
    emitByte(static_cast<uint8_t>(idx_slot), node.loc.line);
    emitOp(OpCode::OP_GET_INDEX, node.loc.line);
    addLocal(node.item_name, false, node.loc.line);

    for (auto& stmt : node.body) {
        stmt->accept(*this);
    }
    endScope();

    // Increment idx
    emitOp(OpCode::OP_GET_LOCAL, node.loc.line);
    emitByte(static_cast<uint8_t>(idx_slot), node.loc.line);
    emitConstant(Value::makeNumber(1, true), node.loc.line);
    emitOp(OpCode::OP_ADD, node.loc.line);
    emitOp(OpCode::OP_SET_LOCAL, node.loc.line);
    emitByte(static_cast<uint8_t>(idx_slot), node.loc.line);
    emitOp(OpCode::OP_POP, node.loc.line);

    emitLoop(loop_start, node.loc.line);

    for (int bj : currentScope()->break_jumps.back()) {
        patchJump(bj);
    }
    currentScope()->break_jumps.pop_back();
    currentScope()->continue_targets.pop_back();

    endScope();
}

void BytecodeCompiler::visit(FuncDeclStmt& node) {
    CompilerScope fn_scope;
    fn_scope.function = std::make_shared<ObjFunction>(node.name);
    fn_scope.function->arity = static_cast<int>(node.params.size());
    fn_scope.function->param_names = node.params;
    scopes_.push_back(fn_scope);

    beginScope();
    for (const auto& param : node.params) {
        addLocal(param, false, node.loc.line);
    }

    for (auto& stmt : node.body) {
        stmt->accept(*this);
    }

    emitOp(OpCode::OP_NULL, node.loc.line);
    emitOp(OpCode::OP_RETURN, node.loc.line);
    endScope();

    auto compiled_fn = scopes_.back().function;
    scopes_.pop_back();

    Value fn_val;
    fn_val.type = ValueType::VAL_FUNCTION;
    fn_val.as_function = compiled_fn;

    size_t name_idx = currentChunk()->addConstant(Value::makeString(node.name));
    emitConstant(fn_val, node.loc.line);
    emitOp(OpCode::OP_DEFINE_GLOBAL, node.loc.line);
    emitByte(static_cast<uint8_t>(name_idx), node.loc.line);
}

void BytecodeCompiler::visit(ReturnStmt& node) {
    if (node.value) {
        node.value->accept(*this);
    } else {
        emitOp(OpCode::OP_NULL, node.loc.line);
    }
    emitOp(OpCode::OP_RETURN, node.loc.line);
}

void BytecodeCompiler::visit(ClassDeclStmt& node) {
    auto klass = std::make_shared<ObjClass>(node.name);
    klass->fields = node.fields;

    for (auto& m : node.methods) {
        CompilerScope m_scope;
        m_scope.function = std::make_shared<ObjFunction>(m->name);
        m_scope.function->arity = static_cast<int>(m->params.size());
        m_scope.function->param_names = m->params;
        scopes_.push_back(m_scope);

        beginScope();
        for (const auto& param : m->params) {
            addLocal(param, false, m->loc.line);
        }
        for (auto& s : m->body) {
            s->accept(*this);
        }
        emitOp(OpCode::OP_NULL, m->loc.line);
        emitOp(OpCode::OP_RETURN, m->loc.line);
        endScope();

        klass->methods[m->name] = scopes_.back().function;
        scopes_.pop_back();
    }

    Value cls_val;
    cls_val.type = ValueType::VAL_CLASS;
    cls_val.as_class = klass;

    size_t name_idx = currentChunk()->addConstant(Value::makeString(node.name));
    emitConstant(cls_val, node.loc.line);
    emitOp(OpCode::OP_DEFINE_GLOBAL, node.loc.line);
    emitByte(static_cast<uint8_t>(name_idx), node.loc.line);
}

void BytecodeCompiler::visit(TryCatchStmt& node) {
    for (auto& stmt : node.try_block) {
        stmt->accept(*this);
    }
}

void BytecodeCompiler::visit(ImportStmt& node) {
    // Loaded at runtime
}

void BytecodeCompiler::visit(BreakStmt& node) {
    if (!currentScope()->break_jumps.empty()) {
        int bj = emitJump(OpCode::OP_JUMP, node.loc.line);
        currentScope()->break_jumps.back().push_back(bj);
    }
}

void BytecodeCompiler::visit(ContinueStmt& node) {
    if (!currentScope()->continue_targets.empty()) {
        emitLoop(currentScope()->continue_targets.back(), node.loc.line);
    }
}

void BytecodeCompiler::visit(ExprStmt& node) {
    node.expr->accept(*this);
    emitOp(OpCode::OP_POP, node.loc.line);
}

void BytecodeCompiler::visit(Program& node) {
    for (auto& stmt : node.statements) {
        stmt->accept(*this);
    }
}

// ==================== Virtual Machine ====================

VM::VM() {
    initStandardLibrary();
}

void VM::defineNative(const std::string& name, NativeFn fn) {
    globals_[name] = Value::makeNative(fn);
}

void VM::defineGlobal(const std::string& name, Value value) {
    globals_[name] = value;
}

Value* VM::getGlobal(const std::string& name) {
    auto it = globals_.find(name);
    if (it != globals_.end()) return &it->second;
    return nullptr;
}

void VM::push(Value val) {
    stack_.push_back(val);
}

Value VM::pop() {
    if (stack_.empty()) return Value::makeNull();
    Value v = stack_.back();
    stack_.pop_back();
    return v;
}

Value VM::peek(int distance) const {
    if (stack_.size() <= static_cast<size_t>(distance)) return Value::makeNull();
    return stack_[stack_.size() - 1 - distance];
}

void VM::runtimeError(const std::string& message) {
    has_runtime_error_ = true;
    std::ostringstream ss;
    ss << "Runtime Error: " << message << "\n";
    if (!frames_.empty()) {
        ss << "Call Stack (most recent call first):\n";
        for (int i = static_cast<int>(frames_.size()) - 1; i >= 0; --i) {
            auto& f = frames_[i];
            int line = 1;
            if (f.ip > 0 && f.ip - 1 < f.function->chunk.lines.size()) {
                line = f.function->chunk.lines[f.ip - 1];
            }
            ss << "  in " << f.function->name << "() at line " << line << "\n";
        }
    }
    stderr_stream_ << ss.str();
}

void VM::initStandardLibrary() {
    StandardLibrary::loadAll(*this);
}

InterpretResult VM::interpretSource(const std::string& source, const std::string& filename) {
    clearOutput();
    has_runtime_error_ = false;

    DiagnosticEngine diag;
    Lexer lexer(source, filename, &diag);
    auto tokens = lexer.tokenize();

    if (diag.hasErrors()) {
        for (const auto& d : diag.getDiagnostics()) {
            stderr_stream_ << d.toString() << "\n";
        }
        return InterpretResult::COMPILE_ERROR;
    }

    Parser parser(tokens, diag, source);
    auto program = parser.parse();

    if (diag.hasErrors() || !program) {
        for (const auto& d : diag.getDiagnostics()) {
            stderr_stream_ << d.toString() << "\n";
        }
        return InterpretResult::COMPILE_ERROR;
    }

    SemanticAnalyzer sema(diag);
    if (!sema.analyze(*program)) {
        for (const auto& d : diag.getDiagnostics()) {
            stderr_stream_ << d.toString() << "\n";
        }
        return InterpretResult::COMPILE_ERROR;
    }

    BytecodeCompiler compiler(diag);
    auto main_fn = compiler.compile(*program);

    if (diag.hasErrors() || !main_fn) {
        for (const auto& d : diag.getDiagnostics()) {
            stderr_stream_ << d.toString() << "\n";
        }
        return InterpretResult::COMPILE_ERROR;
    }

    return interpret(main_fn);
}

InterpretResult VM::interpret(std::shared_ptr<ObjFunction> function) {
    stack_.clear();
    frames_.clear();

    CallFrame initial_frame;
    initial_frame.function = function;
    initial_frame.ip = 0;
    initial_frame.stack_base = 0;
    frames_.push_back(initial_frame);

    while (!frames_.empty()) {
        CallFrame* frame = &frames_.back();
        Chunk* chunk = &frame->function->chunk;

        if (frame->ip >= chunk->code.size()) {
            frames_.pop_back();
            continue;
        }

        uint8_t instruction = chunk->code[frame->ip++];
        OpCode op = static_cast<OpCode>(instruction);

        switch (op) {
            case OpCode::OP_CONSTANT: {
                uint8_t const_idx = chunk->code[frame->ip++];
                push(chunk->constants[const_idx]);
                break;
            }
            case OpCode::OP_NULL: push(Value::makeNull()); break;
            case OpCode::OP_TRUE: push(Value::makeBool(true)); break;
            case OpCode::OP_FALSE: push(Value::makeBool(false)); break;
            case OpCode::OP_POP: pop(); break;

            case OpCode::OP_DEFINE_GLOBAL: {
                uint8_t const_idx = chunk->code[frame->ip++];
                std::string name = *chunk->constants[const_idx].as_string;
                globals_[name] = pop();
                break;
            }
            case OpCode::OP_GET_GLOBAL: {
                uint8_t const_idx = chunk->code[frame->ip++];
                std::string name = *chunk->constants[const_idx].as_string;
                auto it = globals_.find(name);
                if (it != globals_.end()) {
                    push(it->second);
                } else {
                    runtimeError("Undefined global variable '" + name + "'.");
                    return InterpretResult::RUNTIME_ERROR;
                }
                break;
            }
            case OpCode::OP_SET_GLOBAL: {
                uint8_t const_idx = chunk->code[frame->ip++];
                std::string name = *chunk->constants[const_idx].as_string;
                globals_[name] = peek(0);
                break;
            }
            case OpCode::OP_GET_LOCAL: {
                uint8_t slot = chunk->code[frame->ip++];
                push(stack_[frame->stack_base + slot]);
                break;
            }
            case OpCode::OP_SET_LOCAL: {
                uint8_t slot = chunk->code[frame->ip++];
                stack_[frame->stack_base + slot] = peek(0);
                break;
            }
            case OpCode::OP_EQUAL: {
                Value b = pop();
                Value a = pop();
                push(Value::makeBool(a == b));
                break;
            }
            case OpCode::OP_GREATER: {
                Value b = pop();
                Value a = pop();
                if (a.type == ValueType::VAL_NUMBER && b.type == ValueType::VAL_NUMBER) {
                    push(Value::makeBool(a.as_number > b.as_number));
                } else {
                    push(Value::makeBool(false));
                }
                break;
            }
            case OpCode::OP_LESS: {
                Value b = pop();
                Value a = pop();
                if (a.type == ValueType::VAL_NUMBER && b.type == ValueType::VAL_NUMBER) {
                    push(Value::makeBool(a.as_number < b.as_number));
                } else {
                    push(Value::makeBool(false));
                }
                break;
            }
            case OpCode::OP_ADD: {
                Value b = pop();
                Value a = pop();
                if (a.type == ValueType::VAL_STRING || b.type == ValueType::VAL_STRING) {
                    push(Value::makeString(a.toString() + b.toString()));
                } else if (a.type == ValueType::VAL_NUMBER && b.type == ValueType::VAL_NUMBER) {
                    push(Value::makeNumber(a.as_number + b.as_number, a.is_integer && b.is_integer));
                } else {
                    runtimeError("Operands must be numbers or text for addition.");
                    return InterpretResult::RUNTIME_ERROR;
                }
                break;
            }
            case OpCode::OP_SUBTRACT: {
                Value b = pop();
                Value a = pop();
                if (a.type == ValueType::VAL_NUMBER && b.type == ValueType::VAL_NUMBER) {
                    push(Value::makeNumber(a.as_number - b.as_number, a.is_integer && b.is_integer));
                } else {
                    runtimeError("Operands must be numbers for subtraction.");
                    return InterpretResult::RUNTIME_ERROR;
                }
                break;
            }
            case OpCode::OP_MULTIPLY: {
                Value b = pop();
                Value a = pop();
                if (a.type == ValueType::VAL_NUMBER && b.type == ValueType::VAL_NUMBER) {
                    push(Value::makeNumber(a.as_number * b.as_number, a.is_integer && b.is_integer));
                } else {
                    runtimeError("Operands must be numbers for multiplication.");
                    return InterpretResult::RUNTIME_ERROR;
                }
                break;
            }
            case OpCode::OP_DIVIDE: {
                Value b = pop();
                Value a = pop();
                if (a.type == ValueType::VAL_NUMBER && b.type == ValueType::VAL_NUMBER) {
                    if (b.as_number == 0.0) {
                        runtimeError("Division by zero.");
                        return InterpretResult::RUNTIME_ERROR;
                    }
                    push(Value::makeNumber(a.as_number / b.as_number, false));
                } else {
                    runtimeError("Operands must be numbers for division.");
                    return InterpretResult::RUNTIME_ERROR;
                }
                break;
            }
            case OpCode::OP_MODULO: {
                Value b = pop();
                Value a = pop();
                if (a.type == ValueType::VAL_NUMBER && b.type == ValueType::VAL_NUMBER) {
                    if (b.as_number == 0.0) {
                        runtimeError("Modulo by zero.");
                        return InterpretResult::RUNTIME_ERROR;
                    }
                    push(Value::makeNumber(std::fmod(a.as_number, b.as_number), true));
                } else {
                    runtimeError("Operands must be numbers for modulo.");
                    return InterpretResult::RUNTIME_ERROR;
                }
                break;
            }
            case OpCode::OP_POWER: {
                Value b = pop();
                Value a = pop();
                if (a.type == ValueType::VAL_NUMBER && b.type == ValueType::VAL_NUMBER) {
                    push(Value::makeNumber(std::pow(a.as_number, b.as_number), false));
                } else {
                    runtimeError("Operands must be numbers for exponentiation.");
                    return InterpretResult::RUNTIME_ERROR;
                }
                break;
            }
            case OpCode::OP_NOT: {
                Value v = pop();
                push(Value::makeBool(!v.isTruthy()));
                break;
            }
            case OpCode::OP_NEGATE: {
                Value v = pop();
                if (v.type == ValueType::VAL_NUMBER) {
                    push(Value::makeNumber(-v.as_number, v.is_integer));
                } else {
                    runtimeError("Operand must be a number for negation.");
                    return InterpretResult::RUNTIME_ERROR;
                }
                break;
            }
            case OpCode::OP_PRINT: {
                uint8_t count = chunk->code[frame->ip++];
                std::vector<Value> args(count);
                for (int i = count - 1; i >= 0; --i) {
                    args[i] = pop();
                }
                for (size_t i = 0; i < args.size(); ++i) {
                    if (i > 0) stdout_stream_ << " ";
                    stdout_stream_ << args[i].toString();
                }
                stdout_stream_ << "\n";
                break;
            }
            case OpCode::OP_JUMP: {
                uint16_t offset = static_cast<uint16_t>(chunk->code[frame->ip] << 8) | chunk->code[frame->ip + 1];
                frame->ip += 2;
                frame->ip += offset;
                break;
            }
            case OpCode::OP_JUMP_IF_FALSE: {
                uint16_t offset = static_cast<uint16_t>(chunk->code[frame->ip] << 8) | chunk->code[frame->ip + 1];
                frame->ip += 2;
                if (!peek(0).isTruthy()) {
                    frame->ip += offset;
                }
                break;
            }
            case OpCode::OP_LOOP: {
                uint16_t offset = static_cast<uint16_t>(chunk->code[frame->ip] << 8) | chunk->code[frame->ip + 1];
                frame->ip += 2;
                frame->ip -= offset;
                break;
            }
            case OpCode::OP_CALL: {
                uint8_t arg_count = chunk->code[frame->ip++];
                Value callee = peek(arg_count);

                if (callee.type == ValueType::VAL_NATIVE_FN) {
                    std::vector<Value> args(arg_count);
                    for (int i = arg_count - 1; i >= 0; --i) {
                        args[i] = pop();
                    }
                    pop(); // Pop callee
                    Value result = callee.as_native(*this, arg_count, args.data());
                    push(result);
                } else if (callee.type == ValueType::VAL_FUNCTION) {
                    auto fn = callee.as_function;
                    CallFrame new_frame;
                    new_frame.function = fn;
                    new_frame.ip = 0;
                    new_frame.stack_base = stack_.size() - arg_count;
                    frames_.push_back(new_frame);
                } else if (callee.type == ValueType::VAL_CLASS) {
                    // Instantiate class
                    auto inst = std::make_shared<ObjInstance>(callee.as_class);
                    pop(); // pop class
                    Value inst_val;
                    inst_val.type = ValueType::VAL_INSTANCE;
                    inst_val.as_instance = inst;
                    push(inst_val);
                } else {
                    runtimeError("Can only call functions and classes.");
                    return InterpretResult::RUNTIME_ERROR;
                }
                break;
            }
            case OpCode::OP_RETURN: {
                Value result = pop();
                size_t base = frame->stack_base;
                frames_.pop_back();

                // Unwind stack to before callee
                size_t target_size = (base > 0) ? (base - 1) : 0;
                while (stack_.size() > target_size) {
                    stack_.pop_back();
                }
                push(result);
                break;
            }
            case OpCode::OP_BUILD_LIST: {
                uint8_t count = chunk->code[frame->ip++];
                std::vector<Value> elements(count);
                for (int i = count - 1; i >= 0; --i) {
                    elements[i] = pop();
                }
                push(Value::makeList(elements));
                break;
            }
            case OpCode::OP_BUILD_DICT: {
                uint8_t pair_count = chunk->code[frame->ip++];
                Value dict = Value::makeDict();
                for (int i = 0; i < pair_count; ++i) {
                    Value val = pop();
                    Value key = pop();
                    (*dict.as_dict)[key.toString()] = val;
                }
                push(dict);
                break;
            }
            case OpCode::OP_GET_INDEX: {
                Value index = pop();
                Value col = pop();
                if (col.type == ValueType::VAL_LIST) {
                    if (index.type == ValueType::VAL_NUMBER) {
                        int idx = static_cast<int>(index.as_number);
                        if (idx >= 0 && idx < static_cast<int>(col.as_list->size())) {
                            push((*col.as_list)[idx]);
                        } else {
                            push(Value::makeNull());
                        }
                    } else {
                        push(Value::makeNull());
                    }
                } else if (col.type == ValueType::VAL_DICT) {
                    std::string key = index.toString();
                    auto it = col.as_dict->find(key);
                    if (it != col.as_dict->end()) {
                        push(it->second);
                    } else {
                        push(Value::makeNull());
                    }
                } else if (col.type == ValueType::VAL_STRING) {
                    if (index.type == ValueType::VAL_NUMBER) {
                        int idx = static_cast<int>(index.as_number);
                        if (idx >= 0 && idx < static_cast<int>(col.as_string->size())) {
                            push(Value::makeString(std::string(1, (*col.as_string)[idx])));
                        } else {
                            push(Value::makeNull());
                        }
                    } else {
                        push(Value::makeNull());
                    }
                } else {
                    push(Value::makeNull());
                }
                break;
            }
            case OpCode::OP_SET_INDEX: {
                Value val = pop();
                Value index = pop();
                Value col = pop();
                if (col.type == ValueType::VAL_LIST && index.type == ValueType::VAL_NUMBER) {
                    int idx = static_cast<int>(index.as_number);
                    if (idx >= 0 && idx < static_cast<int>(col.as_list->size())) {
                        (*col.as_list)[idx] = val;
                    }
                } else if (col.type == ValueType::VAL_DICT) {
                    (*col.as_dict)[index.toString()] = val;
                }
                push(val);
                break;
            }
            case OpCode::OP_GET_PROPERTY: {
                uint8_t const_idx = chunk->code[frame->ip++];
                std::string prop = *chunk->constants[const_idx].as_string;
                Value obj = pop();
                if (obj.type == ValueType::VAL_INSTANCE && obj.as_instance) {
                    auto it = obj.as_instance->fields.find(prop);
                    if (it != obj.as_instance->fields.end()) {
                        push(it->second);
                    } else {
                        push(Value::makeNull());
                    }
                } else {
                    push(Value::makeNull());
                }
                break;
            }
            case OpCode::OP_SET_PROPERTY: {
                uint8_t const_idx = chunk->code[frame->ip++];
                std::string prop = *chunk->constants[const_idx].as_string;
                Value val = pop();
                Value obj = pop();
                if (obj.type == ValueType::VAL_INSTANCE && obj.as_instance) {
                    obj.as_instance->fields[prop] = val;
                }
                push(val);
                break;
            }
            case OpCode::OP_HALT:
                return InterpretResult::OK;
        }
    }

    return InterpretResult::OK;
}

} // namespace huma
