#pragma once

#include "Common.h"
#include "Lexer.h"
#include "AST.h"
#include <vector>

namespace Scarlet {

class Parser {
public:
    explicit Parser(const TokenList& tokens);
    
    // Main parsing function
    ASTNodePtr parse();
    
    // Expression parsing (following operator precedence)
    ExpressionPtr parseExpression();
    ExpressionPtr parseAssignment();
    ExpressionPtr parseLogicalOr();
    ExpressionPtr parseLogicalAnd();
    ExpressionPtr parseEquality();
    ExpressionPtr parseComparison();
    ExpressionPtr parseTerm();
    ExpressionPtr parseFactor();
    ExpressionPtr parseUnary();
    ExpressionPtr parsePrimary();
    
    // Statement parsing
    StatementPtr parseStatement();
    StatementPtr parseExpressionStatement();
    StatementPtr parseBlockStatement();
    StatementPtr parseIfStatement();
    StatementPtr parseWhileStatement();
    StatementPtr parseReturnStatement();
    StatementPtr parseVariableDeclaration();
    StatementPtr parseFunctionDeclaration();
    
    // Helper methods
    bool match(TokenType type);
    bool check(TokenType type) const;
    Token* advance();
    Token* peek() const;
    Token* previous() const;
    bool isAtEnd() const;
    
    // Error handling
    void synchronize();
    void error(const String& message);
    Token* consume(TokenType type, const String& message);
    
private:
    TokenList tokens_;
    size_t current_;
    
    // Error recovery
    bool hadError_;
    bool panicMode_;
    
    // Helper for parsing function parameters
    std::vector<std::pair<String, DataType>> parseParameters();
    
    // Helper for parsing type annotations
    DataType parseType();
    
    // Helper for parsing argument lists
    std::vector<ExpressionPtr> parseArguments();
};

} // namespace Scarlet 