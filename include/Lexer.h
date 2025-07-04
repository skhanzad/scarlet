#pragma once

#include "Common.h"
#include "Utils.h"
#include <unordered_map>

namespace Scarlet {

class Token {
public:
    Token(TokenType type, const String& value, const SourceLocation& location)
        : type_(type), value_(value), location_(location) {}
    
    TokenType type() const { return type_; }
    const String& value() const { return value_; }
    const SourceLocation& location() const { return location_; }
    
    String toString() const {
        return fmt::format("Token({}, '{}', {})", 
            tokenTypeToString(type_), value_, location_.toString());
    }
    
private:
    TokenType type_;
    String value_;
    SourceLocation location_;
};

class Lexer {
public:
    explicit Lexer(const String& source);
    
    // Main lexing function
    TokenList tokenize();
    
    // Individual token parsing
    std::unique_ptr<Token> nextToken();
    
    // Helper methods
    void skipWhitespace();
    void skipComment();
    std::unique_ptr<Token> readNumber();
    std::unique_ptr<Token> readString();
    std::unique_ptr<Token> readIdentifier();
    std::unique_ptr<Token> readOperator();
    
    // Character utilities
    char peek(size_t offset = 0) const;
    char advance();
    bool match(char expected);
    bool isAtEnd() const;
    
    // Error handling
    std::unique_ptr<Token> errorToken(const String& message);
    
private:
    String source_;
    size_t current_;
    size_t start_;
    SourceLocation location_;
    
    // Keyword mapping
    static const std::unordered_map<String, TokenType> keywords_;
    
    // Initialize keywords
    static std::unordered_map<String, TokenType> initializeKeywords();
};

} // namespace Scarlet 