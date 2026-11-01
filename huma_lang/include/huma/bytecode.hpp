#pragma once

#include "huma/common.hpp"
#include <string>
#include <vector>
#include <memory>
#include <map>
#include <functional>
#include <iostream>

namespace huma {

enum class OpCode : uint8_t {
    OP_CONSTANT,
    OP_NULL,
    OP_TRUE,
    OP_FALSE,
    OP_POP,
    OP_DEFINE_GLOBAL,
    OP_GET_GLOBAL,
    OP_SET_GLOBAL,
    OP_GET_LOCAL,
    OP_SET_LOCAL,
    OP_EQUAL,
    OP_GREATER,
    OP_LESS,
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_MODULO,
    OP_POWER,
    OP_NOT,
    OP_NEGATE,
    OP_PRINT,
    OP_JUMP,
    OP_JUMP_IF_FALSE,
    OP_LOOP,
    OP_CALL,
    OP_RETURN,
    OP_BUILD_LIST,
    OP_BUILD_DICT,
    OP_GET_INDEX,
    OP_SET_INDEX,
    OP_GET_PROPERTY,
    OP_SET_PROPERTY,
    OP_HALT
};

std::string opCodeToString(OpCode op);

// Forward declarations
struct Value;
class Chunk;
class VM;

using NativeFn = std::function<Value(VM& vm, int arg_count, Value* args)>;

enum class ValueType {
    VAL_NULL,
    VAL_BOOL,
    VAL_NUMBER,
    VAL_STRING,
    VAL_LIST,
    VAL_DICT,
    VAL_FUNCTION,
    VAL_NATIVE_FN,
    VAL_CLASS,
    VAL_INSTANCE
};

struct ObjFunction;
struct ObjClass;
struct ObjInstance;

struct Value {
    ValueType type{ValueType::VAL_NULL};
    bool as_bool{false};
    double as_number{0.0};
    bool is_integer{false};
    std::shared_ptr<std::string> as_string{nullptr};
    std::shared_ptr<std::vector<Value>> as_list{nullptr};
    std::shared_ptr<std::map<std::string, Value>> as_dict{nullptr};
    std::shared_ptr<ObjFunction> as_function{nullptr};
    NativeFn as_native{nullptr};
    std::shared_ptr<ObjClass> as_class{nullptr};
    std::shared_ptr<ObjInstance> as_instance{nullptr};

    static Value makeNull();
    static Value makeBool(bool b);
    static Value makeNumber(double n, bool is_int = false);
    static Value makeString(const std::string& str);
    static Value makeList(const std::vector<Value>& items = {});
    static Value makeDict();
    static Value makeNative(NativeFn fn);

    std::string toString() const;
    bool isTruthy() const;
    bool operator==(const Value& other) const;
};

class Chunk {
public:
    std::vector<uint8_t> code;
    std::vector<Value> constants;
    std::vector<int> lines;

    void write(uint8_t byte, int line);
    void writeOp(OpCode op, int line);
    size_t addConstant(Value val);
    std::string disassemble(const std::string& name = "main") const;
    int disassembleInstruction(int offset, std::ostringstream& ss) const;
};

struct ObjFunction {
    std::string name;
    int arity{0};
    std::vector<std::string> param_names;
    Chunk chunk;

    ObjFunction(std::string n = "") : name(std::move(n)) {}
};

struct ObjClass {
    std::string name;
    std::vector<std::string> fields;
    std::map<std::string, std::shared_ptr<ObjFunction>> methods;

    ObjClass(std::string n = "") : name(std::move(n)) {}
};

struct ObjInstance {
    std::shared_ptr<ObjClass> klass;
    std::map<std::string, Value> fields;

    ObjInstance(std::shared_ptr<ObjClass> k) : klass(std::move(k)) {}
};

} // namespace huma
