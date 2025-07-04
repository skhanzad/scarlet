#include "Lexer.h"
#include <cctype>

namespace Scarlet {

// Initialize keyword mapping
std::unordered_map<String, TokenType> Lexer::initializeKeywords() {
    std::unordered_map<String, TokenType> keywords;
    keywords["if"] = TokenType::IF;
    keywords["else"] = TokenType::ELSE;
    keywords["while"] = TokenType::WHILE;
    keywords["for"] = TokenType::FOR;
    keywords["return"] = TokenType::RETURN;
    keywords["function"] = TokenType::FUNCTION;
    keywords["var"] = TokenType::VAR;
    keywords["let"] = TokenType::LET;
    keywords["const"] = TokenType::CONST;
    keywords["true"] = TokenType::TRUE;
    keywords["false"] = TokenType::FALSE;
    keywords["null"] = TokenType::NULL_LITERAL;
    return keywords;
}

const std::unordered_map<String, TokenType> Lexer::keywords_ = Lexer::initializeKeywords();

Lexer::Lexer(const String& source) 
    : source_(source), current_(0), start_(0), location_(1, 1, 0) {}

TokenList Lexer::tokenize() {
    TokenList tokens;
    
    while (!isAtEnd()) {
        start_ = current_;
        auto token = nextToken();
        if (token) {
            tokens.push_back(std::move(token));
        }
    }
    
    // Add EOF token
    tokens.push_back(std::make_unique<Token>(TokenType::END_OF_FILE, "", location_));
    
    return tokens;
}

std::unique_ptr<Token> Lexer::nextToken() {
    skipWhitespace();
    
    if (isAtEnd()) {
        return nullptr;
    }
    
    start_ = current_;
    char c = advance();
    
    // Handle comments
    if (c == '/' && match('/')) {
        skipComment();
        return nextToken();
    }
    
    // Handle identifiers and keywords
    if (isAlpha(c)) {
        return readIdentifier();
    }
    
    // Handle numbers
    if (isDigit(c)) {
        return readNumber();
    }
    
    // Handle strings
    if (c == '"') {
        return readString();
    }
    
    // Handle operators and delimiters
    return readOperator();
}

void Lexer::skipWhitespace() {
    while (!isAtEnd() && isWhitespace(peek())) {
        advance();
    }
}

void Lexer::skipComment() {
    while (!isAtEnd() && peek() != '\n') {
        advance();
    }
}

std::unique_ptr<Token> Lexer::readNumber() {
    bool hasDecimal = false;
    
    while (!isAtEnd() && (isDigit(peek()) || (!hasDecimal && peek() == '.'))) {
        if (peek() == '.') {
            hasDecimal = true;
        }
        advance();
    }
    
    String number = source_.substr(start_, current_ - start_);
    TokenType type = hasDecimal ? TokenType::FLOAT : TokenType::INTEGER;
    
    return std::make_unique<Token>(type, number, location_);
}

std::unique_ptr<Token> Lexer::readString() {
    while (!isAtEnd() && peek() != '"') {
        if (peek() == '\n') {
            return errorToken("Unterminated string");
        }
        advance();
    }
    
    if (isAtEnd()) {
        return errorToken("Unterminated string");
    }
    
    // Consume the closing quote
    advance();
    
    // Extract the string content (excluding quotes)
    String value = source_.substr(start_ + 1, current_ - start_ - 2);
    return std::make_unique<Token>(TokenType::STRING, value, location_);
}

std::unique_ptr<Token> Lexer::readIdentifier() {
    while (!isAtEnd() && isAlphaNumeric(peek())) {
        advance();
    }
    
    String text = source_.substr(start_, current_ - start_);
    
    // Check if it's a keyword
    auto it = keywords_.find(text);
    if (it != keywords_.end()) {
        return std::make_unique<Token>(it->second, text, location_);
    }
    
    return std::make_unique<Token>(TokenType::IDENTIFIER, text, location_);
}

std::unique_ptr<Token> Lexer::readOperator() {
    char c = source_[current_ - 1];
    
    // Handle two-character operators
    if (!isAtEnd()) {
        char next = peek();
        String twoChar = String(1, c) + next;
        
        if (twoChar == "==") {
            advance();
            return std::make_unique<Token>(TokenType::EQUAL, twoChar, location_);
        }
        if (twoChar == "!=") {
            advance();
            return std::make_unique<Token>(TokenType::NOT_EQUAL, twoChar, location_);
        }
        if (twoChar == "<=") {
            advance();
            return std::make_unique<Token>(TokenType::LESS_EQUAL, twoChar, location_);
        }
        if (twoChar == ">=") {
            advance();
            return std::make_unique<Token>(TokenType::GREATER_EQUAL, twoChar, location_);
        }
        if (twoChar == "&&") {
            advance();
            return std::make_unique<Token>(TokenType::AND, twoChar, location_);
        }
        if (twoChar == "||") {
            advance();
            return std::make_unique<Token>(TokenType::OR, twoChar, location_);
        }
    }
    
    // Handle single-character operators and delimiters
    switch (c) {
        case '(': return std::make_unique<Token>(TokenType::LEFT_PAREN, "(", location_);
        case ')': return std::make_unique<Token>(TokenType::RIGHT_PAREN, ")", location_);
        case '{': return std::make_unique<Token>(TokenType::LEFT_BRACE, "{", location_);
        case '}': return std::make_unique<Token>(TokenType::RIGHT_BRACE, "}", location_);
        case '[': return std::make_unique<Token>(TokenType::LEFT_BRACKET, "[", location_);
        case ']': return std::make_unique<Token>(TokenType::RIGHT_BRACKET, "]", location_);
        case ';': return std::make_unique<Token>(TokenType::SEMICOLON, ";", location_);
        case ',': return std::make_unique<Token>(TokenType::COMMA, ",", location_);
        case '.': return std::make_unique<Token>(TokenType::DOT, ".", location_);
        case '+': return std::make_unique<Token>(TokenType::PLUS, "+", location_);
        case '-': return std::make_unique<Token>(TokenType::MINUS, "-", location_);
        case '*': return std::make_unique<Token>(TokenType::MULTIPLY, "*", location_);
        case '/': return std::make_unique<Token>(TokenType::DIVIDE, "/", location_);
        case '%': return std::make_unique<Token>(TokenType::MODULO, "%", location_);
        case '=': return std::make_unique<Token>(TokenType::ASSIGN, "=", location_);
        case '<': return std::make_unique<Token>(TokenType::LESS, "<", location_);
        case '>': return std::make_unique<Token>(TokenType::GREATER, ">", location_);
        case '!': return std::make_unique<Token>(TokenType::NOT, "!", location_);
        default: return errorToken("Unexpected character: " + String(1, c));
    }
}

char Lexer::peek(size_t offset) const {
    if (current_ + offset >= source_.length()) {
        return '\0';
    }
    return source_[current_ + offset];
}

char Lexer::advance() {
    if (isAtEnd()) {
        return '\0';
    }
    
    char c = source_[current_++];
    location_.advance(c);
    return c;
}

bool Lexer::match(char expected) {
    if (isAtEnd()) return false;
    if (source_[current_] != expected) return false;
    
    current_++;
    location_.advance(expected);
    return true;
}

bool Lexer::isAtEnd() const {
    return current_ >= source_.length();
}

std::unique_ptr<Token> Lexer::errorToken(const String& message) {
    return std::make_unique<Token>(TokenType::ERROR, message, location_);
}

} // namespace Scarlet 