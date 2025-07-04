#include "Parser.h"
#include <stdexcept>

namespace Scarlet {

Parser::Parser(const TokenList& tokens) 
    : tokens_(std::move(tokens)), current_(0), hadError_(false), panicMode_(false) {}

ASTNodePtr Parser::parse() {
    std::vector<StatementPtr> statements;
    
    while (!isAtEnd()) {
        try {
            auto stmt = parseStatement();
            if (stmt) {
                statements.push_back(std::move(stmt));
            }
        } catch (const CompilerError& e) {
            error(e.what());
            synchronize();
        }
    }
    
    return std::make_unique<Program>(std::move(statements));
}

ExpressionPtr Parser::parseExpression() {
    return parseAssignment();
}

ExpressionPtr Parser::parseAssignment() {
    ExpressionPtr expr = parseLogicalOr();
    
    if (match(TokenType::ASSIGN)) {
        Token* equals = previous();
        ExpressionPtr value = parseAssignment();
        
        if (auto varExpr = dynamic_cast<VariableExpression*>(expr.get())) {
            return std::make_unique<AssignmentExpression>(varExpr->name(), std::move(value), equals->location());
        }
        
        error("Invalid assignment target");
    }
    
    return expr;
}

ExpressionPtr Parser::parseLogicalOr() {
    ExpressionPtr expr = parseLogicalAnd();
    
    while (match(TokenType::OR)) {
        Token* operator_ = previous();
        ExpressionPtr right = parseLogicalAnd();
        expr = std::make_unique<BinaryExpression>(std::move(expr), OperatorType::OR, std::move(right), operator_->location());
    }
    
    return expr;
}

ExpressionPtr Parser::parseLogicalAnd() {
    ExpressionPtr expr = parseEquality();
    
    while (match(TokenType::AND)) {
        Token* operator_ = previous();
        ExpressionPtr right = parseEquality();
        expr = std::make_unique<BinaryExpression>(std::move(expr), OperatorType::AND, std::move(right), operator_->location());
    }
    
    return expr;
}

ExpressionPtr Parser::parseEquality() {
    ExpressionPtr expr = parseComparison();
    
    while (match(TokenType::NOT_EQUAL) || match(TokenType::EQUAL)) {
        Token* operator_ = previous();
        ExpressionPtr right = parseComparison();
        OperatorType op = operator_->type() == TokenType::EQUAL ? OperatorType::EQUAL : OperatorType::NOT_EQUAL;
        expr = std::make_unique<BinaryExpression>(std::move(expr), op, std::move(right), operator_->location());
    }
    
    return expr;
}

ExpressionPtr Parser::parseComparison() {
    ExpressionPtr expr = parseTerm();
    
    while (match(TokenType::GREATER) || match(TokenType::GREATER_EQUAL) || 
           match(TokenType::LESS) || match(TokenType::LESS_EQUAL)) {
        Token* operator_ = previous();
        ExpressionPtr right = parseTerm();
        
        OperatorType op;
        switch (operator_->type()) {
            case TokenType::GREATER: op = OperatorType::GREATER; break;
            case TokenType::GREATER_EQUAL: op = OperatorType::GREATER_EQUAL; break;
            case TokenType::LESS: op = OperatorType::LESS; break;
            case TokenType::LESS_EQUAL: op = OperatorType::LESS_EQUAL; break;
            default: SCARLET_UNREACHABLE();
        }
        
        expr = std::make_unique<BinaryExpression>(std::move(expr), op, std::move(right), operator_->location());
    }
    
    return expr;
}

ExpressionPtr Parser::parseTerm() {
    ExpressionPtr expr = parseFactor();
    
    while (match(TokenType::MINUS) || match(TokenType::PLUS)) {
        Token* operator_ = previous();
        ExpressionPtr right = parseFactor();
        OperatorType op = operator_->type() == TokenType::PLUS ? OperatorType::ADD : OperatorType::SUBTRACT;
        expr = std::make_unique<BinaryExpression>(std::move(expr), op, std::move(right), operator_->location());
    }
    
    return expr;
}

ExpressionPtr Parser::parseFactor() {
    ExpressionPtr expr = parseUnary();
    
    while (match(TokenType::SLASH) || match(TokenType::MULTIPLY) || match(TokenType::MODULO)) {
        Token* operator_ = previous();
        ExpressionPtr right = parseUnary();
        
        OperatorType op;
        switch (operator_->type()) {
            case TokenType::MULTIPLY: op = OperatorType::MULTIPLY; break;
            case TokenType::SLASH: op = OperatorType::DIVIDE; break;
            case TokenType::MODULO: op = OperatorType::MODULO; break;
            default: SCARLET_UNREACHABLE();
        }
        
        expr = std::make_unique<BinaryExpression>(std::move(expr), op, std::move(right), operator_->location());
    }
    
    return expr;
}

ExpressionPtr Parser::parseUnary() {
    if (match(TokenType::NOT) || match(TokenType::MINUS)) {
        Token* operator_ = previous();
        ExpressionPtr right = parseUnary();
        OperatorType op = operator_->type() == TokenType::MINUS ? OperatorType::SUBTRACT : OperatorType::NOT;
        return std::make_unique<UnaryExpression>(op, std::move(right), operator_->location());
    }
    
    return parsePrimary();
}

ExpressionPtr Parser::parsePrimary() {
    if (match(TokenType::FALSE)) {
        return std::make_unique<LiteralExpression>("false", DataType::BOOL, previous()->location());
    }
    if (match(TokenType::TRUE)) {
        return std::make_unique<LiteralExpression>("true", DataType::BOOL, previous()->location());
    }
    if (match(TokenType::NULL_LITERAL)) {
        return std::make_unique<LiteralExpression>("null", DataType::UNKNOWN, previous()->location());
    }
    
    if (match(TokenType::INTEGER)) {
        return std::make_unique<LiteralExpression>(previous()->value(), DataType::INT, previous()->location());
    }
    if (match(TokenType::FLOAT)) {
        return std::make_unique<LiteralExpression>(previous()->value(), DataType::FLOAT, previous()->location());
    }
    if (match(TokenType::STRING)) {
        return std::make_unique<LiteralExpression>(previous()->value(), DataType::STRING, previous()->location());
    }
    
    if (match(TokenType::IDENTIFIER)) {
        return std::make_unique<VariableExpression>(previous()->value(), previous()->location());
    }
    
    if (match(TokenType::LEFT_PAREN)) {
        ExpressionPtr expr = parseExpression();
        consume(TokenType::RIGHT_PAREN, "Expect ')' after expression.");
        return expr;
    }
    
    throw CompilerError("Expect expression.", peek()->location());
}

StatementPtr Parser::parseStatement() {
    if (match(TokenType::IF)) return parseIfStatement();
    if (match(TokenType::WHILE)) return parseWhileStatement();
    if (match(TokenType::RETURN)) return parseReturnStatement();
    if (match(TokenType::LEFT_BRACE)) return parseBlockStatement();
    if (match(TokenType::VAR) || match(TokenType::LET) || match(TokenType::CONST)) return parseVariableDeclaration();
    if (match(TokenType::FUNCTION)) return parseFunctionDeclaration();
    
    return parseExpressionStatement();
}

StatementPtr Parser::parseExpressionStatement() {
    ExpressionPtr expr = parseExpression();
    consume(TokenType::SEMICOLON, "Expect ';' after expression.");
    return std::make_unique<ExpressionStatement>(std::move(expr), previous()->location());
}

StatementPtr Parser::parseBlockStatement() {
    std::vector<StatementPtr> statements;
    
    while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
        statements.push_back(parseStatement());
    }
    
    consume(TokenType::RIGHT_BRACE, "Expect '}' after block.");
    return std::make_unique<BlockStatement>(std::move(statements), previous()->location());
}

StatementPtr Parser::parseIfStatement() {
    consume(TokenType::LEFT_PAREN, "Expect '(' after 'if'.");
    ExpressionPtr condition = parseExpression();
    consume(TokenType::RIGHT_PAREN, "Expect ')' after if condition.");
    
    StatementPtr thenBranch = parseStatement();
    StatementPtr elseBranch = nullptr;
    
    if (match(TokenType::ELSE)) {
        elseBranch = parseStatement();
    }
    
    return std::make_unique<IfStatement>(std::move(condition), std::move(thenBranch), std::move(elseBranch), previous()->location());
}

StatementPtr Parser::parseWhileStatement() {
    consume(TokenType::LEFT_PAREN, "Expect '(' after 'while'.");
    ExpressionPtr condition = parseExpression();
    consume(TokenType::RIGHT_PAREN, "Expect ')' after condition.");
    
    StatementPtr body = parseStatement();
    
    return std::make_unique<WhileStatement>(std::move(condition), std::move(body), previous()->location());
}

StatementPtr Parser::parseReturnStatement() {
    Token* keyword = previous();
    ExpressionPtr value = nullptr;
    
    if (!check(TokenType::SEMICOLON)) {
        value = parseExpression();
    }
    
    consume(TokenType::SEMICOLON, "Expect ';' after return value.");
    return std::make_unique<ReturnStatement>(std::move(value), keyword->location());
}

StatementPtr Parser::parseVariableDeclaration() {
    Token* keyword = previous();
    consume(TokenType::IDENTIFIER, "Expect variable name.");
    Token* name = previous();
    
    DataType type = DataType::UNKNOWN;
    if (match(TokenType::COLON)) {
        type = parseType();
    }
    
    ExpressionPtr initializer = nullptr;
    if (match(TokenType::ASSIGN)) {
        initializer = parseExpression();
    }
    
    consume(TokenType::SEMICOLON, "Expect ';' after variable declaration.");
    return std::make_unique<VariableDeclaration>(name->value(), type, std::move(initializer), keyword->location());
}

StatementPtr Parser::parseFunctionDeclaration() {
    consume(TokenType::IDENTIFIER, "Expect function name.");
    Token* name = previous();
    
    consume(TokenType::LEFT_PAREN, "Expect '(' after function name.");
    auto parameters = parseParameters();
    consume(TokenType::RIGHT_PAREN, "Expect ')' after parameters.");
    
    DataType returnType = DataType::VOID;
    if (match(TokenType::COLON)) {
        returnType = parseType();
    }
    
    consume(TokenType::LEFT_BRACE, "Expect '{' before function body.");
    StatementPtr body = parseBlockStatement();
    
    return std::make_unique<FunctionDeclaration>(name->value(), returnType, std::move(parameters), std::move(body), name->location());
}

std::vector<std::pair<String, DataType>> Parser::parseParameters() {
    std::vector<std::pair<String, DataType>> parameters;
    
    if (!check(TokenType::RIGHT_PAREN)) {
        do {
            consume(TokenType::IDENTIFIER, "Expect parameter name.");
            Token* name = previous();
            
            consume(TokenType::COLON, "Expect ':' after parameter name.");
            DataType type = parseType();
            
            parameters.emplace_back(name->value(), type);
        } while (match(TokenType::COMMA));
    }
    
    return parameters;
}

DataType Parser::parseType() {
    if (match(TokenType::IDENTIFIER)) {
        String typeName = previous()->value();
        if (typeName == "int") return DataType::INT;
        if (typeName == "float") return DataType::FLOAT;
        if (typeName == "bool") return DataType::BOOL;
        if (typeName == "string") return DataType::STRING;
        if (typeName == "void") return DataType::VOID;
        
        error("Unknown type: " + typeName);
        return DataType::UNKNOWN;
    }
    
    error("Expect type name.");
    return DataType::UNKNOWN;
}

std::vector<ExpressionPtr> Parser::parseArguments() {
    std::vector<ExpressionPtr> arguments;
    
    if (!check(TokenType::RIGHT_PAREN)) {
        do {
            arguments.push_back(parseExpression());
        } while (match(TokenType::COMMA));
    }
    
    return arguments;
}

bool Parser::match(TokenType type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

bool Parser::check(TokenType type) const {
    if (isAtEnd()) return false;
    return peek()->type() == type;
}

Token* Parser::advance() {
    if (!isAtEnd()) current_++;
    return previous();
}

Token* Parser::peek() const {
    return tokens_[current_].get();
}

Token* Parser::previous() const {
    return tokens_[current_ - 1].get();
}

bool Parser::isAtEnd() const {
    return peek()->type() == TokenType::END_OF_FILE;
}

void Parser::synchronize() {
    panicMode_ = false;
    
    while (!isAtEnd()) {
        if (previous()->type() == TokenType::SEMICOLON) return;
        
        switch (peek()->type()) {
            case TokenType::FUNCTION:
            case TokenType::VAR:
            case TokenType::LET:
            case TokenType::CONST:
            case TokenType::FOR:
            case TokenType::IF:
            case TokenType::WHILE:
            case TokenType::RETURN:
                return;
            default:
                break;
        }
        
        advance();
    }
}

void Parser::error(const String& message) {
    hadError_ = true;
    panicMode_ = true;
    
    if (!isAtEnd()) {
        throw CompilerError(message, peek()->location());
    } else {
        throw CompilerError(message, previous()->location());
    }
}

Token* Parser::consume(TokenType type, const String& message) {
    if (check(type)) return advance();
    
    throw CompilerError(message, peek()->location());
}

} // namespace Scarlet 