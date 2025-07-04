#pragma once

#include "Common.h"
#include "AST.h"
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Type.h>
#include <unordered_map>

namespace Scarlet {

// Code generation interface
class CodeGenerator {
public:
    virtual ~CodeGenerator() = default;
    virtual bool generate(Program* program) = 0;
    virtual bool writeToFile(const String& filename) = 0;
    virtual String getGeneratedCode() = 0;
};

// LLVM-based code generator
class LLVMCodeGenerator : public CodeGenerator, public ASTVisitor {
public:
    LLVMCodeGenerator();
    ~LLVMCodeGenerator() override;
    
    // CodeGenerator interface
    bool generate(Program* program) override;
    bool writeToFile(const String& filename) override;
    String getGeneratedCode() override;
    
    // ASTVisitor implementations
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
    
    // LLVM utilities
    llvm::LLVMContext& getContext() { return context_; }
    llvm::Module* getModule() { return module_.get(); }
    llvm::IRBuilder<>& getBuilder() { return builder_; }
    
private:
    // LLVM components
    llvm::LLVMContext context_;
    std::unique_ptr<llvm::Module> module_;
    llvm::IRBuilder<> builder_;
    
    // Code generation state
    llvm::Value* currentValue_;
    llvm::Function* currentFunction_;
    std::unordered_map<String, llvm::Value*> variables_;
    std::unordered_map<String, llvm::Function*> functions_;
    
    // Control flow
    std::vector<llvm::BasicBlock*> breakTargets_;
    std::vector<llvm::BasicBlock*> continueTargets_;
    
    // Helper methods
    llvm::Type* getLLVMType(DataType type);
    llvm::Value* createAlloca(llvm::Type* type, const String& name);
    void createBuiltinFunctions();
    
    // Expression helpers
    llvm::Value* generateExpression(Expression* expr);
    llvm::Value* generateBinaryOperation(llvm::Value* left, OperatorType op, llvm::Value* right);
    llvm::Value* generateUnaryOperation(OperatorType op, llvm::Value* operand);
    
    // Statement helpers
    void generateStatement(Statement* stmt);
    void generateBlock(const std::vector<StatementPtr>& statements);
    
    // Function helpers
    llvm::Function* createFunction(const String& name, DataType returnType, 
                                  const std::vector<std::pair<String, DataType>>& params);
    void generateFunctionBody(FunctionDeclaration* func);
    
    // Control flow helpers
    void generateIfStatement(IfStatement* stmt);
    void generateWhileStatement(WhileStatement* stmt);
    void generateReturnStatement(ReturnStatement* stmt);
    
    // Variable management
    void declareVariable(const String& name, llvm::Value* value);
    llvm::Value* getVariable(const String& name);
    void setVariable(const String& name, llvm::Value* value);
    
    // Error handling
    void reportError(const String& message, const SourceLocation& location);
    bool hasErrors_;
    std::vector<String> errors_;
};

// Optimization passes
class Optimizer {
public:
    static bool optimize(llvm::Module* module);
    
private:
    static bool runFunctionPasses(llvm::Function* function);
    static bool runModulePasses(llvm::Module* module);
};

// Target code generation
class TargetCodeGenerator {
public:
    TargetCodeGenerator();
    ~TargetCodeGenerator();
    
    bool generateObjectFile(llvm::Module* module, const String& filename);
    bool generateExecutable(llvm::Module* module, const String& filename);
    bool generateAssembly(llvm::Module* module, const String& filename);
    
private:
    llvm::TargetMachine* targetMachine_;
    llvm::Target* target_;
    
    bool initializeTarget();
    void setTargetOptions();
};

} // namespace Scarlet 