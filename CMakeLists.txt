cmake_minimum_required(VERSION 3.16)
project(ScarletCompiler VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# LLVM Paths (adjust as needed)
set(LLVM_INSTALL_PREFIX "C:/Program Files/LLVM")
set(LLVM_INCLUDE_DIRS "${LLVM_INSTALL_PREFIX}/include")
set(LLVM_LIBRARY_DIRS "${LLVM_INSTALL_PREFIX}/lib")

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${LLVM_INCLUDE_DIRS})

# Link directories
link_directories(${LLVM_LIBRARY_DIRS})

# Check if LLVM headers exist
if(NOT EXISTS "${LLVM_INCLUDE_DIRS}/llvm-c/Remarks.h")
    message(FATAL_ERROR "LLVM headers not found at ${LLVM_INCLUDE_DIRS}")
endif()

# Check if LLVM libraries exist (commonly named LLVM.lib or clang.lib)
if(NOT EXISTS "${LLVM_LIBRARY_DIRS}/LLVM-C.lib")
    message(FATAL_ERROR "LLVM library not found at ${LLVM_LIBRARY_DIRS}")
endif()

# Compiler flags
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -O2")
endif()

# Add subdirectories for project components
add_subdirectory(src)

# Main executable
add_executable(scarlet_compiler 
    src/main.cpp
)

# Link libraries (adjust based on what your LLVM provides)
target_link_libraries(scarlet_compiler 
    scarlet_lexer
    scarlet_parser
    scarlet_ast
    scarlet_semantic
    scarlet_codegen
    scarlet_utils
    LLVM    # Link against LLVM library
    clang   # If you need libclang functionality
)

# fmt library setup (optional fallback)
add_subdirectory(external/fmt)
target_link_libraries(scarlet_compiler fmt::fmt)

# Output directories
set_target_properties(scarlet_compiler PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Install target
install(TARGETS scarlet_compiler
    RUNTIME DESTINATION bin
)
