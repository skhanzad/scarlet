#pragma once

#include "Common.h"
#include "Utils.h"
#include <vector>

namespace Scarlet {

// Forward declarations
class ASTVisitor;

// Base AST node class
class ASTNode {
public:
    virtual ~ASTNode() = default;
    virtual void accept(ASTVisitor& visitor) = 0;
    virtual SourceLocation location() const = 0;
};

// Expression base class
class Expression : public ASTNode {
public:
    virtual ~Expression() = default;
    virtual DataType type() const = 0;
};

// Statement base class
class Statement : public ASTNode {
public:
    virtual ~Statement() = default;
};

// Literal expressions
class LiteralExpression : public Expression {
public:
    LiteralExpression(const String& value, DataType type, const SourceLocation& location)
        : value_(value), type_(type), location_(location) {}
    
    const String& value() const { return value_; }
    DataType type() const override { return type_; }
    SourceLocation location() const override { return location_; }
    void accept(ASTVisitor& visitor) override;
    
private:
    String value_;
    DataType type_;
    SourceLocation location_;
};

// Variable expressions
class VariableExpression : public Expression {
public:
    VariableExpression(const String& name, const SourceLocation& location)
        : name_(name), location_(location) {}
    
    const String& name() const { return name_; }
    DataType type() const override { return DataType::UNKNOWN; } // Will be resolved during semantic analysis
    SourceLocation location() const override { return location_; }
    void accept(ASTVisitor& visitor) override;
    
private:
    String name_;
    SourceLocation location_;
};

// Binary expressions
class BinaryExpression : public Expression {
public:
    BinaryExpression(ExpressionPtr left, OperatorType op, ExpressionPtr right, const SourceLocation& location)
        : left_(std::move(left)), op_(op), right_(std::move(right)), location_(location) {}
    
    Expression* left() const { return left_.get(); }
    Expression* right() const { return right_.get(); }
    OperatorType op() const { return op_; }
    DataType type() const override { return DataType::UNKNOWN; } // Will be resolved during semantic analysis
    SourceLocation location() const override { return location_; }
    void accept(ASTVisitor& visitor) override;
    
private:
    ExpressionPtr left_;
    OperatorType op_;
    ExpressionPtr right_;
    SourceLocation location_;
};

// Unary expressions
class UnaryExpression : public Expression {
public:
    UnaryExpression(OperatorType op, ExpressionPtr operand, const SourceLocation& location)
        : op_(op), operand_(std::move(operand)), location_(location) {}
    
    OperatorType op() const { return op_; }
    Expression* operand() const { return operand_.get(); }
    DataType type() const override { return DataType::UNKNOWN; } // Will be resolved during semantic analysis
    SourceLocation location() const override { return location_; }
    void accept(ASTVisitor& visitor) override;
    
private:
    OperatorType op_;
    ExpressionPtr operand_;
    SourceLocation location_;
};

// Assignment expressions
class AssignmentExpression : public Expression {
public:
    AssignmentExpression(const String& name, ExpressionPtr value, const SourceLocation& location)
        : name_(name), value_(std::move(value)), location_(location) {}
    
    const String& name() const { return name_; }
    Expression* value() const { return value_.get(); }
    DataType type() const override { return DataType::UNKNOWN; } // Will be resolved during semantic analysis
    SourceLocation location() const override { return location_; }
    void accept(ASTVisitor& visitor) override;
    
private:
    String name_;
    ExpressionPtr value_;
    SourceLocation location_;
};

// Function call expressions
class FunctionCallExpression : public Expression {
public:
    FunctionCallExpression(const String& name, std::vector<ExpressionPtr> arguments, const SourceLocation& location)
        : name_(name), arguments_(std::move(arguments)), location_(location) {}
    
    const String& name() const { return name_; }
    const std::vector<ExpressionPtr>& arguments() const { return arguments_; }
    DataType type() const override { return DataType::UNKNOWN; } // Will be resolved during semantic analysis
    SourceLocation location() const override { return location_; }
    void accept(ASTVisitor& visitor) override;
    
private:
    String name_;
    std::vector<ExpressionPtr> arguments_;
    SourceLocation location_;
};

// Block statements
class BlockStatement : public Statement {
public:
    explicit BlockStatement(std::vector<StatementPtr> statements, const SourceLocation& location)
        : statements_(std::move(statements)), location_(location) {}
    
    const std::vector<StatementPtr>& statements() const { return statements_; }
    SourceLocation location() const override { return location_; }
    void accept(ASTVisitor& visitor) override;
    
private:
    std::vector<StatementPtr> statements_;
    SourceLocation location_;
};

// Variable declarations
class VariableDeclaration : public Statement {
public:
    VariableDeclaration(const String& name, DataType type, ExpressionPtr initializer, const SourceLocation& location)
        : name_(name), type_(type), initializer_(std::move(initializer)), location_(location) {}
    
    const String& name() const { return name_; }
    DataType type() const { return type_; }
    Expression* initializer() const { return initializer_.get(); }
    SourceLocation location() const override { return location_; }
    void accept(ASTVisitor& visitor) override;
    
private:
    String name_;
    DataType type_;
    ExpressionPtr initializer_;
    SourceLocation location_;
};

// Function declarations
class FunctionDeclaration : public Statement {
public:
    FunctionDeclaration(const String& name, DataType returnType, 
                       std::vector<std::pair<String, DataType>> parameters,
                       StatementPtr body, const SourceLocation& location)
        : name_(name), returnType_(returnType), parameters_(std::move(parameters)),
          body_(std::move(body)), location_(location) {}
    
    const String& name() const { return name_; }
    DataType returnType() const { return returnType_; }
    const std::vector<std::pair<String, DataType>>& parameters() const { return parameters_; }
    Statement* body() const { return body_.get(); }
    SourceLocation location() const override { return location_; }
    void accept(ASTVisitor& visitor) override;
    
private:
    String name_;
    DataType returnType_;
    std::vector<std::pair<String, DataType>> parameters_;
    StatementPtr body_;
    SourceLocation location_;
};

// If statements
class IfStatement : public Statement {
public:
    IfStatement(ExpressionPtr condition, StatementPtr thenBranch, 
                StatementPtr elseBranch, const SourceLocation& location)
        : condition_(std::move(condition)), thenBranch_(std::move(thenBranch)),
          elseBranch_(std::move(elseBranch)), location_(location) {}
    
    Expression* condition() const { return condition_.get(); }
    Statement* thenBranch() const { return thenBranch_.get(); }
    Statement* elseBranch() const { return elseBranch_.get(); }
    SourceLocation location() const override { return location_; }
    void accept(ASTVisitor& visitor) override;
    
private:
    ExpressionPtr condition_;
    StatementPtr thenBranch_;
    StatementPtr elseBranch_;
    SourceLocation location_;
};

// While statements
class WhileStatement : public Statement {
public:
    WhileStatement(ExpressionPtr condition, StatementPtr body, const SourceLocation& location)
        : condition_(std::move(condition)), body_(std::move(body)), location_(location) {}
    
    Expression* condition() const { return condition_.get(); }
    Statement* body() const { return body_.get(); }
    SourceLocation location() const override { return location_; }
    void accept(ASTVisitor& visitor) override;
    
private:
    ExpressionPtr condition_;
    StatementPtr body_;
    SourceLocation location_;
};

// Return statements
class ReturnStatement : public Statement {
public:
    ReturnStatement(ExpressionPtr value, const SourceLocation& location)
        : value_(std::move(value)), location_(location) {}
    
    Expression* value() const { return value_.get(); }
    SourceLocation location() const override { return location_; }
    void accept(ASTVisitor& visitor) override;
    
private:
    ExpressionPtr value_;
    SourceLocation location_;
};

// Expression statements
class ExpressionStatement : public Statement {
public:
    ExpressionStatement(ExpressionPtr expression, const SourceLocation& location)
        : expression_(std::move(expression)), location_(location) {}
    
    Expression* expression() const { return expression_.get(); }
    SourceLocation location() const override { return location_; }
    void accept(ASTVisitor& visitor) override;
    
private:
    ExpressionPtr expression_;
    SourceLocation location_;
};

// Program root
class Program : public ASTNode {
public:
    explicit Program(std::vector<StatementPtr> statements)
        : statements_(std::move(statements)) {}
    
    const std::vector<StatementPtr>& statements() const { return statements_; }
    SourceLocation location() const override { return SourceLocation(); } // Root has no location
    void accept(ASTVisitor& visitor) override;
    
private:
    std::vector<StatementPtr> statements_;
};

// Visitor pattern for AST traversal
class ASTVisitor {
public:
    virtual ~ASTVisitor() = default;
    
    // Expression visitors
    virtual void visitLiteralExpression(LiteralExpression* expr) = 0;
    virtual void visitVariableExpression(VariableExpression* expr) = 0;
    virtual void visitBinaryExpression(BinaryExpression* expr) = 0;
    virtual void visitUnaryExpression(UnaryExpression* expr) = 0;
    virtual void visitAssignmentExpression(AssignmentExpression* expr) = 0;
    virtual void visitFunctionCallExpression(FunctionCallExpression* expr) = 0;
    
    // Statement visitors
    virtual void visitBlockStatement(BlockStatement* stmt) = 0;
    virtual void visitVariableDeclaration(VariableDeclaration* stmt) = 0;
    virtual void visitFunctionDeclaration(FunctionDeclaration* stmt) = 0;
    virtual void visitIfStatement(IfStatement* stmt) = 0;
    virtual void visitWhileStatement(WhileStatement* stmt) = 0;
    virtual void visitReturnStatement(ReturnStatement* stmt) = 0;
    virtual void visitExpressionStatement(ExpressionStatement* stmt) = 0;
    
    // Program visitor
    virtual void visitProgram(Program* program) = 0;
};

} // namespace Scarlet 