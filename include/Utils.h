#pragma once

#include "Common.h"
#include <stdexcept>
#include <vector>
#include <memory>

namespace Scarlet {

class SourceLocation {
public:
    SourceLocation(size_t line = 1, size_t column = 1, size_t offset = 0)
        : line_(line), column_(column), offset_(offset) {}
    
    size_t line() const { return line_; }
    size_t column() const { return column_; }
    size_t offset() const { return offset_; }
    
    void advance(char c) {
        if (c == '\n') {
            line_++;
            column_ = 1;
        } else {
            column_++;
        }
        offset_++;
    }
    
    String toString() const {
        return fmt::format("{}:{}", line_, column_);
    }
    
private:
    size_t line_;
    size_t column_;
    size_t offset_;
};

class CompilerError : public std::runtime_error {
public:
    CompilerError(const String& message, const SourceLocation& location = SourceLocation())
        : std::runtime_error(fmt::format("{}: {}", location.toString(), message))
        , location_(location) {}
    
    const SourceLocation& location() const { return location_; }
    
private:
    SourceLocation location_;
};

class Logger {
public:
    enum class Level {
        DEBUG,
        INFO,
        WARNING,
        ERROR
    };
    
    static Logger& instance() {
        static Logger instance;
        return instance;
    }
    
    void setLevel(Level level) { level_ = level; }
    
    template<typename... Args>
    void debug(const String& format, Args&&... args) {
        log(Level::DEBUG, format, std::forward<Args>(args)...);
    }
    
    template<typename... Args>
    void info(const String& format, Args&&... args) {
        log(Level::INFO, format, std::forward<Args>(args)...);
    }
    
    template<typename... Args>
    void warning(const String& format, Args&&... args) {
        log(Level::WARNING, format, std::forward<Args>(args)...);
    }
    
    template<typename... Args>
    void error(const String& format, Args&&... args) {
        log(Level::ERROR, format, std::forward<Args>(args)...);
    }
    
private:
    Logger() : level_(Level::INFO) {}
    
    template<typename... Args>
    void log(Level msgLevel, const String& format, Args&&... args) {
        if (msgLevel >= level_) {
            String levelStr;
            switch (msgLevel) {
                case Level::DEBUG: levelStr = "DEBUG"; break;
                case Level::INFO: levelStr = "INFO"; break;
                case Level::WARNING: levelStr = "WARNING"; break;
                case Level::ERROR: levelStr = "ERROR"; break;
            }
            
            String message = fmt::format(format, std::forward<Args>(args)...);
            std::cerr << fmt::format("[{}] {}\n", levelStr, message);
        }
    }
    
    Level level_;
};

// Utility functions
String readFile(const String& filename);
String escapeString(const String& str);
String unescapeString(const String& str);
bool isWhitespace(char c);
bool isDigit(char c);
bool isAlpha(char c);
bool isAlphaNumeric(char c);
String tokenTypeToString(TokenType type);
String dataTypeToString(DataType type);
String operatorTypeToString(OperatorType type);

// Template utilities
template<typename T>
class ScopeGuard {
public:
    ScopeGuard(T&& cleanup) : cleanup_(std::forward<T>(cleanup)), active_(true) {}
    
    ~ScopeGuard() {
        if (active_) {
            cleanup_();
        }
    }
    
    void dismiss() { active_ = false; }
    
private:
    T cleanup_;
    bool active_;
};

template<typename T>
ScopeGuard<T> makeScopeGuard(T&& cleanup) {
    return ScopeGuard<T>(std::forward<T>(cleanup));
}

} // namespace Scarlet 