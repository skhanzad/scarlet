#include "Utils.h"
#include <fstream>
#include <sstream>
#include <cctype>

namespace Scarlet {

String readFile(const String& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw CompilerError("Could not open file: " + filename);
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

String escapeString(const String& str) {
    String result;
    result.reserve(str.length());
    
    for (char c : str) {
        switch (c) {
            case '\\': result += "\\\\"; break;
            case '\"': result += "\\\""; break;
            case '\n': result += "\\n"; break;
            case '\t': result += "\\t"; break;
            case '\r': result += "\\r"; break;
            default: result += c; break;
        }
    }
    
    return result;
}

String unescapeString(const String& str) {
    String result;
    result.reserve(str.length());
    
    for (size_t i = 0; i < str.length(); ++i) {
        if (str[i] == '\\' && i + 1 < str.length()) {
            switch (str[i + 1]) {
                case '\\': result += '\\'; break;
                case '\"': result += '\"'; break;
                case 'n': result += '\n'; break;
                case 't': result += '\t'; break;
                case 'r': result += '\r'; break;
                default: result += str[i + 1]; break;
            }
            ++i; // Skip the next character
        } else {
            result += str[i];
        }
    }
    
    return result;
}

bool isWhitespace(char c) {
    return std::isspace(static_cast<unsigned char>(c));
}

bool isDigit(char c) {
    return std::isdigit(static_cast<unsigned char>(c));
}

bool isAlpha(char c) {
    return std::isalpha(static_cast<unsigned char>(c)) || c == '_';
}

bool isAlphaNumeric(char c) {
    return isAlpha(c) || isDigit(c);
}

String tokenTypeToString(TokenType type) {
    switch (type) {
        case TokenType::INTEGER: return "INTEGER";
        case TokenType::FLOAT: return "FLOAT";
        case TokenType::STRING: return "STRING";
        case TokenType::IDENTIFIER: return "IDENTIFIER";
        case TokenType::IF: return "IF";
        case TokenType::ELSE: return "ELSE";
        case TokenType::WHILE: return "WHILE";
        case TokenType::FOR: return "FOR";
        case TokenType::RETURN: return "RETURN";
        case TokenType::FUNCTION: return "FUNCTION";
        case TokenType::VAR: return "VAR";
        case TokenType::LET: return "LET";
        case TokenType::CONST: return "CONST";
        case TokenType::TRUE: return "TRUE";
        case TokenType::FALSE: return "FALSE";
        case TokenType::NULL_LITERAL: return "NULL";
        case TokenType::PLUS: return "PLUS";
        case TokenType::MINUS: return "MINUS";
        case TokenType::MULTIPLY: return "MULTIPLY";
        case TokenType::DIVIDE: return "DIVIDE";
        case TokenType::MODULO: return "MODULO";
        case TokenType::ASSIGN: return "ASSIGN";
        case TokenType::EQUAL: return "EQUAL";
        case TokenType::NOT_EQUAL: return "NOT_EQUAL";
        case TokenType::LESS: return "LESS";
        case TokenType::LESS_EQUAL: return "LESS_EQUAL";
        case TokenType::GREATER: return "GREATER";
        case TokenType::GREATER_EQUAL: return "GREATER_EQUAL";
        case TokenType::AND: return "AND";
        case TokenType::OR: return "OR";
        case TokenType::NOT: return "NOT";
        case TokenType::LEFT_PAREN: return "LEFT_PAREN";
        case TokenType::RIGHT_PAREN: return "RIGHT_PAREN";
        case TokenType::LEFT_BRACE: return "LEFT_BRACE";
        case TokenType::RIGHT_BRACE: return "RIGHT_BRACE";
        case TokenType::LEFT_BRACKET: return "LEFT_BRACKET";
        case TokenType::RIGHT_BRACKET: return "RIGHT_BRACKET";
        case TokenType::SEMICOLON: return "SEMICOLON";
        case TokenType::COMMA: return "COMMA";
        case TokenType::DOT: return "DOT";
        case TokenType::END_OF_FILE: return "END_OF_FILE";
        case TokenType::ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

String dataTypeToString(DataType type) {
    switch (type) {
        case DataType::VOID: return "void";
        case DataType::INT: return "int";
        case DataType::FLOAT: return "float";
        case DataType::BOOL: return "bool";
        case DataType::STRING: return "string";
        case DataType::ARRAY: return "array";
        case DataType::FUNCTION: return "function";
        case DataType::UNKNOWN: return "unknown";
        default: return "unknown";
    }
}

String operatorTypeToString(OperatorType type) {
    switch (type) {
        case OperatorType::ADD: return "+";
        case OperatorType::SUBTRACT: return "-";
        case OperatorType::MULTIPLY: return "*";
        case OperatorType::DIVIDE: return "/";
        case OperatorType::MODULO: return "%";
        case OperatorType::ASSIGN: return "=";
        case OperatorType::EQUAL: return "==";
        case OperatorType::NOT_EQUAL: return "!=";
        case OperatorType::LESS: return "<";
        case OperatorType::LESS_EQUAL: return "<=";
        case OperatorType::GREATER: return ">";
        case OperatorType::GREATER_EQUAL: return ">=";
        case OperatorType::AND: return "&&";
        case OperatorType::OR: return "||";
        case OperatorType::NOT: return "!";
        default: return "unknown";
    }
}

} // namespace Scarlet 