#include "huma/lexer.hpp"
#include <cctype>
#include <unordered_map>

namespace huma {

static const std::unordered_map<std::string, TokenType> kKeywords = {
    {"make", TokenType::MAKE},
    {"change", TokenType::CHANGE},
    {"to", TokenType::TO},
    {"fixed", TokenType::FIXED},
    {"show", TokenType::SHOW},
    {"print", TokenType::SHOW},
    {"if", TokenType::IF},
    {"then", TokenType::THEN},
    {"otherwise", TokenType::OTHERWISE},
    {"else", TokenType::ELSE},
    {"repeat", TokenType::REPEAT},
    {"times", TokenType::TIMES},
    {"while", TokenType::WHILE},
    {"for", TokenType::FOR},
    {"each", TokenType::EACH},
    {"in", TokenType::IN},
    {"create", TokenType::CREATE},
    {"function", TokenType::FUNCTION},
    {"using", TokenType::USING},
    {"give", TokenType::GIVE},
    {"back", TokenType::BACK},
    {"call", TokenType::CALL},
    {"class", TokenType::CLASS},
    {"struct", TokenType::STRUCT},
    {"import", TokenType::IMPORT},
    {"export", TokenType::EXPORT},
    {"try", TokenType::TRY},
    {"catch", TokenType::CATCH},
    {"end", TokenType::END},
    {"break", TokenType::BREAK},
    {"continue", TokenType::CONTINUE},
    {"return", TokenType::RETURN},
    {"match", TokenType::MATCH},
    {"case", TokenType::CASE},
    {"as", TokenType::AS},

    {"is", TokenType::IS},
    {"greater", TokenType::GREATER},
    {"less", TokenType::LESS},
    {"than", TokenType::THAN},
    {"equal", TokenType::EQUAL},
    {"not", TokenType::NOT},
    {"and", TokenType::AND},
    {"or", TokenType::OR},
    {"yes", TokenType::YES},
    {"true", TokenType::YES},
    {"no", TokenType::NO},
    {"false", TokenType::NO},
    {"nothing", TokenType::NOTHING},
    {"null", TokenType::NOTHING}
};

Lexer::Lexer(std::string source, std::string filename, DiagnosticEngine* diag)
    : source_(std::move(source)), filename_(std::move(filename)), diag_(diag) {}

char Lexer::peek() const {
    if (isAtEnd()) return '\0';
    return source_[current_];
}

char Lexer::peekNext() const {
    if (current_ + 1 >= source_.size()) return '\0';
    return source_[current_ + 1];
}

char Lexer::advance() {
    char c = source_[current_++];
    column_++;
    return c;
}

bool Lexer::match(char expected) {
    if (isAtEnd()) return false;
    if (source_[current_] != expected) return false;
    current_++;
    column_++;
    return true;
}

bool Lexer::isAtEnd() const {
    return current_ >= source_.size();
}

void Lexer::skipWhitespaceAndComments() {
    while (!isAtEnd()) {
        char c = peek();
        if (c == ' ' || c == '\t' || c == '\r') {
            advance();
        } else if (c == '\n') {
            line_++;
            column_ = 1;
            advance();
        } else if (c == '#' || (c == '/' && peekNext() == '/')) {
            // Comment until newline
            while (peek() != '\n' && !isAtEnd()) {
                advance();
            }
        } else {
            break;
        }
    }
}

Token Lexer::scanToken() {
    skipWhitespaceAndComments();

    if (isAtEnd()) {
        return Token{TokenType::END_OF_FILE, "", {filename_, line_, column_}, 0.0, false};
    }

    start_ = current_;
    token_start_col_ = column_;
    char c = advance();

    if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') {
        return identifierOrKeyword();
    }

    if (std::isdigit(static_cast<unsigned char>(c))) {
        return numberLiteral();
    }

    SourceLocation loc{filename_, line_, token_start_col_};

    switch (c) {
        case '(': return Token{TokenType::LPAREN, "(", loc};
        case ')': return Token{TokenType::RPAREN, ")", loc};
        case '[': return Token{TokenType::LBRACKET, "[", loc};
        case ']': return Token{TokenType::RBRACKET, "]", loc};
        case '{': return Token{TokenType::LBRACE, "{", loc};
        case '}': return Token{TokenType::RBRACE, "}", loc};
        case ',': return Token{TokenType::COMMA, ",", loc};
        case ':': return Token{TokenType::COLON, ":", loc};
        case '.': return Token{TokenType::DOT, ".", loc};
        case '+': return Token{TokenType::PLUS, "+", loc};
        case '-': return Token{TokenType::MINUS, "-", loc};
        case '*': return Token{TokenType::STAR, "*", loc};
        case '/': return Token{TokenType::SLASH, "/", loc};
        case '%': return Token{TokenType::PERCENT, "%", loc};
        case '^': return Token{TokenType::CARET, "^", loc};
        case '=':
            if (match('=')) return Token{TokenType::EQ_EQ, "==", loc};
            return Token{TokenType::EQUAL_SIGN, "=", loc};
        case '!':
            if (match('=')) return Token{TokenType::BANG_EQ, "!=", loc};
            return Token{TokenType::NOT, "!", loc};
        case '<':
            if (match('=')) return Token{TokenType::LT_EQ, "<=", loc};
            return Token{TokenType::LT, "<", loc};
        case '>':
            if (match('=')) return Token{TokenType::GT_EQ, ">=", loc};
            return Token{TokenType::GT, ">", loc};
        case '"':
        case '\'':
            return stringLiteral();
        default:
            if (diag_) {
                diag_->error(std::string("Unexpected character: '") + c + "'", loc);
            }
            return Token{TokenType::UNKNOWN, std::string(1, c), loc};
    }
}

Token Lexer::stringLiteral() {
    char quote_type = source_[start_];
    std::string val;
    SourceLocation loc{filename_, line_, token_start_col_};

    while (!isAtEnd() && peek() != quote_type) {
        if (peek() == '\n') {
            line_++;
            column_ = 1;
        }
        if (peek() == '\\') {
            advance(); // Consume '\'
            if (!isAtEnd()) {
                char esc = advance();
                switch (esc) {
                    case 'n': val += '\n'; break;
                    case 't': val += '\t'; break;
                    case 'r': val += '\r'; break;
                    case '\\': val += '\\'; break;
                    case '"': val += '"'; break;
                    case '\'': val += '\''; break;
                    default: val += esc; break;
                }
            }
        } else {
            val += advance();
        }
    }

    if (isAtEnd()) {
        if (diag_) {
            diag_->error("Unterminated string literal.", loc, "Add closing quotation mark.");
        }
        return Token{TokenType::STRING, val, loc};
    }

    advance(); // Consume closing quote
    Token tok{TokenType::STRING, val, loc};
    return tok;
}

Token Lexer::numberLiteral() {
    SourceLocation loc{filename_, line_, token_start_col_};
    bool is_float = false;

    while (std::isdigit(static_cast<unsigned char>(peek()))) {
        advance();
    }

    if (peek() == '.' && std::isdigit(static_cast<unsigned char>(peekNext()))) {
        is_float = true;
        advance(); // consume '.'
        while (std::isdigit(static_cast<unsigned char>(peek()))) {
            advance();
        }
    }

    std::string num_str = source_.substr(start_, current_ - start_);
    double val = std::stod(num_str);
    Token tok{TokenType::NUMBER, num_str, loc, val, !is_float};
    return tok;
}

Token Lexer::identifierOrKeyword() {
    SourceLocation loc{filename_, line_, token_start_col_};
    while (std::isalnum(static_cast<unsigned char>(peek())) || peek() == '_') {
        advance();
    }

    std::string text = source_.substr(start_, current_ - start_);
    auto it = kKeywords.find(text);
    if (it != kKeywords.end()) {
        return Token{it->second, text, loc};
    }
    return Token{TokenType::IDENTIFIER, text, loc};
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    while (true) {
        Token tok = scanToken();
        tokens.push_back(tok);
        if (tok.type == TokenType::END_OF_FILE) break;
    }
    return tokens;
}

} // namespace huma
