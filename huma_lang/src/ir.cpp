#include "huma/ir.hpp"
#include <sstream>
#include <cmath>

namespace huma {

std::string IRInstruction::toString() const {
    std::ostringstream ss;
    switch (op) {
        case IROp::LOAD_CONST:
            ss << result << " = " << arg1;
            break;
        case IROp::ASSIGN:
            ss << result << " = " << arg1;
            break;
        case IROp::ADD:
            ss << result << " = " << arg1 << " + " << arg2;
            break;
        case IROp::SUB:
            ss << result << " = " << arg1 << " - " << arg2;
            break;
        case IROp::MUL:
            ss << result << " = " << arg1 << " * " << arg2;
            break;
        case IROp::DIV:
            ss << result << " = " << arg1 << " / " << arg2;
            break;
        case IROp::MOD:
            ss << result << " = " << arg1 << " % " << arg2;
            break;
        case IROp::POW:
            ss << result << " = " << arg1 << " ^ " << arg2;
            break;
        case IROp::EQUAL:
            ss << result << " = (" << arg1 << " == " << arg2 << ")";
            break;
        case IROp::NOT_EQUAL:
            ss << result << " = (" << arg1 << " != " << arg2 << ")";
            break;
        case IROp::GREATER:
            ss << result << " = (" << arg1 << " > " << arg2 << ")";
            break;
        case IROp::LESS:
            ss << result << " = (" << arg1 << " < " << arg2 << ")";
            break;
        case IROp::GREATER_EQUAL:
            ss << result << " = (" << arg1 << " >= " << arg2 << ")";
            break;
        case IROp::LESS_EQUAL:
            ss << result << " = (" << arg1 << " <= " << arg2 << ")";
            break;
        case IROp::LOGIC_AND:
            ss << result << " = " << arg1 << " and " << arg2;
            break;
        case IROp::LOGIC_OR:
            ss << result << " = " << arg1 << " or " << arg2;
            break;
        case IROp::LOGIC_NOT:
            ss << result << " = not " << arg1;
            break;
        case IROp::NEGATE:
            ss << result << " = -" << arg1;
            break;
        case IROp::LABEL:
            ss << result << ":";
            break;
        case IROp::JUMP:
            ss << "jump " << result;
            break;
        case IROp::JUMP_IF_FALSE:
            ss << "jump_if_false " << arg1 << " to " << result;
            break;
        case IROp::CALL:
            ss << result << " = call " << arg1 << " [";
            for (size_t i = 0; i < var_args.size(); ++i) {
                if (i > 0) ss << ", ";
                ss << var_args[i];
            }
            ss << "]";
            break;
        case IROp::RETURN:
            ss << "return " << arg1;
            break;
        case IROp::PRINT:
            ss << "print " << arg1;
            break;
        case IROp::MAKE_LIST:
            ss << result << " = list [";
            for (size_t i = 0; i < var_args.size(); ++i) {
                if (i > 0) ss << ", ";
                ss << var_args[i];
            }
            ss << "]";
            break;
        case IROp::MAKE_DICT:
            ss << result << " = dict";
            break;
        case IROp::GET_INDEX:
            ss << result << " = " << arg1 << "[" << arg2 << "]";
            break;
        case IROp::SET_INDEX:
            ss << arg1 << "[" << arg2 << "] = " << result;
            break;
        case IROp::GET_MEMBER:
            ss << result << " = " << arg1 << "." << arg2;
            break;
        case IROp::SET_MEMBER:
            ss << arg1 << "." << arg2 << " = " << result;
            break;
        case IROp::NOP:
            ss << "nop";
            break;
    }
    return ss.str();
}

std::string IRProgram::dump() const {
    std::ostringstream ss;
    ss << "=== HUMA X Intermediate Representation (TAC) ===\n";
    for (const auto& instr : instructions) {
        if (instr.op == IROp::LABEL) {
            ss << instr.toString() << "\n";
        } else {
            ss << "    " << instr.toString() << "\n";
        }
    }

    for (const auto& fn : functions) {
        ss << "\nFunction @" << fn.name << "(";
        for (size_t i = 0; i < fn.params.size(); ++i) {
            if (i > 0) ss << ", ";
            ss << fn.params[i];
        }
        ss << "):\n";
        for (const auto& block : fn.blocks) {
            ss << block.label << ":\n";
            for (const auto& instr : block.instructions) {
                ss << "    " << instr.toString() << "\n";
            }
        }
    }
    return ss.str();
}

IRGenerator::IRGenerator() = default;

std::string IRGenerator::newTemp() {
    return "%t" + std::to_string(temp_counter_++);
}

std::string IRGenerator::newLabel(const std::string& prefix) {
    return prefix + "_" + std::to_string(label_counter_++);
}

void IRGenerator::emit(IROp op, const std::string& res, const std::string& a1, const std::string& a2) {
    IRInstruction instr;
    instr.op = op;
    instr.result = res;
    instr.arg1 = a1;
    instr.arg2 = a2;
    ir_prog_.instructions.push_back(instr);
}

IRProgram IRGenerator::generate(Program& program) {
    program.accept(*this);
    IROptimizer::optimize(ir_prog_);
    return ir_prog_;
}

void IRGenerator::visit(LiteralExpr& node) {
    std::string temp = newTemp();
    switch (node.lit_type) {
        case LiteralType::NUMBER:
            if (node.is_integer) {
                emit(IROp::LOAD_CONST, temp, std::to_string(static_cast<int64_t>(node.number_val)));
            } else {
                emit(IROp::LOAD_CONST, temp, std::to_string(node.number_val));
            }
            break;
        case LiteralType::STRING:
            emit(IROp::LOAD_CONST, temp, "\"" + node.string_val + "\"");
            break;
        case LiteralType::BOOLEAN:
            emit(IROp::LOAD_CONST, temp, node.bool_val ? "yes" : "no");
            break;
        case LiteralType::NOTHING:
            emit(IROp::LOAD_CONST, temp, "nothing");
            break;
    }
    last_temp_ = temp;
}

void IRGenerator::visit(IdentifierExpr& node) {
    last_temp_ = node.name;
}

void IRGenerator::visit(BinaryExpr& node) {
    node.left->accept(*this);
    std::string left = last_temp_;
    node.right->accept(*this);
    std::string right = last_temp_;

    std::string res = newTemp();
    IROp op = IROp::ADD;
    switch (node.op) {
        case BinaryOp::ADD: op = IROp::ADD; break;
        case BinaryOp::SUB: op = IROp::SUB; break;
        case BinaryOp::MUL: op = IROp::MUL; break;
        case BinaryOp::DIV: op = IROp::DIV; break;
        case BinaryOp::MOD: op = IROp::MOD; break;
        case BinaryOp::POW: op = IROp::POW; break;
        case BinaryOp::EQUAL: op = IROp::EQUAL; break;
        case BinaryOp::NOT_EQUAL: op = IROp::NOT_EQUAL; break;
        case BinaryOp::GREATER: op = IROp::GREATER; break;
        case BinaryOp::LESS: op = IROp::LESS; break;
        case BinaryOp::GREATER_EQUAL: op = IROp::GREATER_EQUAL; break;
        case BinaryOp::LESS_EQUAL: op = IROp::LESS_EQUAL; break;
        case BinaryOp::AND: op = IROp::LOGIC_AND; break;
        case BinaryOp::OR: op = IROp::LOGIC_OR; break;
    }
    emit(op, res, left, right);
    last_temp_ = res;
}

void IRGenerator::visit(UnaryExpr& node) {
    node.operand->accept(*this);
    std::string res = newTemp();
    if (node.op == UnaryOp::NOT) {
        emit(IROp::LOGIC_NOT, res, last_temp_);
    } else {
        emit(IROp::NEGATE, res, last_temp_);
    }
    last_temp_ = res;
}

void IRGenerator::visit(CallExpr& node) {
    std::vector<std::string> arg_temps;
    for (auto& arg : node.args) {
        arg->accept(*this);
        arg_temps.push_back(last_temp_);
    }

    std::string res = newTemp();
    IRInstruction instr;
    instr.op = IROp::CALL;
    instr.result = res;
    instr.arg1 = node.callee;
    instr.var_args = arg_temps;
    ir_prog_.instructions.push_back(instr);
    last_temp_ = res;
}

void IRGenerator::visit(ListExpr& node) {
    std::vector<std::string> elem_temps;
    for (auto& elem : node.elements) {
        elem->accept(*this);
        elem_temps.push_back(last_temp_);
    }
    std::string res = newTemp();
    IRInstruction instr;
    instr.op = IROp::MAKE_LIST;
    instr.result = res;
    instr.var_args = elem_temps;
    ir_prog_.instructions.push_back(instr);
    last_temp_ = res;
}

void IRGenerator::visit(DictExpr& node) {
    std::string res = newTemp();
    emit(IROp::MAKE_DICT, res);
    for (auto& entry : node.entries) {
        entry.second->accept(*this);
        emit(IROp::SET_INDEX, last_temp_, res, "\"" + entry.first + "\"");
    }
    last_temp_ = res;
}

void IRGenerator::visit(IndexExpr& node) {
    node.collection->accept(*this);
    std::string col = last_temp_;
    node.index->accept(*this);
    std::string idx = last_temp_;

    std::string res = newTemp();
    emit(IROp::GET_INDEX, res, col, idx);
    last_temp_ = res;
}

void IRGenerator::visit(MemberExpr& node) {
    node.object->accept(*this);
    std::string obj = last_temp_;
    std::string res = newTemp();
    emit(IROp::GET_MEMBER, res, obj, node.member);
    last_temp_ = res;
}

void IRGenerator::visit(VarDeclStmt& node) {
    node.init_expr->accept(*this);
    emit(IROp::ASSIGN, node.name, last_temp_);
}

void IRGenerator::visit(ChangeStmt& node) {
    node.value_expr->accept(*this);
    emit(IROp::ASSIGN, node.target_name, last_temp_);
}

void IRGenerator::visit(ShowStmt& node) {
    for (auto& expr : node.expressions) {
        expr->accept(*this);
        emit(IROp::PRINT, "", last_temp_);
    }
}

void IRGenerator::visit(IfStmt& node) {
    node.condition->accept(*this);
    std::string cond = last_temp_;
    std::string false_label = newLabel("else");
    std::string end_label = newLabel("endif");

    emit(IROp::JUMP_IF_FALSE, false_label, cond);
    for (auto& stmt : node.then_branch) {
        stmt->accept(*this);
    }
    emit(IROp::JUMP, end_label);

    emit(IROp::LABEL, false_label);
    for (auto& stmt : node.otherwise_branch) {
        stmt->accept(*this);
    }
    emit(IROp::LABEL, end_label);
}

void IRGenerator::visit(RepeatTimesStmt& node) {
    node.count_expr->accept(*this);
    std::string count = last_temp_;
    std::string idx = newTemp();
    emit(IROp::LOAD_CONST, idx, "0");

    std::string loop_start = newLabel("loop_start");
    std::string loop_end = newLabel("loop_end");

    emit(IROp::LABEL, loop_start);
    std::string cond = newTemp();
    emit(IROp::LESS, cond, idx, count);
    emit(IROp::JUMP_IF_FALSE, loop_end, cond);

    break_labels_.push_back(loop_end);
    continue_labels_.push_back(loop_start);

    for (auto& stmt : node.body) {
        stmt->accept(*this);
    }

    break_labels_.pop_back();
    continue_labels_.pop_back();

    std::string one = newTemp();
    emit(IROp::LOAD_CONST, one, "1");
    emit(IROp::ADD, idx, idx, one);
    emit(IROp::JUMP, loop_start);
    emit(IROp::LABEL, loop_end);
}

void IRGenerator::visit(WhileStmt& node) {
    std::string loop_start = newLabel("while_start");
    std::string loop_end = newLabel("while_end");

    emit(IROp::LABEL, loop_start);
    node.condition->accept(*this);
    emit(IROp::JUMP_IF_FALSE, loop_end, last_temp_);

    break_labels_.push_back(loop_end);
    continue_labels_.push_back(loop_start);

    for (auto& stmt : node.body) {
        stmt->accept(*this);
    }

    break_labels_.pop_back();
    continue_labels_.pop_back();

    emit(IROp::JUMP, loop_start);
    emit(IROp::LABEL, loop_end);
}

void IRGenerator::visit(ForEachStmt& node) {
    node.collection_expr->accept(*this);
    std::string col = last_temp_;
    std::string idx = newTemp();
    emit(IROp::LOAD_CONST, idx, "0");

    std::string loop_start = newLabel("for_start");
    std::string loop_end = newLabel("for_end");

    emit(IROp::LABEL, loop_start);
    // get element
    emit(IROp::GET_INDEX, node.item_name, col, idx);

    break_labels_.push_back(loop_end);
    continue_labels_.push_back(loop_start);

    for (auto& stmt : node.body) {
        stmt->accept(*this);
    }

    break_labels_.pop_back();
    continue_labels_.pop_back();

    std::string one = newTemp();
    emit(IROp::LOAD_CONST, one, "1");
    emit(IROp::ADD, idx, idx, one);
    emit(IROp::JUMP, loop_start);
    emit(IROp::LABEL, loop_end);
}

void IRGenerator::visit(FuncDeclStmt& node) {
    std::string fn_label = "func_" + node.name;
    std::string end_label = "func_end_" + node.name;
    emit(IROp::JUMP, end_label);
    emit(IROp::LABEL, fn_label);

    for (auto& stmt : node.body) {
        stmt->accept(*this);
    }
    emit(IROp::RETURN, "", "nothing");
    emit(IROp::LABEL, end_label);
}

void IRGenerator::visit(ReturnStmt& node) {
    if (node.value) {
        node.value->accept(*this);
        emit(IROp::RETURN, "", last_temp_);
    } else {
        emit(IROp::RETURN, "", "nothing");
    }
}

void IRGenerator::visit(ClassDeclStmt& node) {
    for (auto& m : node.methods) {
        m->accept(*this);
    }
}

void IRGenerator::visit(TryCatchStmt& node) {
    for (auto& stmt : node.try_block) {
        stmt->accept(*this);
    }
}

void IRGenerator::visit(ImportStmt& node) {
    emit(IROp::NOP, "", "import " + node.module_name);
}

void IRGenerator::visit(BreakStmt& node) {
    if (!break_labels_.empty()) {
        emit(IROp::JUMP, break_labels_.back());
    }
}

void IRGenerator::visit(ContinueStmt& node) {
    if (!continue_labels_.empty()) {
        emit(IROp::JUMP, continue_labels_.back());
    }
}

void IRGenerator::visit(ExprStmt& node) {
    node.expr->accept(*this);
}

void IRGenerator::visit(Program& node) {
    for (auto& stmt : node.statements) {
        stmt->accept(*this);
    }
}

void IROptimizer::optimize(IRProgram& program) {
    bool changed = true;
    while (changed) {
        changed = false;
        changed |= constantFolding(program.instructions);
        changed |= deadCodeElimination(program.instructions);
    }
}

bool IROptimizer::constantFolding(std::vector<IRInstruction>& instrs) {
    bool changed = false;
    for (auto& instr : instrs) {
        if (instr.op == IROp::ADD || instr.op == IROp::SUB ||
            instr.op == IROp::MUL || instr.op == IROp::DIV) {
            try {
                if (!instr.arg1.empty() && !instr.arg2.empty() &&
                    (std::isdigit(instr.arg1[0]) || (instr.arg1[0] == '-' && instr.arg1.size() > 1 && std::isdigit(instr.arg1[1]))) &&
                    (std::isdigit(instr.arg2[0]) || (instr.arg2[0] == '-' && instr.arg2.size() > 1 && std::isdigit(instr.arg2[1])))) {
                    double a = std::stod(instr.arg1);
                    double b = std::stod(instr.arg2);
                    double res = 0.0;
                    if (instr.op == IROp::ADD) res = a + b;
                    else if (instr.op == IROp::SUB) res = a - b;
                    else if (instr.op == IROp::MUL) res = a * b;
                    else if (instr.op == IROp::DIV && b != 0) res = a / b;

                    instr.op = IROp::LOAD_CONST;
                    instr.arg1 = std::to_string(res);
                    instr.arg2 = "";
                    changed = true;
                }
            } catch (...) {}
        }
    }
    return changed;
}

bool IROptimizer::deadCodeElimination(std::vector<IRInstruction>& instrs) {
    std::vector<IRInstruction> new_instrs;
    bool skipping = false;
    bool changed = false;

    for (const auto& instr : instrs) {
        if (skipping) {
            if (instr.op == IROp::LABEL) {
                skipping = false;
                new_instrs.push_back(instr);
            } else {
                changed = true; // eliminated dead code instruction
            }
        } else {
            new_instrs.push_back(instr);
            if (instr.op == IROp::JUMP || instr.op == IROp::RETURN) {
                skipping = true;
            }
        }
    }
    instrs = std::move(new_instrs);
    return changed;
}

} // namespace huma
