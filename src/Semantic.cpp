#include "Semantic.h"
#include <algorithm>

namespace Scarlet {

// SymbolTable implementation
SymbolTable::SymbolTable() {
    enterScope(); // Global scope
}

void SymbolTable::enterScope() {
    scopes_.emplace_back();
}

void SymbolTable::exitScope() {
    if (scopes_.size() > 1) {
        scopes_.pop_back();
    }
}

bool SymbolTable::insert(const String& name, const Symbol& symbol) {
    if (lookupCurrentScope(name)) {
        return false; // Already declared in current scope
    }
    
    scopes_.back()[name] = symbol;
    return true;
}

Symbol* SymbolTable::lookup(const String& name) {
    for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
        auto symbolIt = it->find(name);
        if (symbolIt != it->end()) {
            return &symbolIt->second;
        }
    }
    return nullptr;
}

Symbol* SymbolTable::lookupCurrentScope(const String& name) {
    auto it = scopes_.back().find(name);
    return it != scopes_.back().end() ? &it->second : nullptr;
}

void SymbolTable::clear() {
    scopes_.clear();
    enterScope(); // Restore global scope
}

// TypeChecker implementation
TypeChecker::TypeChecker() : currentReturnType_(DataType::VOID), inFunction_(false) {
    BuiltinFunctions::registerBuiltins(symbolTable_);
}

DataType TypeChecker::checkExpression(Expression* expr) {
    expr->accept(*this);
    return currentExpressionType_;
}

void TypeChecker::checkStatement(Statement* stmt) {
    stmt->accept(*this);
}

void TypeChecker::checkProgram(Program* program) {
    program->accept(*this);
}

bool TypeChecker::isCompatible(DataType from, DataType to) {
    if (from == to) return true;
    if (from == DataType::UNKNOWN || to == DataType::UNKNOWN) return true;
    
    // Allow implicit conversions
    if (from == DataType::INT && to == DataType::FLOAT) return true;
    if (from == DataType::FLOAT && to == DataType::INT) return true;
    
    return false;
}

DataType TypeChecker::getResultType(OperatorType op, DataType left, DataType right) {
    switch (op) {
        case OperatorType::ADD:
        case OperatorType::SUBTRACT:
        case OperatorType::MULTIPLY:
        case OperatorType::DIVIDE:
        case OperatorType::MODULO:
            if (left == DataType::INT && right == DataType::INT) return DataType::INT;
            if ((left == DataType::INT || left == DataType::FLOAT) && 
                (right == DataType::INT || right == DataType::FLOAT)) {
                return (left == DataType::FLOAT || right == DataType::FLOAT) ? DataType::FLOAT : DataType::INT;
            }
            break;
            
        case OperatorType::EQUAL:
        case OperatorType::NOT_EQUAL:
        case OperatorType::LESS:
        case OperatorType::LESS_EQUAL:
        case OperatorType::GREATER:
        case OperatorType::GREATER_EQUAL:
            return DataType::BOOL;
            
        case OperatorType::AND:
        case OperatorType::OR:
            if (left == DataType::BOOL && right == DataType::BOOL) return DataType::BOOL;
            break;
            
        default:
            break;
    }
    
    return DataType::UNKNOWN;
}

DataType TypeChecker::getUnaryResultType(OperatorType op, DataType operand) {
    switch (op) {
        case OperatorType::SUBTRACT:
            if (operand == DataType::INT || operand == DataType::FLOAT) return operand;
            break;
        case OperatorType::NOT:
            if (operand == DataType::BOOL) return DataType::BOOL;
            break;
        default:
            break;
    }
    return DataType::UNKNOWN;
}

void TypeChecker::reportError(const String& message, const SourceLocation& location) {
    errors_.push_back(fmt::format("{}: {}", location.toString(), message));
}

// SemanticAnalyzer implementation
SemanticAnalyzer::SemanticAnalyzer() 
    : currentExpressionType_(DataType::UNKNOWN), inLoop_(false), inFunction_(false), currentFunctionReturnType_(DataType::VOID) {
    BuiltinFunctions::registerBuiltins(symbolTable_);
}

bool SemanticAnalyzer::analyze(Program* program) {
    program->accept(*this);
    return !hasErrors();
}

// Expression visitors
void SemanticAnalyzer::visitLiteralExpression(LiteralExpression* expr) {
    currentExpressionType_ = expr->type();
}

void SemanticAnalyzer::visitVariableExpression(VariableExpression* expr) {
    Symbol* symbol = symbolTable_.lookup(expr->name());
    if (!symbol) {
        typeChecker_.reportError("Undefined variable: " + expr->name(), expr->location());
        currentExpressionType_ = DataType::UNKNOWN;
        return;
    }
    currentExpressionType_ = symbol->type;
}

void SemanticAnalyzer::visitBinaryExpression(BinaryExpression* expr) {
    DataType leftType = checkExpression(expr->left());
    DataType rightType = checkExpression(expr->right());
    
    currentExpressionType_ = typeChecker_.getResultType(expr->op(), leftType, rightType);
    
    if (currentExpressionType_ == DataType::UNKNOWN) {
        typeChecker_.reportError("Invalid operation between types " + 
            dataTypeToString(leftType) + " and " + dataTypeToString(rightType), expr->location());
    }
}

void SemanticAnalyzer::visitUnaryExpression(UnaryExpression* expr) {
    DataType operandType = checkExpression(expr->operand());
    currentExpressionType_ = typeChecker_.getUnaryResultType(expr->op(), operandType);
    
    if (currentExpressionType_ == DataType::UNKNOWN) {
        typeChecker_.reportError("Invalid unary operation on type " + 
            dataTypeToString(operandType), expr->location());
    }
}

void SemanticAnalyzer::visitAssignmentExpression(AssignmentExpression* expr) {
    DataType valueType = checkExpression(expr->value());
    
    Symbol* symbol = symbolTable_.lookup(expr->name());
    if (!symbol) {
        typeChecker_.reportError("Undefined variable: " + expr->name(), expr->location());
        currentExpressionType_ = DataType::UNKNOWN;
        return;
    }
    
    if (!typeChecker_.isCompatible(valueType, symbol->type)) {
        typeChecker_.reportError("Cannot assign " + dataTypeToString(valueType) + 
            " to variable of type " + dataTypeToString(symbol->type), expr->location());
    }
    
    currentExpressionType_ = symbol->type;
}

void SemanticAnalyzer::visitFunctionCallExpression(FunctionCallExpression* expr) {
    Symbol* symbol = symbolTable_.lookup(expr->name());
    if (!symbol || !symbol->isFunction) {
        typeChecker_.reportError("Undefined function: " + expr->name(), expr->location());
        currentExpressionType_ = DataType::UNKNOWN;
        return;
    }
    
    if (expr->arguments().size() != symbol->parameterTypes.size()) {
        typeChecker_.reportError("Function " + expr->name() + " expects " + 
            std::to_string(symbol->parameterTypes.size()) + " arguments, got " + 
            std::to_string(expr->arguments().size()), expr->location());
        currentExpressionType_ = DataType::UNKNOWN;
        return;
    }
    
    for (size_t i = 0; i < expr->arguments().size(); ++i) {
        DataType argType = checkExpression(expr->arguments()[i].get());
        if (!typeChecker_.isCompatible(argType, symbol->parameterTypes[i])) {
            typeChecker_.reportError("Argument " + std::to_string(i + 1) + " type mismatch", expr->location());
        }
    }
    
    currentExpressionType_ = symbol->returnType;
}

// Statement visitors
void SemanticAnalyzer::visitBlockStatement(BlockStatement* stmt) {
    symbolTable_.enterScope();
    
    for (const auto& statement : stmt->statements()) {
        statement->accept(*this);
    }
    
    symbolTable_.exitScope();
}

void SemanticAnalyzer::visitVariableDeclaration(VariableDeclaration* stmt) {
    DataType declaredType = stmt->type();
    DataType initializerType = DataType::UNKNOWN;
    
    if (stmt->initializer()) {
        initializerType = checkExpression(stmt->initializer());
    }
    
    if (declaredType == DataType::UNKNOWN) {
        declaredType = initializerType;
    } else if (initializerType != DataType::UNKNOWN && !typeChecker_.isCompatible(initializerType, declaredType)) {
        typeChecker_.reportError("Cannot initialize " + dataTypeToString(declaredType) + 
            " with " + dataTypeToString(initializerType), stmt->location());
    }
    
    Symbol symbol(stmt->name(), declaredType, false, stmt->location());
    if (!symbolTable_.insert(stmt->name(), symbol)) {
        typeChecker_.reportError("Variable already declared: " + stmt->name(), stmt->location());
    }
}

void SemanticAnalyzer::visitFunctionDeclaration(FunctionDeclaration* stmt) {
    Symbol symbol(stmt->name(), stmt->returnType(), true, stmt->location());
    symbol.returnType = stmt->returnType();
    
    for (const auto& param : stmt->parameters()) {
        symbol.parameterTypes.push_back(param.second);
    }
    
    if (!symbolTable_.insert(stmt->name(), symbol)) {
        typeChecker_.reportError("Function already declared: " + stmt->name(), stmt->location());
        return;
    }
    
    // Enter function scope
    symbolTable_.enterScope();
    bool wasInFunction = inFunction_;
    DataType wasReturnType = currentFunctionReturnType_;
    
    inFunction_ = true;
    currentFunctionReturnType_ = stmt->returnType();
    
    // Add parameters to scope
    for (const auto& param : stmt->parameters()) {
        Symbol paramSymbol(param.first, param.second, false, stmt->location());
        symbolTable_.insert(param.first, paramSymbol);
    }
    
    // Check function body
    stmt->body()->accept(*this);
    
    // Restore state
    inFunction_ = wasInFunction;
    currentFunctionReturnType_ = wasReturnType;
    symbolTable_.exitScope();
}

void SemanticAnalyzer::visitIfStatement(IfStatement* stmt) {
    DataType conditionType = checkExpression(stmt->condition());
    if (conditionType != DataType::BOOL) {
        typeChecker_.reportError("If condition must be boolean", stmt->condition()->location());
    }
    
    stmt->thenBranch()->accept(*this);
    if (stmt->elseBranch()) {
        stmt->elseBranch()->accept(*this);
    }
}

void SemanticAnalyzer::visitWhileStatement(WhileStatement* stmt) {
    DataType conditionType = checkExpression(stmt->condition());
    if (conditionType != DataType::BOOL) {
        typeChecker_.reportError("While condition must be boolean", stmt->condition()->location());
    }
    
    bool wasInLoop = inLoop_;
    inLoop_ = true;
    
    stmt->body()->accept(*this);
    
    inLoop_ = wasInLoop;
}

void SemanticAnalyzer::visitReturnStatement(ReturnStatement* stmt) {
    if (!inFunction_) {
        typeChecker_.reportError("Return statement outside function", stmt->location());
        return;
    }
    
    DataType returnType = DataType::VOID;
    if (stmt->value()) {
        returnType = checkExpression(stmt->value());
    }
    
    if (!typeChecker_.isCompatible(returnType, currentFunctionReturnType_)) {
        typeChecker_.reportError("Return type mismatch", stmt->location());
    }
}

void SemanticAnalyzer::visitExpressionStatement(ExpressionStatement* stmt) {
    checkExpression(stmt->expression());
}

void SemanticAnalyzer::visitProgram(Program* program) {
    for (const auto& statement : program->statements()) {
        statement->accept(*this);
    }
}

// BuiltinFunctions implementation
void BuiltinFunctions::registerBuiltins(SymbolTable& symbolTable) {
    registerPrintFunction(symbolTable);
    registerInputFunction(symbolTable);
    registerMathFunctions(symbolTable);
}

void BuiltinFunctions::registerPrintFunction(SymbolTable& symbolTable) {
    Symbol printSymbol("print", DataType::FUNCTION, true);
    printSymbol.returnType = DataType::VOID;
    printSymbol.parameterTypes = {DataType::STRING};
    symbolTable.insert("print", printSymbol);
}

void BuiltinFunctions::registerInputFunction(SymbolTable& symbolTable) {
    Symbol inputSymbol("input", DataType::FUNCTION, true);
    inputSymbol.returnType = DataType::STRING;
    inputSymbol.parameterTypes = {};
    symbolTable.insert("input", inputSymbol);
}

void BuiltinFunctions::registerMathFunctions(SymbolTable& symbolTable) {
    // Add common math functions like sqrt, abs, etc.
    Symbol sqrtSymbol("sqrt", DataType::FUNCTION, true);
    sqrtSymbol.returnType = DataType::FLOAT;
    sqrtSymbol.parameterTypes = {DataType::FLOAT};
    symbolTable.insert("sqrt", sqrtSymbol);
}

} // namespace Scarlet 