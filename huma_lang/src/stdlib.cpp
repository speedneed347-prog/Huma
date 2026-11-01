#include "huma/stdlib.hpp"
#include <cmath>
#include <chrono>
#include <thread>
#include <fstream>
#include <sstream>
#include <random>
#include <algorithm>

namespace huma {

void StandardLibrary::loadCore(VM& vm) {
    vm.defineNative("print", [](VM&, int count, Value* args) -> Value {
        for (int i = 0; i < count; ++i) {
            std::cout << args[i].toString() << (i == count - 1 ? "\n" : " ");
        }
        return Value::makeNull();
    });

    vm.defineNative("length", [](VM&, int count, Value* args) -> Value {
        if (count < 1) return Value::makeNumber(0, true);
        if (args[0].type == ValueType::VAL_STRING && args[0].as_string) {
            return Value::makeNumber(args[0].as_string->size(), true);
        }
        if (args[0].type == ValueType::VAL_LIST && args[0].as_list) {
            return Value::makeNumber(args[0].as_list->size(), true);
        }
        if (args[0].type == ValueType::VAL_DICT && args[0].as_dict) {
            return Value::makeNumber(args[0].as_dict->size(), true);
        }
        return Value::makeNumber(0, true);
    });

    vm.defineNative("type_of", [](VM&, int count, Value* args) -> Value {
        if (count < 1) return Value::makeString("nothing");
        switch (args[0].type) {
            case ValueType::VAL_NULL: return Value::makeString("nothing");
            case ValueType::VAL_BOOL: return Value::makeString("boolean");
            case ValueType::VAL_NUMBER: return Value::makeString(args[0].is_integer ? "integer" : "decimal");
            case ValueType::VAL_STRING: return Value::makeString("text");
            case ValueType::VAL_LIST: return Value::makeString("list");
            case ValueType::VAL_DICT: return Value::makeString("dictionary");
            case ValueType::VAL_FUNCTION:
            case ValueType::VAL_NATIVE_FN: return Value::makeString("function");
            case ValueType::VAL_CLASS: return Value::makeString("class");
            case ValueType::VAL_INSTANCE: return Value::makeString("instance");
            default: return Value::makeString("unknown");
        }
    });

    vm.defineNative("to_string", [](VM&, int count, Value* args) -> Value {
        if (count < 1) return Value::makeString("");
        return Value::makeString(args[0].toString());
    });

    vm.defineNative("to_number", [](VM&, int count, Value* args) -> Value {
        if (count < 1) return Value::makeNumber(0, true);
        if (args[0].type == ValueType::VAL_NUMBER) return args[0];
        try {
            double n = std::stod(args[0].toString());
            bool is_int = (std::floor(n) == n);
            return Value::makeNumber(n, is_int);
        } catch (...) {
            return Value::makeNumber(0, true);
        }
    });

    vm.defineNative("range", [](VM&, int count, Value* args) -> Value {
        int start = 0;
        int end = 0;
        int step = 1;
        if (count == 1) {
            end = static_cast<int>(args[0].as_number);
        } else if (count >= 2) {
            start = static_cast<int>(args[0].as_number);
            end = static_cast<int>(args[1].as_number);
            if (count >= 3) step = static_cast<int>(args[2].as_number);
        }
        std::vector<Value> list;
        if (step > 0) {
            for (int i = start; i < end; i += step) {
                list.push_back(Value::makeNumber(i, true));
            }
        }
        return Value::makeList(list);
    });

    vm.defineNative("assert", [](VM& vm_inst, int count, Value* args) -> Value {
        if (count < 1 || !args[0].isTruthy()) {
            std::string msg = (count >= 2) ? args[1].toString() : "Assertion failed.";
            vm_inst.runtimeError(msg);
        }
        return Value::makeBool(true);
    });
}

void StandardLibrary::loadMath(VM& vm) {
    vm.defineGlobal("PI", Value::makeNumber(3.14159265358979323846, false));
    vm.defineGlobal("E", Value::makeNumber(2.71828182845904523536, false));

    vm.defineNative("sqrt", [](VM&, int count, Value* args) -> Value {
        if (count < 1 || args[0].type != ValueType::VAL_NUMBER) return Value::makeNumber(0);
        return Value::makeNumber(std::sqrt(args[0].as_number));
    });

    vm.defineNative("pow", [](VM&, int count, Value* args) -> Value {
        if (count < 2) return Value::makeNumber(0);
        return Value::makeNumber(std::pow(args[0].as_number, args[1].as_number));
    });

    vm.defineNative("abs", [](VM&, int count, Value* args) -> Value {
        if (count < 1) return Value::makeNumber(0);
        return Value::makeNumber(std::abs(args[0].as_number), args[0].is_integer);
    });

    vm.defineNative("sin", [](VM&, int count, Value* args) -> Value {
        if (count < 1) return Value::makeNumber(0);
        return Value::makeNumber(std::sin(args[0].as_number));
    });

    vm.defineNative("cos", [](VM&, int count, Value* args) -> Value {
        if (count < 1) return Value::makeNumber(0);
        return Value::makeNumber(std::cos(args[0].as_number));
    });

    vm.defineNative("tan", [](VM&, int count, Value* args) -> Value {
        if (count < 1) return Value::makeNumber(0);
        return Value::makeNumber(std::tan(args[0].as_number));
    });

    vm.defineNative("floor", [](VM&, int count, Value* args) -> Value {
        if (count < 1) return Value::makeNumber(0);
        return Value::makeNumber(std::floor(args[0].as_number), true);
    });

    vm.defineNative("ceil", [](VM&, int count, Value* args) -> Value {
        if (count < 1) return Value::makeNumber(0);
        return Value::makeNumber(std::ceil(args[0].as_number), true);
    });

    vm.defineNative("round", [](VM&, int count, Value* args) -> Value {
        if (count < 1) return Value::makeNumber(0);
        return Value::makeNumber(std::round(args[0].as_number), true);
    });

    vm.defineNative("min", [](VM&, int count, Value* args) -> Value {
        if (count < 2) return (count == 1) ? args[0] : Value::makeNumber(0);
        return Value::makeNumber(std::min(args[0].as_number, args[1].as_number));
    });

    vm.defineNative("max", [](VM&, int count, Value* args) -> Value {
        if (count < 2) return (count == 1) ? args[0] : Value::makeNumber(0);
        return Value::makeNumber(std::max(args[0].as_number, args[1].as_number));
    });

    vm.defineNative("random", [](VM&, int, Value*) -> Value {
        static std::mt19937_64 rng(std::random_device{}());
        static std::uniform_real_distribution<double> dist(0.0, 1.0);
        return Value::makeNumber(dist(rng));
    });
}

void StandardLibrary::loadText(VM& vm) {
    vm.defineNative("to_upper", [](VM&, int count, Value* args) -> Value {
        if (count < 1) return Value::makeString("");
        std::string s = args[0].toString();
        for (char& c : s) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
        return Value::makeString(s);
    });

    vm.defineNative("to_lower", [](VM&, int count, Value* args) -> Value {
        if (count < 1) return Value::makeString("");
        std::string s = args[0].toString();
        for (char& c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        return Value::makeString(s);
    });

    vm.defineNative("trim", [](VM&, int count, Value* args) -> Value {
        if (count < 1) return Value::makeString("");
        std::string s = args[0].toString();
        auto start = std::find_if_not(s.begin(), s.end(), [](unsigned char c) { return std::isspace(c); });
        auto end = std::find_if_not(s.rbegin(), s.rend(), [](unsigned char c) { return std::isspace(c); }).base();
        return Value::makeString((start < end) ? std::string(start, end) : std::string());
    });

    vm.defineNative("starts_with", [](VM&, int count, Value* args) -> Value {
        if (count < 2) return Value::makeBool(false);
        std::string s = args[0].toString();
        std::string prefix = args[1].toString();
        bool res = (s.rfind(prefix, 0) == 0);
        return Value::makeBool(res);
    });

    vm.defineNative("ends_with", [](VM&, int count, Value* args) -> Value {
        if (count < 2) return Value::makeBool(false);
        std::string s = args[0].toString();
        std::string suffix = args[1].toString();
        if (s.length() < suffix.length()) return Value::makeBool(false);
        bool res = (s.compare(s.length() - suffix.length(), suffix.length(), suffix) == 0);
        return Value::makeBool(res);
    });

    vm.defineNative("contains", [](VM&, int count, Value* args) -> Value {
        if (count < 2) return Value::makeBool(false);
        if (args[0].type == ValueType::VAL_STRING) {
            std::string s = args[0].toString();
            std::string sub = args[1].toString();
            return Value::makeBool(s.find(sub) != std::string::npos);
        }
        if (args[0].type == ValueType::VAL_LIST && args[0].as_list) {
            for (const auto& item : *args[0].as_list) {
                if (item == args[1]) return Value::makeBool(true);
            }
            return Value::makeBool(false);
        }
        if (args[0].type == ValueType::VAL_DICT && args[0].as_dict) {
            return Value::makeBool(args[0].as_dict->find(args[1].toString()) != args[0].as_dict->end());
        }
        return Value::makeBool(false);
    });

    vm.defineNative("split", [](VM&, int count, Value* args) -> Value {
        if (count < 1) return Value::makeList();
        std::string s = args[0].toString();
        std::string delim = (count >= 2) ? args[1].toString() : " ";
        std::vector<Value> tokens;
        size_t start = 0;
        size_t end = s.find(delim);
        while (end != std::string::npos) {
            tokens.push_back(Value::makeString(s.substr(start, end - start)));
            start = end + delim.length();
            end = s.find(delim, start);
        }
        tokens.push_back(Value::makeString(s.substr(start)));
        return Value::makeList(tokens);
    });

    vm.defineNative("join", [](VM&, int count, Value* args) -> Value {
        if (count < 1 || args[0].type != ValueType::VAL_LIST || !args[0].as_list) {
            return Value::makeString("");
        }
        std::string delim = (count >= 2) ? args[1].toString() : ", ";
        std::ostringstream ss;
        for (size_t i = 0; i < args[0].as_list->size(); ++i) {
            if (i > 0) ss << delim;
            ss << (*args[0].as_list)[i].toString();
        }
        return Value::makeString(ss.str());
    });
}

void StandardLibrary::loadCollections(VM& vm) {
    vm.defineNative("push", [](VM&, int count, Value* args) -> Value {
        if (count >= 2 && args[0].type == ValueType::VAL_LIST && args[0].as_list) {
            args[0].as_list->push_back(args[1]);
            return Value::makeNumber(args[0].as_list->size(), true);
        }
        return Value::makeNull();
    });

    vm.defineNative("pop", [](VM&, int count, Value* args) -> Value {
        if (count >= 1 && args[0].type == ValueType::VAL_LIST && args[0].as_list && !args[0].as_list->empty()) {
            Value last = args[0].as_list->back();
            args[0].as_list->pop_back();
            return last;
        }
        return Value::makeNull();
    });

    vm.defineNative("keys", [](VM&, int count, Value* args) -> Value {
        if (count >= 1 && args[0].type == ValueType::VAL_DICT && args[0].as_dict) {
            std::vector<Value> klist;
            for (const auto& pair : *args[0].as_dict) {
                klist.push_back(Value::makeString(pair.first));
            }
            return Value::makeList(klist);
        }
        return Value::makeList();
    });

    vm.defineNative("values", [](VM&, int count, Value* args) -> Value {
        if (count >= 1 && args[0].type == ValueType::VAL_DICT && args[0].as_dict) {
            std::vector<Value> vlist;
            for (const auto& pair : *args[0].as_dict) {
                vlist.push_back(pair.second);
            }
            return Value::makeList(vlist);
        }
        return Value::makeList();
    });
}

void StandardLibrary::loadFiles(VM& vm) {
    vm.defineNative("read_file", [](VM&, int count, Value* args) -> Value {
        if (count < 1) return Value::makeNull();
        std::string filename = args[0].toString();
        std::ifstream file(filename);
        if (!file.is_open()) return Value::makeNull();
        std::stringstream buffer;
        buffer << file.rdbuf();
        return Value::makeString(buffer.str());
    });

    vm.defineNative("write_file", [](VM&, int count, Value* args) -> Value {
        if (count < 2) return Value::makeBool(false);
        std::string filename = args[0].toString();
        std::string content = args[1].toString();
        std::ofstream file(filename);
        if (!file.is_open()) return Value::makeBool(false);
        file << content;
        return Value::makeBool(true);
    });

    vm.defineNative("file_exists", [](VM&, int count, Value* args) -> Value {
        if (count < 1) return Value::makeBool(false);
        std::ifstream file(args[0].toString());
        return Value::makeBool(file.good());
    });
}

void StandardLibrary::loadSystem(VM& vm) {
    vm.defineNative("platform", [](VM&, int, Value*) -> Value {
#if defined(_WIN32)
        return Value::makeString("windows");
#elif defined(__APPLE__)
        return Value::makeString("macos");
#elif defined(__linux__)
        return Value::makeString("linux");
#else
        return Value::makeString("unknown");
#endif
    });

    vm.defineNative("clock", [](VM&, int, Value*) -> Value {
        auto now = std::chrono::steady_clock::now().time_since_epoch();
        double secs = std::chrono::duration<double>(now).count();
        return Value::makeNumber(secs);
    });
}

void StandardLibrary::loadTime(VM& vm) {
    vm.defineNative("now", [](VM&, int, Value*) -> Value {
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        return Value::makeNumber(static_cast<double>(ms), true);
    });

    vm.defineNative("sleep", [](VM&, int count, Value* args) -> Value {
        if (count >= 1 && args[0].type == ValueType::VAL_NUMBER) {
            int ms = static_cast<int>(args[0].as_number);
            std::this_thread::sleep_for(std::chrono::milliseconds(ms));
        }
        return Value::makeNull();
    });
}

void StandardLibrary::loadJson(VM& vm) {
    vm.defineNative("json_stringify", [](VM&, int count, Value* args) -> Value {
        if (count < 1) return Value::makeString("null");
        return Value::makeString(args[0].toString());
    });
}

void StandardLibrary::loadCrypto(VM& vm) {
    vm.defineNative("base64_encode", [](VM&, int count, Value* args) -> Value {
        if (count < 1) return Value::makeString("");
        static const char b64_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        const std::string in = args[0].toString();
        std::string out;
        int val = 0, valb = -6;
        for (unsigned char c : in) {
            val = (val << 8) + c;
            valb += 8;
            while (valb >= 0) {
                out.push_back(b64_table[(val >> valb) & 0x3F]);
                valb -= 6;
            }
        }
        if (valb > -6) out.push_back(b64_table[((val << 8) >> (valb + 8)) & 0x3F]);
        while (out.size() % 4) out.push_back('=');
        return Value::makeString(out);
    });
}

void StandardLibrary::loadTesting(VM& vm) {
    vm.defineNative("assert_equal", [](VM& vm_inst, int count, Value* args) -> Value {
        if (count < 2) return Value::makeBool(false);
        if (!(args[0] == args[1])) {
            std::string msg = "Assertion Failed: expected '" + args[1].toString() + "', but got '" + args[0].toString() + "'";
            if (count >= 3) msg += " (" + args[2].toString() + ")";
            vm_inst.runtimeError(msg);
            return Value::makeBool(false);
        }
        return Value::makeBool(true);
    });

    vm.defineNative("assert_true", [](VM& vm_inst, int count, Value* args) -> Value {
        if (count < 1 || !args[0].isTruthy()) {
            std::string msg = "Assertion Failed: expected true";
            if (count >= 2) msg += " (" + args[1].toString() + ")";
            vm_inst.runtimeError(msg);
            return Value::makeBool(false);
        }
        return Value::makeBool(true);
    });
}

void StandardLibrary::loadAll(VM& vm) {
    loadCore(vm);
    loadMath(vm);
    loadText(vm);
    loadCollections(vm);
    loadFiles(vm);
    loadSystem(vm);
    loadTime(vm);
    loadJson(vm);
    loadCrypto(vm);
    loadTesting(vm);
}

} // namespace huma
