#include "huma/common.hpp"
#include <algorithm>
#include <cmath>

namespace huma {

std::string Token::typeName() const {
    switch (type) {
        case TokenType::MAKE: return "MAKE";
        case TokenType::CHANGE: return "CHANGE";
        case TokenType::TO: return "TO";
        case TokenType::FIXED: return "FIXED";
        case TokenType::SHOW: return "SHOW";
        case TokenType::IF: return "IF";
        case TokenType::THEN: return "THEN";
        case TokenType::OTHERWISE: return "OTHERWISE";
        case TokenType::ELSE: return "ELSE";
        case TokenType::REPEAT: return "REPEAT";
        case TokenType::TIMES: return "TIMES";
        case TokenType::WHILE: return "WHILE";
        case TokenType::FOR: return "FOR";
        case TokenType::EACH: return "EACH";
        case TokenType::IN: return "IN";
        case TokenType::CREATE: return "CREATE";
        case TokenType::FUNCTION: return "FUNCTION";
        case TokenType::USING: return "USING";
        case TokenType::GIVE: return "GIVE";
        case TokenType::BACK: return "BACK";
        case TokenType::CALL: return "CALL";
        case TokenType::CLASS: return "CLASS";
        case TokenType::STRUCT: return "STRUCT";
        case TokenType::IMPORT: return "IMPORT";
        case TokenType::EXPORT: return "EXPORT";
        case TokenType::TRY: return "TRY";
        case TokenType::CATCH: return "CATCH";
        case TokenType::END: return "END";
        case TokenType::BREAK: return "BREAK";
        case TokenType::CONTINUE: return "CONTINUE";
        case TokenType::RETURN: return "RETURN";
        case TokenType::MATCH: return "MATCH";
        case TokenType::CASE: return "CASE";
        case TokenType::AS: return "AS";
        case TokenType::IS: return "IS";
        case TokenType::GREATER: return "GREATER";
        case TokenType::LESS: return "LESS";
        case TokenType::THAN: return "THAN";
        case TokenType::EQUAL: return "EQUAL";
        case TokenType::NOT: return "NOT";
        case TokenType::AND: return "AND";
        case TokenType::OR: return "OR";
        case TokenType::YES: return "YES";
        case TokenType::NO: return "NO";
        case TokenType::NOTHING: return "NOTHING";
        case TokenType::PLUS: return "PLUS";
        case TokenType::MINUS: return "MINUS";
        case TokenType::STAR: return "STAR";
        case TokenType::SLASH: return "SLASH";
        case TokenType::PERCENT: return "PERCENT";
        case TokenType::CARET: return "CARET";
        case TokenType::EQUAL_SIGN: return "EQUAL_SIGN";
        case TokenType::EQ_EQ: return "EQ_EQ";
        case TokenType::BANG_EQ: return "BANG_EQ";
        case TokenType::LT: return "LT";
        case TokenType::GT: return "GT";
        case TokenType::LT_EQ: return "LT_EQ";
        case TokenType::GT_EQ: return "GT_EQ";
        case TokenType::COMMA: return "COMMA";
        case TokenType::COLON: return "COLON";
        case TokenType::DOT: return "DOT";
        case TokenType::LPAREN: return "LPAREN";
        case TokenType::RPAREN: return "RPAREN";
        case TokenType::LBRACKET: return "LBRACKET";
        case TokenType::RBRACKET: return "RBRACKET";
        case TokenType::LBRACE: return "LBRACE";
        case TokenType::RBRACE: return "RBRACE";
        case TokenType::IDENTIFIER: return "IDENTIFIER";
        case TokenType::NUMBER: return "NUMBER";
        case TokenType::STRING: return "STRING";
        case TokenType::NEWLINE: return "NEWLINE";
        case TokenType::END_OF_FILE: return "EOF";
        case TokenType::UNKNOWN: return "UNKNOWN";
        default: return "TOKEN";
    }
}

int levenshteinDistance(const std::string& s1, const std::string& s2) {
    const size_t m = s1.size();
    const size_t n = s2.size();
    std::vector<std::vector<int>> dp(m + 1, std::vector<int>(n + 1, 0));

    for (size_t i = 0; i <= m; ++i) dp[i][0] = static_cast<int>(i);
    for (size_t j = 0; j <= n; ++j) dp[0][j] = static_cast<int>(j);

    for (size_t i = 1; i <= m; ++i) {
        for (size_t j = 1; j <= n; ++j) {
            int cost = (s1[i - 1] == s2[j - 1]) ? 0 : 1;
            dp[i][j] = std::min({
                dp[i - 1][j] + 1,      // deletion
                dp[i][j - 1] + 1,      // insertion
                dp[i - 1][j - 1] + cost // substitution
            });
        }
    }
    return dp[m][n];
}

std::string findClosestMatch(const std::string& target, const std::vector<std::string>& candidates, int max_dist) {
    std::string best_match = "";
    int min_dist = max_dist + 1;

    for (const auto& candidate : candidates) {
        int dist = levenshteinDistance(target, candidate);
        if (dist < min_dist) {
            min_dist = dist;
            best_match = candidate;
        }
    }
    return best_match;
}

} // namespace huma
