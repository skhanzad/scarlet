# Scarlet Compiler Template

A comprehensive C++ template for building a compiler from scratch using LLVM for code generation. This template provides a solid foundation for creating a programming language compiler with all essential components.

## Features

- **Lexical Analysis**: Token-based lexer with support for keywords, operators, literals, and identifiers
- **Parsing**: Recursive descent parser with operator precedence and error recovery
- **Abstract Syntax Tree (AST)**: Complete AST representation with visitor pattern
- **Semantic Analysis**: Type checking, symbol table management, and error reporting
- **Code Generation**: LLVM-based code generation with optimization support
- **Error Handling**: Comprehensive error reporting with source location tracking
- **Logging**: Configurable logging system for debugging and development

## Project Structure

```
scarlet/
├── CMakeLists.txt          # Main CMake configuration
├── include/                # Header files
│   ├── Common.h           # Common includes and forward declarations
│   ├── Utils.h            # Utility classes and functions
│   ├── Lexer.h            # Lexical analyzer
│   ├── AST.h              # Abstract Syntax Tree
│   ├── Parser.h           # Recursive descent parser
│   ├── Semantic.h         # Semantic analysis
│   └── CodeGen.h          # LLVM code generation
├── src/                   # Source files
│   ├── CMakeLists.txt     # Source CMake configuration
│   ├── main.cpp           # Main entry point
│   ├── Utils.cpp          # Utility implementations
│   ├── Lexer.cpp          # Lexer implementation
│   ├── AST.cpp            # AST visitor implementations
│   ├── Parser.cpp         # Parser implementation
│   ├── Semantic.cpp       # Semantic analyzer
│   └── CodeGen.cpp        # Code generator
└── README.md              # This file
```

## Dependencies

- **C++17** or later
- **CMake** 3.16 or later
- **LLVM** (for code generation)
- **fmt** library (for string formatting)

## Building

### Prerequisites

#### Option 1: Automatic Installation (Recommended)

**Linux/macOS:**
```bash
chmod +x install_dependencies.sh
./install_dependencies.sh
```

**Windows:**
```cmd
install_dependencies.bat
```

#### Option 2: Manual Installation

1. **LLVM** (Required):
   ```bash
   # Ubuntu/Debian
   sudo apt-get install llvm-dev
   
   # macOS
   brew install llvm
   
   # Windows (using vcpkg)
   vcpkg install llvm
   ```

2. **fmt library** (Optional - compiler works without it):
   ```bash
   # Ubuntu/Debian
   sudo apt-get install libfmt-dev
   
   # macOS
   brew install fmt
   
   # Windows (using vcpkg)
   vcpkg install fmt
   ```

#### Option 3: Build Without External Dependencies

If you want to build without the fmt library, use the simplified CMakeLists:

```bash
cp CMakeLists_simple.txt CMakeLists.txt
mkdir build && cd build
cmake ..
make
```

### Build Instructions

```bash
mkdir build
cd build
cmake ..
make
```

## Usage

The compiler supports various command-line options:

```bash
# Basic compilation
./scarlet_compiler input.sc

# Generate assembly
./scarlet_compiler -S input.sc

# Specify output file
./scarlet_compiler -o output.o input.sc

# Verbose output
./scarlet_compiler -v input.sc

# Preprocess only (output tokens)
./scarlet_compiler -E input.sc

# Show help
./scarlet_compiler --help
```

## Language Features

The template supports a C-like language with the following features:

### Data Types
- `int` - 32-bit integers
- `float` - Double precision floating point
- `bool` - Boolean values
- `string` - String literals
- `void` - No return type

### Expressions
- Arithmetic: `+`, `-`, `*`, `/`, `%`
- Comparison: `==`, `!=`, `<`, `<=`, `>`, `>=`
- Logical: `&&`, `||`, `!`
- Assignment: `=`

### Statements
- Variable declarations: `var x: int = 10;`
- Function declarations: `function add(a: int, b: int): int { ... }`
- If statements: `if (condition) { ... } else { ... }`
- While loops: `while (condition) { ... }`
- Return statements: `return value;`
- Expression statements: `x + y;`

### Built-in Functions
- `print(string)` - Print to console
- `input()` - Read from console
- `sqrt(float)` - Square root

## Example Program

```c
function factorial(n: int): int {
    if (n <= 1) {
        return 1;
    } else {
        return n * factorial(n - 1);
    }
}

function main(): void {
    var result: int = factorial(5);
    print("Factorial of 5 is: ");
    print(result);
}
```

## Architecture Overview

### 1. Lexical Analysis (`Lexer`)
- Converts source code into tokens
- Handles keywords, operators, literals, and identifiers
- Supports comments and whitespace
- Provides detailed error reporting

### 2. Parsing (`Parser`)
- Recursive descent parser with operator precedence
- Builds Abstract Syntax Tree (AST)
- Implements error recovery and synchronization
- Supports all language constructs

### 3. Abstract Syntax Tree (`AST`)
- Complete AST representation for all language constructs
- Visitor pattern for tree traversal
- Type-safe node hierarchy
- Source location tracking

### 4. Semantic Analysis (`Semantic`)
- Symbol table with scoping
- Type checking and validation
- Error reporting and recovery
- Built-in function registration

### 5. Code Generation (`CodeGen`)
- LLVM IR generation
- Target code generation
- Optimization passes
- Assembly and object file output

## Extending the Compiler

### Adding New Language Features

1. **Lexer**: Add new token types in `Common.h` and implement recognition in `Lexer.cpp`
2. **Parser**: Add parsing methods in `Parser.cpp` following the recursive descent pattern
3. **AST**: Create new AST node classes in `AST.h` and implement visitor methods
4. **Semantic**: Add type checking and validation in `Semantic.cpp`
5. **CodeGen**: Implement LLVM IR generation in `CodeGen.cpp`

### Example: Adding Arrays

```cpp
// 1. Add to Common.h
enum class TokenType {
    // ... existing tokens
    LEFT_BRACKET,
    RIGHT_BRACKET,
    // ...
};

// 2. Add to AST.h
class ArrayExpression : public Expression {
    // Implementation
};

// 3. Add parsing in Parser.cpp
ExpressionPtr Parser::parseArrayExpression() {
    // Implementation
}

// 4. Add semantic checking in Semantic.cpp
void SemanticAnalyzer::visitArrayExpression(ArrayExpression* expr) {
    // Implementation
}

// 5. Add code generation in CodeGen.cpp
void LLVMCodeGenerator::visitArrayExpression(ArrayExpression* expr) {
    // Implementation
}
```

## Error Handling

The compiler provides comprehensive error handling:

- **Lexical errors**: Invalid characters, unterminated strings
- **Syntax errors**: Missing tokens, invalid expressions
- **Semantic errors**: Type mismatches, undefined variables
- **Code generation errors**: LLVM IR generation failures

All errors include source location information for easy debugging.

## Testing

The template includes a basic testing framework. Add test cases in the `tests/` directory:

```cpp
// tests/lexer_test.cpp
#include "Lexer.h"
#include <cassert>

void testLexer() {
    String source = "var x: int = 42;";
    Lexer lexer(source);
    auto tokens = lexer.tokenize();
    assert(tokens.size() > 0);
}
```

## Performance Considerations

- **Memory management**: Uses smart pointers for automatic memory management
- **String handling**: Efficient string operations with move semantics
- **LLVM optimization**: Leverages LLVM's optimization passes
- **Error recovery**: Minimal performance impact from error handling

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Add tests for new features
5. Submit a pull request

## License

This template is provided as-is for educational and development purposes. Feel free to modify and extend it for your own projects.

## Resources

- [LLVM Documentation](https://llvm.org/docs/)
- [CMake Documentation](https://cmake.org/documentation/)
- [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/)
- [Compiler Design Principles](https://en.wikipedia.org/wiki/Compiler)

## Support

For questions and issues:
1. Check the documentation
2. Review the example code
3. Open an issue on the repository
4. Consult the LLVM and CMake documentation
