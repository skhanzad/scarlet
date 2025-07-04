# Utility library
add_library(scarlet_utils
    Utils.cpp
)

# Lexer library
add_library(scarlet_lexer
    Lexer.cpp
)

target_link_libraries(scarlet_lexer scarlet_utils)

# AST library
add_library(scarlet_ast
    AST.cpp
)

target_link_libraries(scarlet_ast scarlet_utils)

# Parser library
add_library(scarlet_parser
    Parser.cpp
)

target_link_libraries(scarlet_parser scarlet_lexer scarlet_ast)

# Semantic analysis library
add_library(scarlet_semantic
    Semantic.cpp
)

target_link_libraries(scarlet_semantic scarlet_ast scarlet_utils)

# Code generation library
add_library(scarlet_codegen
    CodeGen.cpp
)

target_link_libraries(scarlet_codegen scarlet_ast scarlet_utils) 