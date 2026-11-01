#pragma once

#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cstdint>
#include <map>
#include <variant>

namespace huma {

struct SourceLocation {
    std::string filename;
    int line{1};
    int column{1};

    std::string toString() const {
        return filename + ":" + std::to_string(line) + ":" + std::to_string(column);
    }
};

enum class TokenType {
    // Keywords
    MAKE,
    CHANGE,
    TO,
    FIXED,
    SHOW,
    IF,
    THEN,
    OTHERWISE,
    ELSE,
    REPEAT,
    TIMES,
    WHILE,
    FOR,
    EACH,
    IN,
    CREATE,
    FUNCTION,
    USING,
    GIVE,
    BACK,
    CALL,
    CLASS,
    STRUCT,
    IMPORT,
    EXPORT,
    TRY,
    CATCH,
    END,
    BREAK,
    CONTINUE,
    RETURN,
    MATCH,
    CASE,
    AS,

    // English-like operators & Booleans
    IS,
    GREATER,
    LESS,
    THAN,
    EQUAL,
    NOT,
    AND,
    OR,
    YES,
    NO,
    NOTHING,

    // Symbols & Standard Operators
    PLUS,
    MINUS,
    STAR,
    SLASH,
    PERCENT,
    CARET,
    EQUAL_SIGN,
    EQ_EQ,
    BANG_EQ,
    LT,
    GT,
    LT_EQ,
    GT_EQ,
    COMMA,
    COLON,
    DOT,
    LPAREN,
    RPAREN,
    LBRACKET,
    RBRACKET,
    LBRACE,
    RBRACE,

    // Literals & Identifiers
    IDENTIFIER,
    NUMBER,
    STRING,

    // Special
    NEWLINE,
    END_OF_FILE,
    UNKNOWN
};

struct Token {
    TokenType type;
    std::string text;
    SourceLocation loc;
    double number_val{0.0};
    bool is_integer{false};

    std::string typeName() const;
};

enum class DiagnosticLevel {
    INFO,
    WARNING,
    ERROR
};

struct Diagnostic {
    DiagnosticLevel level;
    std::string message;
    SourceLocation loc;
    std::string suggestion;
    std::string source_line;

    std::string toString() const {
        std::ostringstream ss;
        switch (level) {
            case DiagnosticLevel::ERROR: ss << "\033[1;31mError:\033[0m\n"; break;
            case DiagnosticLevel::WARNING: ss << "\033[1;33mWarning:\033[0m\n"; break;
            case DiagnosticLevel::INFO: ss << "\033[1;36mInfo:\033[0m\n"; break;
        }
        ss << message << "\n\n";
        ss << "Location:\n" << loc.toString() << "\n";
        if (!source_line.empty()) {
            ss << "\n" << source_line << "\n";
            std::string pointer(std::max(0, loc.column - 1), ' ');
            pointer += "^";
            ss << pointer << "\n";
        }
        if (!suggestion.empty()) {
            ss << "\nSuggestion:\n" << suggestion << "\n";
        }
        return ss.str();
    }
};

class DiagnosticEngine {
public:
    void report(DiagnosticLevel level, const std::string& msg, const SourceLocation& loc,
                const std::string& suggestion = "", const std::string& line = "") {
        diagnostics_.push_back({level, msg, loc, suggestion, line});
    }

    void error(const std::string& msg, const SourceLocation& loc,
               const std::string& suggestion = "", const std::string& line = "") {
        report(DiagnosticLevel::ERROR, msg, loc, suggestion, line);
    }

    void warning(const std::string& msg, const SourceLocation& loc,
                 const std::string& suggestion = "", const std::string& line = "") {
        report(DiagnosticLevel::WARNING, msg, loc, suggestion, line);
    }

    bool hasErrors() const {
        for (const auto& d : diagnostics_) {
            if (d.level == DiagnosticLevel::ERROR) return true;
        }
        return false;
    }

    const std::vector<Diagnostic>& getDiagnostics() const {
        return diagnostics_;
    }

    void clear() { diagnostics_.clear(); }

private:
    std::vector<Diagnostic> diagnostics_;
};

// Levenshtein distance for compiler typo suggestions
int levenshteinDistance(const std::string& s1, const std::string& s2);
std::string findClosestMatch(const std::string& target, const std::vector<std::string>& candidates, int max_dist = 3);

} // namespace huma
