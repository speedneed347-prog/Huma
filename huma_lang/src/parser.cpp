#include "huma/parser.hpp"
#include <sstream>

namespace huma {

Parser::Parser(std::vector<Token> tokens, DiagnosticEngine& diag, std::string source_text)
    : tokens_(std::move(tokens)), diag_(diag), source_text_(std::move(source_text)) {}

const Token& Parser::peek() const {
    return tokens_[current_];
}

const Token& Parser::previous() const {
    return tokens_[current_ - 1];
}

bool Parser::isAtEnd() const {
    return peek().type == TokenType::END_OF_FILE;
}

Token Parser::advance() {
    if (!isAtEnd()) current_++;
    return previous();
}

bool Parser::check(TokenType type) const {
    if (isAtEnd()) return false;
    return peek().type == type;
}

bool Parser::match(TokenType type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

bool Parser::match(const std::vector<TokenType>& types) {
    for (TokenType t : types) {
        if (check(t)) {
            advance();
            return true;
        }
    }
    return false;
}

Token Parser::consume(TokenType type, const std::string& error_message, const std::string& suggestion) {
    if (check(type)) return advance();
    diag_.error(error_message, peek().loc, suggestion, getSourceLine(peek().loc.line));
    return peek();
}

std::string Parser::getSourceLine(int line) const {
    if (source_text_.empty()) return "";
    std::istringstream stream(source_text_);
    std::string line_str;
    int current_line = 1;
    while (std::getline(stream, line_str)) {
        if (current_line == line) return line_str;
        current_line++;
    }
    return "";
}

void Parser::synchronize() {
    advance();
    while (!isAtEnd()) {
        if (previous().type == TokenType::NEWLINE) return;
        switch (peek().type) {
            case TokenType::MAKE:
            case TokenType::CHANGE:
            case TokenType::FIXED:
            case TokenType::SHOW:
            case TokenType::IF:
            case TokenType::REPEAT:
            case TokenType::CREATE:
            case TokenType::GIVE:
            case TokenType::RETURN:
            case TokenType::TRY:
            case TokenType::IMPORT:
                return;
            default:
                break;
        }
        advance();
    }
}

std::unique_ptr<Program> Parser::parse() {
    auto program = std::make_unique<Program>();
    program->loc = peek().loc;

    while (!isAtEnd()) {
        try {
            auto stmt = declaration();
            if (stmt) {
                program->statements.push_back(std::move(stmt));
            }
        } catch (...) {
            synchronize();
        }
    }

    return program;
}

std::unique_ptr<Stmt> Parser::declaration() {
    if (match(TokenType::MAKE)) {
        bool is_fixed = false;
        if (match(TokenType::FIXED)) is_fixed = true;
        return makeStatement(is_fixed);
    }
    if (match(TokenType::FIXED)) {
        return makeStatement(true);
    }
    if (match(TokenType::CREATE)) {
        return createStatement();
    }
    return statement();
}

std::unique_ptr<Stmt> Parser::statement() {
    if (match(TokenType::CHANGE)) return changeStatement();
    if (match(TokenType::SHOW)) return showStatement();
    if (match(TokenType::IF)) return ifStatement();
    if (match(TokenType::REPEAT)) return repeatStatement();
    if (match(TokenType::GIVE)) {
        consume(TokenType::BACK, "Expected 'back' after 'give'", "Write 'give back <expression>'");
        return giveBackStatement();
    }
    if (match(TokenType::RETURN)) return giveBackStatement();
    if (match(TokenType::TRY)) return tryCatchStatement();
    if (match(TokenType::IMPORT)) return importStatement();
    if (match(TokenType::BREAK)) return std::make_unique<BreakStmt>(previous().loc);
    if (match(TokenType::CONTINUE)) return std::make_unique<ContinueStmt>(previous().loc);

    return expressionStatement();
}

std::unique_ptr<Stmt> Parser::makeStatement(bool is_fixed) {
    SourceLocation loc = previous().loc;
    Token name_tok = consume(TokenType::IDENTIFIER, "Expected variable name after 'make'", "Provide a variable identifier, e.g. 'make x = 10'");
    std::string type_ann = "";
    if (match(TokenType::AS)) {
        Token type_tok = advance();
        type_ann = type_tok.text;
    }
    consume(TokenType::EQUAL_SIGN, "Expected '=' in variable declaration", "Assign initial value with '='");
    auto init = expression();
    return std::make_unique<VarDeclStmt>(name_tok.text, is_fixed, std::move(init), loc, type_ann);
}

std::unique_ptr<Stmt> Parser::changeStatement() {
    SourceLocation loc = previous().loc;
    Token name_tok = consume(TokenType::IDENTIFIER, "Expected variable name after 'change'", "Specify which variable to modify, e.g. 'change age to age + 1'");
    consume(TokenType::TO, "Expected 'to' after variable name in 'change' statement", "Use 'change <var> to <value>'");
    auto val = expression();
    return std::make_unique<ChangeStmt>(name_tok.text, std::move(val), loc);
}

std::unique_ptr<Stmt> Parser::showStatement() {
    SourceLocation loc = previous().loc;
    std::vector<std::unique_ptr<Expr>> exprs;
    do {
        exprs.push_back(expression());
    } while (match(TokenType::COMMA));
    return std::make_unique<ShowStmt>(std::move(exprs), loc);
}

std::unique_ptr<Stmt> Parser::ifStatement() {
    SourceLocation loc = previous().loc;
    auto cond = expression();
    if (check(TokenType::THEN)) advance(); // optional 'then'

    std::vector<std::unique_ptr<Stmt>> then_branch;
    std::vector<ElseIfBranch> else_if_branches;
    std::vector<std::unique_ptr<Stmt>> otherwise_branch;

    while (!check(TokenType::END) && !check(TokenType::OTHERWISE) &&
           !check(TokenType::ELSE) && !isAtEnd()) {
        then_branch.push_back(declaration());
    }

    while (check(TokenType::ELSE) || check(TokenType::OTHERWISE)) {
        if (check(TokenType::ELSE)) {
            advance();
            if (match(TokenType::IF)) {
                SourceLocation elif_loc = previous().loc;
                auto elif_cond = expression();
                if (check(TokenType::THEN)) advance();
                std::vector<std::unique_ptr<Stmt>> elif_body;
                while (!check(TokenType::END) && !check(TokenType::OTHERWISE) &&
                       !check(TokenType::ELSE) && !isAtEnd()) {
                    elif_body.push_back(declaration());
                }
                else_if_branches.push_back({std::move(elif_cond), std::move(elif_body), elif_loc});
                continue;
            }
        } else {
            advance(); // consume OTHERWISE
        }

        // It's the final otherwise/else block
        while (!check(TokenType::END) && !isAtEnd()) {
            otherwise_branch.push_back(declaration());
        }
        break;
    }

    consume(TokenType::END, "Expected 'end' at close of 'if' block", "Close conditional block with 'end'");
    auto if_stmt = std::make_unique<IfStmt>(std::move(cond), std::move(then_branch), loc);
    if_stmt->else_if_branches = std::move(else_if_branches);
    if_stmt->otherwise_branch = std::move(otherwise_branch);
    return if_stmt;
}

std::unique_ptr<Stmt> Parser::repeatStatement() {
    SourceLocation loc = previous().loc;
    if (match(TokenType::WHILE)) {
        auto cond = expression();
        std::vector<std::unique_ptr<Stmt>> body;
        while (!check(TokenType::END) && !isAtEnd()) {
            body.push_back(declaration());
        }
        consume(TokenType::END, "Expected 'end' at close of 'repeat while' loop", "End loop with 'end'");
        return std::make_unique<WhileStmt>(std::move(cond), std::move(body), loc);
    }
    if (match(TokenType::FOR)) {
        consume(TokenType::EACH, "Expected 'each' after 'for'", "Write 'repeat for each <item> in <collection>'");
        Token item_tok = consume(TokenType::IDENTIFIER, "Expected loop variable name");
        consume(TokenType::IN, "Expected 'in' after loop variable name");
        auto col = expression();
        std::vector<std::unique_ptr<Stmt>> body;
        while (!check(TokenType::END) && !isAtEnd()) {
            body.push_back(declaration());
        }
        consume(TokenType::END, "Expected 'end' at close of loop");
        return std::make_unique<ForEachStmt>(item_tok.text, std::move(col), std::move(body), loc);
    }

    // repeat N times
    auto count_expr = expression();
    consume(TokenType::TIMES, "Expected 'times' in repeat statement", "Write 'repeat <count> times'");
    std::vector<std::unique_ptr<Stmt>> body;
    while (!check(TokenType::END) && !isAtEnd()) {
        body.push_back(declaration());
    }
    consume(TokenType::END, "Expected 'end' at close of 'repeat' loop", "End loop with 'end'");
    return std::make_unique<RepeatTimesStmt>(std::move(count_expr), std::move(body), loc);
}

std::unique_ptr<Stmt> Parser::createStatement() {
    SourceLocation loc = previous().loc;
    if (match(TokenType::FUNCTION)) {
        Token name_tok = consume(TokenType::IDENTIFIER, "Expected function name after 'create function'");
        std::vector<std::string> params;
        if (match(TokenType::USING) || match(TokenType::LPAREN)) {
            bool use_parens = (previous().type == TokenType::LPAREN);
            if (!check(TokenType::RPAREN) && !check(TokenType::NEWLINE)) {
                do {
                    Token param_tok = consume(TokenType::IDENTIFIER, "Expected parameter name");
                    params.push_back(param_tok.text);
                } while (match(TokenType::COMMA));
            }
            if (use_parens) {
                consume(TokenType::RPAREN, "Expected ')' after parameter list");
            }
        }
        std::vector<std::unique_ptr<Stmt>> body;
        while (!check(TokenType::END) && !isAtEnd()) {
            body.push_back(declaration());
        }
        consume(TokenType::END, "Expected 'end' to close function definition", "Add 'end' at end of function");
        return std::make_unique<FuncDeclStmt>(name_tok.text, std::move(params), std::move(body), loc);
    }
    if (match(TokenType::CLASS) || match(TokenType::STRUCT)) {
        Token name_tok = consume(TokenType::IDENTIFIER, "Expected class name after 'create class'");
        std::vector<std::string> fields;
        std::vector<std::unique_ptr<FuncDeclStmt>> methods;
        while (!check(TokenType::END) && !isAtEnd()) {
            if (match(TokenType::CREATE)) {
                if (match(TokenType::FUNCTION)) {
                    Token mname = consume(TokenType::IDENTIFIER, "Expected method name");
                    std::vector<std::string> params;
                    if (match(TokenType::USING) || match(TokenType::LPAREN)) {
                        bool parens = (previous().type == TokenType::LPAREN);
                        if (!check(TokenType::RPAREN) && !check(TokenType::NEWLINE)) {
                            do {
                                params.push_back(consume(TokenType::IDENTIFIER, "Expected parameter").text);
                            } while (match(TokenType::COMMA));
                        }
                        if (parens) consume(TokenType::RPAREN, "Expected ')'");
                    }
                    std::vector<std::unique_ptr<Stmt>> mbody;
                    while (!check(TokenType::END) && !isAtEnd()) {
                        mbody.push_back(declaration());
                    }
                    consume(TokenType::END, "Expected 'end' for method");
                    methods.push_back(std::make_unique<FuncDeclStmt>(mname.text, params, std::move(mbody), loc));
                    continue;
                }
            } else if (match(TokenType::MAKE)) {
                Token fname = consume(TokenType::IDENTIFIER, "Expected field name");
                fields.push_back(fname.text);
                if (match(TokenType::EQUAL_SIGN)) {
                    expression(); // default init
                }
                continue;
            }
            advance();
        }
        consume(TokenType::END, "Expected 'end' at close of class");
        return std::make_unique<ClassDeclStmt>(name_tok.text, fields, std::move(methods), loc);
    }
    diag_.error("Expected 'function' or 'class' after 'create'", loc);
    return nullptr;
}

std::unique_ptr<Stmt> Parser::giveBackStatement() {
    SourceLocation loc = previous().loc;
    std::unique_ptr<Expr> val = nullptr;
    if (!check(TokenType::END) && !check(TokenType::OTHERWISE) && !isAtEnd()) {
        val = expression();
    }
    return std::make_unique<ReturnStmt>(std::move(val), loc);
}

std::unique_ptr<Stmt> Parser::tryCatchStatement() {
    SourceLocation loc = previous().loc;
    std::vector<std::unique_ptr<Stmt>> try_block;
    while (!check(TokenType::CATCH) && !check(TokenType::END) && !isAtEnd()) {
        try_block.push_back(declaration());
    }
    consume(TokenType::CATCH, "Expected 'catch' in try-catch block");
    Token err_tok = consume(TokenType::IDENTIFIER, "Expected error variable name after 'catch'");
    std::vector<std::unique_ptr<Stmt>> catch_block;
    while (!check(TokenType::END) && !isAtEnd()) {
        catch_block.push_back(declaration());
    }
    consume(TokenType::END, "Expected 'end' to close try-catch block");
    return std::make_unique<TryCatchStmt>(std::move(try_block), err_tok.text, std::move(catch_block), loc);
}

std::unique_ptr<Stmt> Parser::importStatement() {
    SourceLocation loc = previous().loc;
    std::string mod_name;
    if (check(TokenType::STRING)) {
        mod_name = advance().text;
    } else {
        Token t = consume(TokenType::IDENTIFIER, "Expected module name to import");
        mod_name = t.text;
        while (match(TokenType::DOT)) {
            Token sub = consume(TokenType::IDENTIFIER, "Expected identifier after '.' in import");
            mod_name += "." + sub.text;
        }
    }
    return std::make_unique<ImportStmt>(mod_name, loc);
}

std::unique_ptr<Stmt> Parser::expressionStatement() {
    SourceLocation loc = peek().loc;
    auto expr = expression();
    return std::make_unique<ExprStmt>(std::move(expr), loc);
}

// Expressions
std::unique_ptr<Expr> Parser::expression() {
    return logicOr();
}

std::unique_ptr<Expr> Parser::logicOr() {
    auto expr = logicAnd();
    while (match(TokenType::OR)) {
        SourceLocation loc = previous().loc;
        auto right = logicAnd();
        expr = std::make_unique<BinaryExpr>(BinaryOp::OR, std::move(expr), std::move(right), loc);
    }
    return expr;
}

std::unique_ptr<Expr> Parser::logicAnd() {
    auto expr = equality();
    while (match(TokenType::AND)) {
        SourceLocation loc = previous().loc;
        auto right = equality();
        expr = std::make_unique<BinaryExpr>(BinaryOp::AND, std::move(expr), std::move(right), loc);
    }
    return expr;
}

std::unique_ptr<Expr> Parser::equality() {
    auto expr = comparison();
    while (true) {
        if (match(TokenType::EQ_EQ)) {
            SourceLocation loc = previous().loc;
            auto right = comparison();
            expr = std::make_unique<BinaryExpr>(BinaryOp::EQUAL, std::move(expr), std::move(right), loc);
        } else if (match(TokenType::BANG_EQ)) {
            SourceLocation loc = previous().loc;
            auto right = comparison();
            expr = std::make_unique<BinaryExpr>(BinaryOp::NOT_EQUAL, std::move(expr), std::move(right), loc);
        } else {
            break;
        }
    }
    return expr;
}

std::unique_ptr<Expr> Parser::comparison() {
    auto expr = term();

    while (true) {
        // Support English-like "is greater than", "is less than", "is equal to", "is not equal to"
        if (match(TokenType::IS)) {
            SourceLocation loc = previous().loc;
            bool is_not = match(TokenType::NOT);
            if (match(TokenType::GREATER)) {
                consume(TokenType::THAN, "Expected 'than' after 'greater'");
                bool or_equal = false;
                if (match(TokenType::OR)) {
                    consume(TokenType::EQUAL, "Expected 'equal' after 'or'");
                    if (check(TokenType::TO)) advance();
                    or_equal = true;
                }
                auto right = term();
                BinaryOp op = is_not ? (or_equal ? BinaryOp::LESS : BinaryOp::LESS_EQUAL)
                                     : (or_equal ? BinaryOp::GREATER_EQUAL : BinaryOp::GREATER);
                expr = std::make_unique<BinaryExpr>(op, std::move(expr), std::move(right), loc);
            } else if (match(TokenType::LESS)) {
                consume(TokenType::THAN, "Expected 'than' after 'less'");
                bool or_equal = false;
                if (match(TokenType::OR)) {
                    consume(TokenType::EQUAL, "Expected 'equal' after 'or'");
                    if (check(TokenType::TO)) advance();
                    or_equal = true;
                }
                auto right = term();
                BinaryOp op = is_not ? (or_equal ? BinaryOp::GREATER : BinaryOp::GREATER_EQUAL)
                                     : (or_equal ? BinaryOp::LESS_EQUAL : BinaryOp::LESS);
                expr = std::make_unique<BinaryExpr>(op, std::move(expr), std::move(right), loc);
            } else if (match(TokenType::EQUAL)) {
                if (check(TokenType::TO)) advance();
                auto right = term();
                BinaryOp op = is_not ? BinaryOp::NOT_EQUAL : BinaryOp::EQUAL;
                expr = std::make_unique<BinaryExpr>(op, std::move(expr), std::move(right), loc);
            } else {
                // "x is y" -> equality, "x is not y" -> inequality
                auto right = term();
                BinaryOp op = is_not ? BinaryOp::NOT_EQUAL : BinaryOp::EQUAL;
                expr = std::make_unique<BinaryExpr>(op, std::move(expr), std::move(right), loc);
            }
        } else if (match(TokenType::GT)) {
            SourceLocation loc = previous().loc;
            auto right = term();
            expr = std::make_unique<BinaryExpr>(BinaryOp::GREATER, std::move(expr), std::move(right), loc);
        } else if (match(TokenType::LT)) {
            SourceLocation loc = previous().loc;
            auto right = term();
            expr = std::make_unique<BinaryExpr>(BinaryOp::LESS, std::move(expr), std::move(right), loc);
        } else if (match(TokenType::GT_EQ)) {
            SourceLocation loc = previous().loc;
            auto right = term();
            expr = std::make_unique<BinaryExpr>(BinaryOp::GREATER_EQUAL, std::move(expr), std::move(right), loc);
        } else if (match(TokenType::LT_EQ)) {
            SourceLocation loc = previous().loc;
            auto right = term();
            expr = std::make_unique<BinaryExpr>(BinaryOp::LESS_EQUAL, std::move(expr), std::move(right), loc);
        } else {
            break;
        }
    }
    return expr;
}

std::unique_ptr<Expr> Parser::term() {
    auto expr = factor();
    while (match(TokenType::PLUS) || match(TokenType::MINUS)) {
        Token op_tok = previous();
        BinaryOp op = (op_tok.type == TokenType::PLUS) ? BinaryOp::ADD : BinaryOp::SUB;
        auto right = factor();
        expr = std::make_unique<BinaryExpr>(op, std::move(expr), std::move(right), op_tok.loc);
    }
    return expr;
}

std::unique_ptr<Expr> Parser::factor() {
    auto expr = power();
    while (match(TokenType::STAR) || match(TokenType::SLASH) || match(TokenType::PERCENT)) {
        Token op_tok = previous();
        BinaryOp op = BinaryOp::MUL;
        if (op_tok.type == TokenType::SLASH) op = BinaryOp::DIV;
        else if (op_tok.type == TokenType::PERCENT) op = BinaryOp::MOD;
        auto right = power();
        expr = std::make_unique<BinaryExpr>(op, std::move(expr), std::move(right), op_tok.loc);
    }
    return expr;
}

std::unique_ptr<Expr> Parser::power() {
    auto expr = unary();
    if (match(TokenType::CARET)) {
        SourceLocation loc = previous().loc;
        auto right = power(); // Right-associative
        return std::make_unique<BinaryExpr>(BinaryOp::POW, std::move(expr), std::move(right), loc);
    }
    return expr;
}

std::unique_ptr<Expr> Parser::unary() {
    if (match(TokenType::NOT)) {
        SourceLocation loc = previous().loc;
        auto right = unary();
        return std::make_unique<UnaryExpr>(UnaryOp::NOT, std::move(right), loc);
    }
    if (match(TokenType::MINUS)) {
        SourceLocation loc = previous().loc;
        auto right = unary();
        return std::make_unique<UnaryExpr>(UnaryOp::NEGATE, std::move(right), loc);
    }
    return callOrMember();
}

std::unique_ptr<Expr> Parser::callOrMember() {
    if (match(TokenType::CALL)) {
        SourceLocation loc = previous().loc;
        Token callee_tok = consume(TokenType::IDENTIFIER, "Expected function name after 'call'");
        std::vector<std::unique_ptr<Expr>> args;
        if (match(TokenType::USING) || match(TokenType::LPAREN)) {
            bool parens = (previous().type == TokenType::LPAREN);
            if (!check(TokenType::RPAREN) && !check(TokenType::NEWLINE)) {
                do {
                    args.push_back(expression());
                } while (match(TokenType::COMMA));
            }
            if (parens) consume(TokenType::RPAREN, "Expected ')'");
        }
        return std::make_unique<CallExpr>(callee_tok.text, std::move(args), loc);
    }

    auto expr = primary();

    while (true) {
        if (match(TokenType::LPAREN)) {
            // Function call with standard syntax: foo(a, b)
            SourceLocation loc = previous().loc;
            std::vector<std::unique_ptr<Expr>> args;
            if (!check(TokenType::RPAREN)) {
                do {
                    args.push_back(expression());
                } while (match(TokenType::COMMA));
            }
            consume(TokenType::RPAREN, "Expected ')' after argument list");
            if (auto* id_expr = dynamic_cast<IdentifierExpr*>(expr.get())) {
                expr = std::make_unique<CallExpr>(id_expr->name, std::move(args), loc);
            } else {
                diag_.error("Invalid function call target", loc);
            }
        } else if (match(TokenType::LBRACKET)) {
            // Indexing: arr[idx]
            SourceLocation loc = previous().loc;
            auto index = expression();
            consume(TokenType::RBRACKET, "Expected ']' after index");
            expr = std::make_unique<IndexExpr>(std::move(expr), std::move(index), loc);
        } else if (match(TokenType::DOT)) {
            // Member access: obj.field
            SourceLocation loc = previous().loc;
            Token mem_tok = consume(TokenType::IDENTIFIER, "Expected property name after '.'");
            expr = std::make_unique<MemberExpr>(std::move(expr), mem_tok.text, loc);
        } else {
            break;
        }
    }
    return expr;
}

std::unique_ptr<Expr> Parser::primary() {
    if (match(TokenType::NUMBER)) {
        const Token& t = previous();
        return std::make_unique<LiteralExpr>(t.number_val, t.is_integer, t.loc);
    }
    if (match(TokenType::STRING)) {
        const Token& t = previous();
        return std::make_unique<LiteralExpr>(t.text, t.loc);
    }
    if (match(TokenType::YES)) {
        return std::make_unique<LiteralExpr>(true, previous().loc);
    }
    if (match(TokenType::NO)) {
        return std::make_unique<LiteralExpr>(false, previous().loc);
    }
    if (match(TokenType::NOTHING)) {
        return std::make_unique<LiteralExpr>(previous().loc);
    }
    if (match(TokenType::IDENTIFIER)) {
        return std::make_unique<IdentifierExpr>(previous().text, previous().loc);
    }

    // List: [1, 2, 3]
    if (match(TokenType::LBRACKET)) {
        SourceLocation loc = previous().loc;
        std::vector<std::unique_ptr<Expr>> elements;
        if (!check(TokenType::RBRACKET)) {
            do {
                elements.push_back(expression());
            } while (match(TokenType::COMMA));
        }
        consume(TokenType::RBRACKET, "Expected ']' at end of list");
        return std::make_unique<ListExpr>(std::move(elements), loc);
    }

    // Dictionary: {"key": value}
    if (match(TokenType::LBRACE)) {
        SourceLocation loc = previous().loc;
        std::vector<std::pair<std::string, std::unique_ptr<Expr>>> entries;
        if (!check(TokenType::RBRACE)) {
            do {
                std::string key;
                if (match(TokenType::STRING) || match(TokenType::IDENTIFIER)) {
                    key = previous().text;
                } else {
                    diag_.error("Expected dictionary key string or identifier", peek().loc);
                }
                consume(TokenType::COLON, "Expected ':' after dictionary key");
                auto val = expression();
                entries.emplace_back(key, std::move(val));
            } while (match(TokenType::COMMA));
        }
        consume(TokenType::RBRACE, "Expected '}' at end of dictionary");
        return std::make_unique<DictExpr>(std::move(entries), loc);
    }

    // Parenthesized grouping: (expr)
    if (match(TokenType::LPAREN)) {
        auto expr = expression();
        consume(TokenType::RPAREN, "Expected ')' after grouping expression");
        return expr;
    }

    diag_.error("Unexpected token: '" + peek().text + "'", peek().loc, "Check syntax at this location", getSourceLine(peek().loc.line));
    advance();
    return std::make_unique<LiteralExpr>(previous().loc);
}

} // namespace huma
