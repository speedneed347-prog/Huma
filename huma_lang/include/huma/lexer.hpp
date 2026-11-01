#pragma once

#include "huma/common.hpp"
#include <string>
#include <vector>

namespace huma {

class Lexer {
public:
    Lexer(std::string source, std::string filename = "main.huma", DiagnosticEngine* diag = nullptr);

    std::vector<Token> tokenize();

private:
    char peek() const;
    char peekNext() const;
    char advance();
    bool match(char expected);
    bool isAtEnd() const;

    void skipWhitespaceAndComments();
    Token scanToken();
    Token stringLiteral();
    Token numberLiteral();
    Token identifierOrKeyword();

    std::string source_;
    std::string filename_;
    DiagnosticEngine* diag_;
    size_t start_{0};
    size_t current_{0};
    int line_{1};
    int column_{1};
    int token_start_col_{1};
};

} // namespace huma
