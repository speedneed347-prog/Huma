#pragma once

#include "huma/common.hpp"
#include "huma/ast.hpp"
#include <vector>
#include <memory>

namespace huma {

class Parser {
public:
    Parser(std::vector<Token> tokens, DiagnosticEngine& diag, std::string source_text = "");

    std::unique_ptr<Program> parse();

private:
    const Token& peek() const;
    const Token& previous() const;
    bool isAtEnd() const;
    Token advance();
    bool check(TokenType type) const;
    bool match(TokenType type);
    bool match(const std::vector<TokenType>& types);
    Token consume(TokenType type, const std::string& error_message, const std::string& suggestion = "");

    void synchronize();
    std::string getSourceLine(int line) const;

    // Statements
    std::unique_ptr<Stmt> declaration();
    std::unique_ptr<Stmt> statement();
    std::unique_ptr<Stmt> makeStatement(bool is_fixed);
    std::unique_ptr<Stmt> changeStatement();
    std::unique_ptr<Stmt> showStatement();
    std::unique_ptr<Stmt> ifStatement();
    std::unique_ptr<Stmt> repeatStatement();
    std::unique_ptr<Stmt> createStatement();
    std::unique_ptr<Stmt> giveBackStatement();
    std::unique_ptr<Stmt> tryCatchStatement();
    std::unique_ptr<Stmt> importStatement();
    std::unique_ptr<Stmt> expressionStatement();
    std::vector<std::unique_ptr<Stmt>> blockUntilEnd(const std::vector<TokenType>& terminators = {TokenType::END});

    // Expressions
    std::unique_ptr<Expr> expression();
    std::unique_ptr<Expr> logicOr();
    std::unique_ptr<Expr> logicAnd();
    std::unique_ptr<Expr> equality();
    std::unique_ptr<Expr> comparison();
    std::unique_ptr<Expr> term();
    std::unique_ptr<Expr> factor();
    std::unique_ptr<Expr> power();
    std::unique_ptr<Expr> unary();
    std::unique_ptr<Expr> callOrMember();
    std::unique_ptr<Expr> primary();

    std::vector<Token> tokens_;
    DiagnosticEngine& diag_;
    std::string source_text_;
    size_t current_{0};
};

} // namespace huma
