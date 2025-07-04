#pragma once

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <variant>
#include <optional>
#include <functional>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstring>

// LLVM includes
#include <llvm/IR/Value.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/Host.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/raw_ostream.h>

// fmt library for formatting
#ifdef NO_FMT_LIBRARY
    // Fallback to standard C++ formatting
    #include <sstream>
    #include <iomanip>
    namespace fmt {
        template<typename... Args>
        std::string format(const std::string& format_str, Args&&... args) {
            // Simple fallback - just return the format string for now
            // In a real implementation, you'd want a proper formatting library
            return format_str;
        }
    }
#else
    #include <fmt/format.h>
    #include <fmt/ostream.h>
#endif

// Forward declarations
namespace Scarlet {
    // Lexer
    class Token;
    class Lexer;
    
    // Parser
    class Parser;
    
    // AST
    class ASTNode;
    class Expression;
    class Statement;
    class Program;
    class BinaryExpression;
    class UnaryExpression;
    class LiteralExpression;
    class VariableExpression;
    class AssignmentExpression;
    class FunctionCallExpression;
    class IfStatement;
    class WhileStatement;
    class ReturnStatement;
    class VariableDeclaration;
    class FunctionDeclaration;
    class BlockStatement;
    
    // Semantic Analysis
    class SymbolTable;
    class TypeChecker;
    class SemanticAnalyzer;
    
    // Code Generation
    class CodeGenerator;
    class LLVMCodeGenerator;
    
    // Utilities
    class SourceLocation;
    class CompilerError;
    class Logger;
}

// Common type aliases
using String = std::string;
using StringView = std::string_view;
using TokenList = std::vector<std::unique_ptr<Scarlet::Token>>;
using ASTNodePtr = std::unique_ptr<Scarlet::ASTNode>;
using ExpressionPtr = std::unique_ptr<Scarlet::Expression>;
using StatementPtr = std::unique_ptr<Scarlet::Statement>;

// Utility macros
#define SCARLET_ASSERT(condition, message) \
    assert((condition) && (message))

#define SCARLET_UNREACHABLE() \
    assert(false && "Unreachable code reached")

#define SCARLET_NOT_IMPLEMENTED() \
    throw std::runtime_error("Not implemented: " __FUNCTION__)

// Common enums
namespace Scarlet {
    enum class TokenType {
        // Literals
        INTEGER,
        FLOAT,
        STRING,
        IDENTIFIER,
        
        // Keywords
        IF,
        ELSE,
        WHILE,
        FOR,
        RETURN,
        FUNCTION,
        VAR,
        LET,
        CONST,
        TRUE,
        FALSE,
        NULL_LITERAL,
        
        // Operators
        PLUS,
        MINUS,
        MULTIPLY,
        DIVIDE,
        MODULO,
        ASSIGN,
        EQUAL,
        NOT_EQUAL,
        LESS,
        LESS_EQUAL,
        GREATER,
        GREATER_EQUAL,
        AND,
        OR,
        NOT,
        
        // Delimiters
        LEFT_PAREN,
        RIGHT_PAREN,
        LEFT_BRACE,
        RIGHT_BRACE,
        LEFT_BRACKET,
        RIGHT_BRACKET,
        SEMICOLON,
        COMMA,
        DOT,
        
        // Special
        END_OF_FILE,
        ERROR
    };
    
    enum class DataType {
        VOID,
        INT,
        FLOAT,
        BOOL,
        STRING,
        ARRAY,
        FUNCTION,
        UNKNOWN
    };
    
    enum class OperatorType {
        ADD,
        SUBTRACT,
        MULTIPLY,
        DIVIDE,
        MODULO,
        ASSIGN,
        EQUAL,
        NOT_EQUAL,
        LESS,
        LESS_EQUAL,
        GREATER,
        GREATER_EQUAL,
        AND,
        OR,
        NOT
    };
} 