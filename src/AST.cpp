#include "AST.h"

namespace Scarlet {

// LiteralExpression
void LiteralExpression::accept(ASTVisitor& visitor) {
    visitor.visitLiteralExpression(this);
}

// VariableExpression
void VariableExpression::accept(ASTVisitor& visitor) {
    visitor.visitVariableExpression(this);
}

// BinaryExpression
void BinaryExpression::accept(ASTVisitor& visitor) {
    visitor.visitBinaryExpression(this);
}

// UnaryExpression
void UnaryExpression::accept(ASTVisitor& visitor) {
    visitor.visitUnaryExpression(this);
}

// AssignmentExpression
void AssignmentExpression::accept(ASTVisitor& visitor) {
    visitor.visitAssignmentExpression(this);
}

// FunctionCallExpression
void FunctionCallExpression::accept(ASTVisitor& visitor) {
    visitor.visitFunctionCallExpression(this);
}

// BlockStatement
void BlockStatement::accept(ASTVisitor& visitor) {
    visitor.visitBlockStatement(this);
}

// VariableDeclaration
void VariableDeclaration::accept(ASTVisitor& visitor) {
    visitor.visitVariableDeclaration(this);
}

// FunctionDeclaration
void FunctionDeclaration::accept(ASTVisitor& visitor) {
    visitor.visitFunctionDeclaration(this);
}

// IfStatement
void IfStatement::accept(ASTVisitor& visitor) {
    visitor.visitIfStatement(this);
}

// WhileStatement
void WhileStatement::accept(ASTVisitor& visitor) {
    visitor.visitWhileStatement(this);
}

// ReturnStatement
void ReturnStatement::accept(ASTVisitor& visitor) {
    visitor.visitReturnStatement(this);
}

// ExpressionStatement
void ExpressionStatement::accept(ASTVisitor& visitor) {
    visitor.visitExpressionStatement(this);
}

// Program
void Program::accept(ASTVisitor& visitor) {
    visitor.visitProgram(this);
}

} // namespace Scarlet 