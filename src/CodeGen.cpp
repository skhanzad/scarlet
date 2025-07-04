#include "CodeGen.h"
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/IR/Verifier.h>

namespace Scarlet {

// CodeGenerator interface implementations
LLVMCodeGenerator::LLVMCodeGenerator() 
    : builder_(context_), currentValue_(nullptr), currentFunction_(nullptr), hasErrors_(false) {
    module_ = std::make_unique<llvm::Module>("scarlet_module", context_);
    createBuiltinFunctions();
}

LLVMCodeGenerator::~LLVMCodeGenerator() = default;

bool LLVMCodeGenerator::generate(Program* program) {
    program->accept(*this);
    return !hasErrors_;
}

bool LLVMCodeGenerator::writeToFile(const String& filename) {
    std::error_code EC;
    llvm::raw_fd_ostream dest(filename, EC, llvm::sys::fs::OF_None);
    
    if (EC) {
        reportError("Could not open file: " + filename, SourceLocation());
        return false;
    }
    
    module_->print(dest, nullptr);
    return true;
}

String LLVMCodeGenerator::getGeneratedCode() {
    std::string result;
    llvm::raw_string_ostream stream(result);
    module_->print(stream, nullptr);
    return result;
}

// ASTVisitor implementations
void LLVMCodeGenerator::visitLiteralExpression(LiteralExpression* expr) {
    DataType type = expr->type();
    const String& value = expr->value();
    
    switch (type) {
        case DataType::INT:
            currentValue_ = llvm::ConstantInt::get(getLLVMType(type), std::stoll(value));
            break;
        case DataType::FLOAT:
            currentValue_ = llvm::ConstantFP::get(getLLVMType(type), std::stod(value));
            break;
        case DataType::BOOL:
            currentValue_ = llvm::ConstantInt::get(getLLVMType(type), value == "true" ? 1 : 0);
            break;
        case DataType::STRING:
            currentValue_ = builder_.CreateGlobalStringPtr(value);
            break;
        default:
            reportError("Unsupported literal type", expr->location());
            break;
    }
}

void LLVMCodeGenerator::visitVariableExpression(VariableExpression* expr) {
    llvm::Value* var = getVariable(expr->name());
    if (!var) {
        reportError("Undefined variable: " + expr->name(), expr->location());
        return;
    }
    currentValue_ = builder_.CreateLoad(getLLVMType(DataType::UNKNOWN), var);
}

void LLVMCodeGenerator::visitBinaryExpression(BinaryExpression* expr) {
    llvm::Value* left = generateExpression(expr->left());
    llvm::Value* right = generateExpression(expr->right());
    
    if (!left || !right) {
        reportError("Failed to generate operands for binary expression", expr->location());
        return;
    }
    
    currentValue_ = generateBinaryOperation(left, expr->op(), right);
}

void LLVMCodeGenerator::visitUnaryExpression(UnaryExpression* expr) {
    llvm::Value* operand = generateExpression(expr->operand());
    
    if (!operand) {
        reportError("Failed to generate operand for unary expression", expr->location());
        return;
    }
    
    currentValue_ = generateUnaryOperation(expr->op(), operand);
}

void LLVMCodeGenerator::visitAssignmentExpression(AssignmentExpression* expr) {
    llvm::Value* value = generateExpression(expr->value());
    
    if (!value) {
        reportError("Failed to generate value for assignment", expr->location());
        return;
    }
    
    llvm::Value* var = getVariable(expr->name());
    if (!var) {
        reportError("Undefined variable: " + expr->name(), expr->location());
        return;
    }
    
    builder_.CreateStore(value, var);
    currentValue_ = value;
}

void LLVMCodeGenerator::visitFunctionCallExpression(FunctionCallExpression* expr) {
    llvm::Function* func = module_->getFunction(expr->name());
    if (!func) {
        reportError("Undefined function: " + expr->name(), expr->location());
        return;
    }
    
    std::vector<llvm::Value*> args;
    for (const auto& arg : expr->arguments()) {
        llvm::Value* argValue = generateExpression(arg.get());
        if (!argValue) {
            reportError("Failed to generate function argument", expr->location());
            return;
        }
        args.push_back(argValue);
    }
    
    currentValue_ = builder_.CreateCall(func, args);
}

void LLVMCodeGenerator::visitBlockStatement(BlockStatement* stmt) {
    generateBlock(stmt->statements());
}

void LLVMCodeGenerator::visitVariableDeclaration(VariableDeclaration* stmt) {
    llvm::Type* varType = getLLVMType(stmt->type());
    llvm::Value* var = createAlloca(varType, stmt->name());
    
    declareVariable(stmt->name(), var);
    
    if (stmt->initializer()) {
        llvm::Value* initValue = generateExpression(stmt->initializer());
        if (initValue) {
            builder_.CreateStore(initValue, var);
        }
    }
}

void LLVMCodeGenerator::visitFunctionDeclaration(FunctionDeclaration* stmt) {
    std::vector<llvm::Type*> paramTypes;
    for (const auto& param : stmt->parameters()) {
        paramTypes.push_back(getLLVMType(param.second));
    }
    
    llvm::FunctionType* funcType = llvm::FunctionType::get(
        getLLVMType(stmt->returnType()), paramTypes, false);
    
    llvm::Function* func = llvm::Function::Create(
        funcType, llvm::Function::ExternalLinkage, stmt->name(), module_.get());
    
    functions_[stmt->name()] = func;
    
    // Create function body
    llvm::BasicBlock* entry = llvm::BasicBlock::Create(context_, "entry", func);
    builder_.SetInsertPoint(entry);
    
    // Set up parameters
    size_t i = 0;
    for (auto& arg : func->args()) {
        const auto& param = stmt->parameters()[i];
        llvm::Value* paramVar = createAlloca(getLLVMType(param.second), param.first);
        builder_.CreateStore(&arg, paramVar);
        declareVariable(param.first, paramVar);
        ++i;
    }
    
    // Generate function body
    currentFunction_ = func;
    stmt->body()->accept(*this);
    currentFunction_ = nullptr;
    
    // Add return if needed
    if (stmt->returnType() == DataType::VOID && !builder_.GetInsertBlock()->getTerminator()) {
        builder_.CreateRetVoid();
    }
    
    // Verify function
    llvm::verifyFunction(*func, &llvm::errs());
}

void LLVMCodeGenerator::visitIfStatement(IfStatement* stmt) {
    llvm::Value* condition = generateExpression(stmt->condition());
    if (!condition) {
        reportError("Failed to generate if condition", stmt->location());
        return;
    }
    
    llvm::Function* func = builder_.GetInsertBlock()->getParent();
    llvm::BasicBlock* thenBB = llvm::BasicBlock::Create(context_, "then", func);
    llvm::BasicBlock* elseBB = llvm::BasicBlock::Create(context_, "else", func);
    llvm::BasicBlock* mergeBB = llvm::BasicBlock::Create(context_, "ifcont", func);
    
    builder_.CreateCondBr(condition, thenBB, elseBB);
    
    // Generate then block
    builder_.SetInsertPoint(thenBB);
    stmt->thenBranch()->accept(*this);
    if (!builder_.GetInsertBlock()->getTerminator()) {
        builder_.CreateBr(mergeBB);
    }
    
    // Generate else block
    builder_.SetInsertPoint(elseBB);
    if (stmt->elseBranch()) {
        stmt->elseBranch()->accept(*this);
    }
    if (!builder_.GetInsertBlock()->getTerminator()) {
        builder_.CreateBr(mergeBB);
    }
    
    // Continue with merge block
    builder_.SetInsertPoint(mergeBB);
}

void LLVMCodeGenerator::visitWhileStatement(WhileStatement* stmt) {
    llvm::Function* func = builder_.GetInsertBlock()->getParent();
    llvm::BasicBlock* loopBB = llvm::BasicBlock::Create(context_, "loop", func);
    llvm::BasicBlock* bodyBB = llvm::BasicBlock::Create(context_, "while_body", func);
    llvm::BasicBlock* afterBB = llvm::BasicBlock::Create(context_, "while_cont", func);
    
    // Enter loop
    breakTargets_.push_back(afterBB);
    continueTargets_.push_back(loopBB);
    
    builder_.CreateBr(loopBB);
    
    // Loop condition
    builder_.SetInsertPoint(loopBB);
    llvm::Value* condition = generateExpression(stmt->condition());
    if (!condition) {
        reportError("Failed to generate while condition", stmt->location());
        return;
    }
    builder_.CreateCondBr(condition, bodyBB, afterBB);
    
    // Loop body
    builder_.SetInsertPoint(bodyBB);
    stmt->body()->accept(*this);
    if (!builder_.GetInsertBlock()->getTerminator()) {
        builder_.CreateBr(loopBB);
    }
    
    // Exit loop
    builder_.SetInsertPoint(afterBB);
    breakTargets_.pop_back();
    continueTargets_.pop_back();
}

void LLVMCodeGenerator::visitReturnStatement(ReturnStatement* stmt) {
    if (stmt->value()) {
        llvm::Value* returnValue = generateExpression(stmt->value());
        if (returnValue) {
            builder_.CreateRet(returnValue);
        }
    } else {
        builder_.CreateRetVoid();
    }
}

void LLVMCodeGenerator::visitExpressionStatement(ExpressionStatement* stmt) {
    generateExpression(stmt->expression());
}

void LLVMCodeGenerator::visitProgram(Program* program) {
    for (const auto& statement : program->statements()) {
        statement->accept(*this);
    }
}

// Helper methods
llvm::Type* LLVMCodeGenerator::getLLVMType(DataType type) {
    switch (type) {
        case DataType::VOID: return llvm::Type::getVoidTy(context_);
        case DataType::INT: return llvm::Type::getInt32Ty(context_);
        case DataType::FLOAT: return llvm::Type::getDoubleTy(context_);
        case DataType::BOOL: return llvm::Type::getInt1Ty(context_);
        case DataType::STRING: return llvm::Type::getInt8PtrTy(context_);
        case DataType::ARRAY: return llvm::Type::getInt8PtrTy(context_); // Simplified
        case DataType::FUNCTION: return llvm::Type::getInt8PtrTy(context_); // Simplified
        case DataType::UNKNOWN: return llvm::Type::getInt32Ty(context_); // Default to int
        default: return llvm::Type::getInt32Ty(context_);
    }
}

llvm::Value* LLVMCodeGenerator::createAlloca(llvm::Type* type, const String& name) {
    llvm::Function* func = builder_.GetInsertBlock()->getParent();
    llvm::IRBuilder<> tmpBuilder(&func->getEntryBlock(), func->getEntryBlock().begin());
    return tmpBuilder.CreateAlloca(type, nullptr, name);
}

void LLVMCodeGenerator::createBuiltinFunctions() {
    // Create printf function declaration
    std::vector<llvm::Type*> printfArgs = {llvm::Type::getInt8PtrTy(context_)};
    llvm::FunctionType* printfType = llvm::FunctionType::get(
        llvm::Type::getInt32Ty(context_), printfArgs, true);
    llvm::Function::Create(printfType, llvm::Function::ExternalLinkage, "printf", module_.get());
}

llvm::Value* LLVMCodeGenerator::generateExpression(Expression* expr) {
    expr->accept(*this);
    return currentValue_;
}

llvm::Value* LLVMCodeGenerator::generateBinaryOperation(llvm::Value* left, OperatorType op, llvm::Value* right) {
    switch (op) {
        case OperatorType::ADD:
            if (left->getType()->isIntegerTy() && right->getType()->isIntegerTy()) {
                return builder_.CreateAdd(left, right);
            } else {
                return builder_.CreateFAdd(left, right);
            }
        case OperatorType::SUBTRACT:
            if (left->getType()->isIntegerTy() && right->getType()->isIntegerTy()) {
                return builder_.CreateSub(left, right);
            } else {
                return builder_.CreateFSub(left, right);
            }
        case OperatorType::MULTIPLY:
            if (left->getType()->isIntegerTy() && right->getType()->isIntegerTy()) {
                return builder_.CreateMul(left, right);
            } else {
                return builder_.CreateFMul(left, right);
            }
        case OperatorType::DIVIDE:
            if (left->getType()->isIntegerTy() && right->getType()->isIntegerTy()) {
                return builder_.CreateSDiv(left, right);
            } else {
                return builder_.CreateFDiv(left, right);
            }
        case OperatorType::EQUAL:
            if (left->getType()->isIntegerTy() && right->getType()->isIntegerTy()) {
                return builder_.CreateICmpEQ(left, right);
            } else {
                return builder_.CreateFCmpOEQ(left, right);
            }
        case OperatorType::NOT_EQUAL:
            if (left->getType()->isIntegerTy() && right->getType()->isIntegerTy()) {
                return builder_.CreateICmpNE(left, right);
            } else {
                return builder_.CreateFCmpONE(left, right);
            }
        case OperatorType::LESS:
            if (left->getType()->isIntegerTy() && right->getType()->isIntegerTy()) {
                return builder_.CreateICmpSLT(left, right);
            } else {
                return builder_.CreateFCmpOLT(left, right);
            }
        case OperatorType::GREATER:
            if (left->getType()->isIntegerTy() && right->getType()->isIntegerTy()) {
                return builder_.CreateICmpSGT(left, right);
            } else {
                return builder_.CreateFCmpOGT(left, right);
            }
        default:
            reportError("Unsupported binary operation", SourceLocation());
            return nullptr;
    }
}

llvm::Value* LLVMCodeGenerator::generateUnaryOperation(OperatorType op, llvm::Value* operand) {
    switch (op) {
        case OperatorType::SUBTRACT:
            if (operand->getType()->isIntegerTy()) {
                return builder_.CreateNeg(operand);
            } else {
                return builder_.CreateFNeg(operand);
            }
        case OperatorType::NOT:
            return builder_.CreateNot(operand);
        default:
            reportError("Unsupported unary operation", SourceLocation());
            return nullptr;
    }
}

void LLVMCodeGenerator::generateStatement(Statement* stmt) {
    stmt->accept(*this);
}

void LLVMCodeGenerator::generateBlock(const std::vector<StatementPtr>& statements) {
    for (const auto& statement : statements) {
        statement->accept(*this);
    }
}

void LLVMCodeGenerator::declareVariable(const String& name, llvm::Value* value) {
    variables_[name] = value;
}

llvm::Value* LLVMCodeGenerator::getVariable(const String& name) {
    auto it = variables_.find(name);
    return it != variables_.end() ? it->second : nullptr;
}

void LLVMCodeGenerator::setVariable(const String& name, llvm::Value* value) {
    variables_[name] = value;
}

void LLVMCodeGenerator::reportError(const String& message, const SourceLocation& location) {
    hasErrors_ = true;
    errors_.push_back(fmt::format("{}: {}", location.toString(), message));
}

// Optimizer implementation
bool Optimizer::optimize(llvm::Module* module) {
    return runModulePasses(module);
}

bool Optimizer::runFunctionPasses(llvm::Function* function) {
    // Basic function-level optimizations
    return true;
}

bool Optimizer::runModulePasses(llvm::Module* module) {
    // Basic module-level optimizations
    return true;
}

// TargetCodeGenerator implementation
TargetCodeGenerator::TargetCodeGenerator() : targetMachine_(nullptr), target_(nullptr) {
    initializeTarget();
}

TargetCodeGenerator::~TargetCodeGenerator() = default;

bool TargetCodeGenerator::generateObjectFile(llvm::Module* module, const String& filename) {
    if (!targetMachine_) {
        return false;
    }
    
    std::error_code EC;
    llvm::raw_fd_ostream dest(filename, EC, llvm::sys::fs::OF_None);
    
    if (EC) {
        return false;
    }
    
    llvm::legacy::PassManager pass;
    if (targetMachine_->addPassesToEmitFile(pass, dest, nullptr, llvm::CGFT_ObjectFile)) {
        return false;
    }
    
    pass.run(*module);
    return true;
}

bool TargetCodeGenerator::generateExecutable(llvm::Module* module, const String& filename) {
    // This would require linking with system libraries
    return false;
}

bool TargetCodeGenerator::generateAssembly(llvm::Module* module, const String& filename) {
    if (!targetMachine_) {
        return false;
    }
    
    std::error_code EC;
    llvm::raw_fd_ostream dest(filename, EC, llvm::sys::fs::OF_None);
    
    if (EC) {
        return false;
    }
    
    llvm::legacy::PassManager pass;
    if (targetMachine_->addPassesToEmitFile(pass, dest, nullptr, llvm::CGFT_AssemblyFile)) {
        return false;
    }
    
    pass.run(*module);
    return true;
}

bool TargetCodeGenerator::initializeTarget() {
    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmPrinters();
    
    auto targetTriple = llvm::sys::getDefaultTargetTriple();
    std::string error;
    
    target_ = llvm::TargetRegistry::lookupTarget(targetTriple, error);
    if (!target_) {
        return false;
    }
    
    setTargetOptions();
    return true;
}

void TargetCodeGenerator::setTargetOptions() {
    if (!target_) return;
    
    auto CPU = "generic";
    auto features = "";
    
    llvm::TargetOptions opt;
    auto RM = llvm::Optional<llvm::Reloc::Model>();
    
    targetMachine_ = target_->createTargetMachine(llvm::sys::getDefaultTargetTriple(), CPU, features, opt, RM);
}

} // namespace Scarlet 