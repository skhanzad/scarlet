#pragma once

#include "Common.h"
#include "AST.h"
#include <unordered_map>
#include <stack>
#include <vector>

namespace Scarlet {

// Symbol information
struct Symbol {
    String name;
    DataType type;
    bool isFunction;
    bool isConstant;
    SourceLocation location;
    
    // For functions
    std::vector<DataType> parameterTypes;
    DataType returnType;
    
    Symbol(const String& name, DataType type, bool isFunction = false, 
           const SourceLocation& location = SourceLocation())
        : name(name), type(type), isFunction(isFunction), isConstant(false), location(location) {}
};

// Symbol table with scoping
class SymbolTable {
public:
    SymbolTable();
    
    // Scope management
    void enterScope();
    void exitScope();
    
    // Symbol management
    bool insert(const String& name, const Symbol& symbol);
    Symbol* lookup(const String& name);
    Symbol* lookupCurrentScope(const String& name);
    
    // Utility
    size_t scopeDepth() const { return scopes_.size(); }
    void clear();
    
private:
    std::vector<std::unordered_map<String, Symbol>> scopes_;
};

// Type checking and validation
class TypeChecker {
public:
    TypeChecker();
    
    // Type checking methods
    DataType checkExpression(Expression* expr);
    void checkStatement(Statement* stmt);
    void checkProgram(Program* program);
    
    // Type compatibility
    bool isCompatible(DataType from, DataType to);
    DataType getResultType(OperatorType op, DataType left, DataType right);
    DataType getUnaryResultType(OperatorType op, DataType operand);
    
    // Error reporting
    void reportError(const String& message, const SourceLocation& location);
    bool hasErrors() const { return !errors_.empty(); }
    const std::vector<String>& getErrors() const { return errors_; }
    
private:
    SymbolTable symbolTable_;
    std::vector<String> errors_;
    DataType currentReturnType_;
    bool inFunction_;
    
    // Helper methods
    void checkBinaryExpression(BinaryExpression* expr);
    void checkUnaryExpression(UnaryExpression* expr);
    void checkFunctionCall(FunctionCallExpression* expr);
    void checkVariableDeclaration(VariableDeclaration* stmt);
    void checkFunctionDeclaration(FunctionDeclaration* stmt);
    void checkIfStatement(IfStatement* stmt);
    void checkWhileStatement(WhileStatement* stmt);
    void checkReturnStatement(ReturnStatement* stmt);
};

// Semantic analyzer - main orchestrator
class SemanticAnalyzer : public ASTVisitor {
public:
    SemanticAnalyzer();
    
    // Main analysis function
    bool analyze(Program* program);
    
    // Visitor implementations
    void visitLiteralExpression(LiteralExpression* expr) override;
    void visitVariableExpression(VariableExpression* expr) override;
    void visitBinaryExpression(BinaryExpression* expr) override;
    void visitUnaryExpression(UnaryExpression* expr) override;
    void visitAssignmentExpression(AssignmentExpression* expr) override;
    void visitFunctionCallExpression(FunctionCallExpression* expr) override;
    
    void visitBlockStatement(BlockStatement* stmt) override;
    void visitVariableDeclaration(VariableDeclaration* stmt) override;
    void visitFunctionDeclaration(FunctionDeclaration* stmt) override;
    void visitIfStatement(IfStatement* stmt) override;
    void visitWhileStatement(WhileStatement* stmt) override;
    void visitReturnStatement(ReturnStatement* stmt) override;
    void visitExpressionStatement(ExpressionStatement* stmt) override;
    
    void visitProgram(Program* program) override;
    
    // Results
    bool hasErrors() const { return typeChecker_.hasErrors(); }
    const std::vector<String>& getErrors() const { return typeChecker_.getErrors(); }
    
private:
    TypeChecker typeChecker_;
    SymbolTable symbolTable_;
    
    // Analysis state
    DataType currentExpressionType_;
    bool inLoop_;
    bool inFunction_;
    DataType currentFunctionReturnType_;
};

// Built-in function declarations
class BuiltinFunctions {
public:
    static void registerBuiltins(SymbolTable& symbolTable);
    
private:
    static void registerPrintFunction(SymbolTable& symbolTable);
    static void registerInputFunction(SymbolTable& symbolTable);
    static void registerMathFunctions(SymbolTable& symbolTable);
};

} // namespace Scarlet 