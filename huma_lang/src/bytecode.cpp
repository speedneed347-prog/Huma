#include "huma/bytecode.hpp"
#include <iomanip>
#include <sstream>
#include <cmath>

namespace huma {

std::string opCodeToString(OpCode op) {
    switch (op) {
        case OpCode::OP_CONSTANT: return "OP_CONSTANT";
        case OpCode::OP_NULL: return "OP_NULL";
        case OpCode::OP_TRUE: return "OP_TRUE";
        case OpCode::OP_FALSE: return "OP_FALSE";
        case OpCode::OP_POP: return "OP_POP";
        case OpCode::OP_DEFINE_GLOBAL: return "OP_DEFINE_GLOBAL";
        case OpCode::OP_GET_GLOBAL: return "OP_GET_GLOBAL";
        case OpCode::OP_SET_GLOBAL: return "OP_SET_GLOBAL";
        case OpCode::OP_GET_LOCAL: return "OP_GET_LOCAL";
        case OpCode::OP_SET_LOCAL: return "OP_SET_LOCAL";
        case OpCode::OP_EQUAL: return "OP_EQUAL";
        case OpCode::OP_GREATER: return "OP_GREATER";
        case OpCode::OP_LESS: return "OP_LESS";
        case OpCode::OP_ADD: return "OP_ADD";
        case OpCode::OP_SUBTRACT: return "OP_SUBTRACT";
        case OpCode::OP_MULTIPLY: return "OP_MULTIPLY";
        case OpCode::OP_DIVIDE: return "OP_DIVIDE";
        case OpCode::OP_MODULO: return "OP_MODULO";
        case OpCode::OP_POWER: return "OP_POWER";
        case OpCode::OP_NOT: return "OP_NOT";
        case OpCode::OP_NEGATE: return "OP_NEGATE";
        case OpCode::OP_PRINT: return "OP_PRINT";
        case OpCode::OP_JUMP: return "OP_JUMP";
        case OpCode::OP_JUMP_IF_FALSE: return "OP_JUMP_IF_FALSE";
        case OpCode::OP_LOOP: return "OP_LOOP";
        case OpCode::OP_CALL: return "OP_CALL";
        case OpCode::OP_RETURN: return "OP_RETURN";
        case OpCode::OP_BUILD_LIST: return "OP_BUILD_LIST";
        case OpCode::OP_BUILD_DICT: return "OP_BUILD_DICT";
        case OpCode::OP_GET_INDEX: return "OP_GET_INDEX";
        case OpCode::OP_SET_INDEX: return "OP_SET_INDEX";
        case OpCode::OP_GET_PROPERTY: return "OP_GET_PROPERTY";
        case OpCode::OP_SET_PROPERTY: return "OP_SET_PROPERTY";
        case OpCode::OP_HALT: return "OP_HALT";
        default: return "UNKNOWN_OP";
    }
}

Value Value::makeNull() {
    Value v;
    v.type = ValueType::VAL_NULL;
    return v;
}

Value Value::makeBool(bool b) {
    Value v;
    v.type = ValueType::VAL_BOOL;
    v.as_bool = b;
    return v;
}

Value Value::makeNumber(double n, bool is_int) {
    Value v;
    v.type = ValueType::VAL_NUMBER;
    v.as_number = n;
    v.is_integer = is_int;
    return v;
}

Value Value::makeString(const std::string& str) {
    Value v;
    v.type = ValueType::VAL_STRING;
    v.as_string = std::make_shared<std::string>(str);
    return v;
}

Value Value::makeList(const std::vector<Value>& items) {
    Value v;
    v.type = ValueType::VAL_LIST;
    v.as_list = std::make_shared<std::vector<Value>>(items);
    return v;
}

Value Value::makeDict() {
    Value v;
    v.type = ValueType::VAL_DICT;
    v.as_dict = std::make_shared<std::map<std::string, Value>>();
    return v;
}

Value Value::makeNative(NativeFn fn) {
    Value v;
    v.type = ValueType::VAL_NATIVE_FN;
    v.as_native = fn;
    return v;
}

std::string Value::toString() const {
    switch (type) {
        case ValueType::VAL_NULL: return "nothing";
        case ValueType::VAL_BOOL: return as_bool ? "yes" : "no";
        case ValueType::VAL_NUMBER: {
            if (is_integer || std::floor(as_number) == as_number) {
                return std::to_string(static_cast<int64_t>(as_number));
            }
            std::ostringstream ss;
            ss << as_number;
            return ss.str();
        }
        case ValueType::VAL_STRING: return as_string ? *as_string : "";
        case ValueType::VAL_LIST: {
            if (!as_list) return "[]";
            std::ostringstream ss;
            ss << "[";
            for (size_t i = 0; i < as_list->size(); ++i) {
                if (i > 0) ss << ", ";
                ss << (*as_list)[i].toString();
            }
            ss << "]";
            return ss.str();
        }
        case ValueType::VAL_DICT: {
            if (!as_dict) return "{}";
            std::ostringstream ss;
            ss << "{";
            size_t idx = 0;
            for (const auto& pair : *as_dict) {
                if (idx > 0) ss << ", ";
                ss << "\"" << pair.first << "\": " << pair.second.toString();
                idx++;
            }
            ss << "}";
            return ss.str();
        }
        case ValueType::VAL_FUNCTION:
            return "<function " + (as_function ? as_function->name : "anonymous") + ">";
        case ValueType::VAL_NATIVE_FN:
            return "<native function>";
        case ValueType::VAL_CLASS:
            return "<class " + (as_class ? as_class->name : "") + ">";
        case ValueType::VAL_INSTANCE:
            return "<instance of " + (as_instance && as_instance->klass ? as_instance->klass->name : "object") + ">";
        default:
            return "<unknown>";
    }
}

bool Value::isTruthy() const {
    switch (type) {
        case ValueType::VAL_NULL: return false;
        case ValueType::VAL_BOOL: return as_bool;
        case ValueType::VAL_NUMBER: return as_number != 0.0;
        case ValueType::VAL_STRING: return as_string && !as_string->empty();
        case ValueType::VAL_LIST: return as_list && !as_list->empty();
        case ValueType::VAL_DICT: return as_dict && !as_dict->empty();
        default: return true;
    }
}

bool Value::operator==(const Value& other) const {
    if (type != other.type) return false;
    switch (type) {
        case ValueType::VAL_NULL: return true;
        case ValueType::VAL_BOOL: return as_bool == other.as_bool;
        case ValueType::VAL_NUMBER: return as_number == other.as_number;
        case ValueType::VAL_STRING:
            return (as_string && other.as_string) ? (*as_string == *other.as_string) : false;
        case ValueType::VAL_LIST:
            return as_list == other.as_list;
        case ValueType::VAL_DICT:
            return as_dict == other.as_dict;
        default:
            return false;
    }
}

void Chunk::write(uint8_t byte, int line) {
    code.push_back(byte);
    lines.push_back(line);
}

void Chunk::writeOp(OpCode op, int line) {
    write(static_cast<uint8_t>(op), line);
}

size_t Chunk::addConstant(Value val) {
    constants.push_back(val);
    return constants.size() - 1;
}

std::string Chunk::disassemble(const std::string& name) const {
    std::ostringstream ss;
    ss << "=== Bytecode Disassembly: " << name << " ===\n";
    ss << "Offset | Line | OpCode             | Details\n";
    ss << "-------+------+--------------------+-------------------------\n";

    int offset = 0;
    while (offset < static_cast<int>(code.size())) {
        offset = disassembleInstruction(offset, ss);
    }
    return ss.str();
}

int Chunk::disassembleInstruction(int offset, std::ostringstream& ss) const {
    ss << std::setfill('0') << std::setw(6) << offset << " | ";
    if (offset > 0 && lines[offset] == lines[offset - 1]) {
        ss << "   |  | ";
    } else {
        ss << std::setfill(' ') << std::setw(4) << lines[offset] << " | ";
    }

    uint8_t instruction = code[offset];
    OpCode op = static_cast<OpCode>(instruction);
    ss << std::left << std::setw(18) << opCodeToString(op) << " | ";

    switch (op) {
        case OpCode::OP_CONSTANT: {
            uint8_t const_idx = code[offset + 1];
            ss << "#" << static_cast<int>(const_idx) << " '" << constants[const_idx].toString() << "'\n";
            return offset + 2;
        }
        case OpCode::OP_DEFINE_GLOBAL:
        case OpCode::OP_GET_GLOBAL:
        case OpCode::OP_SET_GLOBAL: {
            uint8_t const_idx = code[offset + 1];
            ss << "#" << static_cast<int>(const_idx) << " '" << constants[const_idx].toString() << "'\n";
            return offset + 2;
        }
        case OpCode::OP_GET_LOCAL:
        case OpCode::OP_SET_LOCAL: {
            uint8_t slot = code[offset + 1];
            ss << "slot " << static_cast<int>(slot) << "\n";
            return offset + 2;
        }
        case OpCode::OP_JUMP:
        case OpCode::OP_JUMP_IF_FALSE: {
            uint16_t jump = static_cast<uint16_t>(code[offset + 1] << 8) | code[offset + 2];
            ss << "-> " << (offset + 3 + jump) << "\n";
            return offset + 3;
        }
        case OpCode::OP_LOOP: {
            uint16_t jump = static_cast<uint16_t>(code[offset + 1] << 8) | code[offset + 2];
            ss << "-> " << (offset + 3 - jump) << "\n";
            return offset + 3;
        }
        case OpCode::OP_PRINT: {
            uint8_t val_count = code[offset + 1];
            ss << val_count << " values\n";
            return offset + 2;
        }
        case OpCode::OP_CALL: {
            uint8_t arg_count = code[offset + 1];
            ss << arg_count << " args\n";
            return offset + 2;
        }
        case OpCode::OP_BUILD_LIST: {
            uint8_t elem_count = code[offset + 1];
            ss << elem_count << " elements\n";
            return offset + 2;
        }
        case OpCode::OP_BUILD_DICT: {
            uint8_t pair_count = code[offset + 1];
            ss << pair_count << " pairs\n";
            return offset + 2;
        }
        default:
            ss << "\n";
            return offset + 1;
    }
}

} // namespace huma
