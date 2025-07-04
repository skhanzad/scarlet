#include "Common.h"
#include "Lexer.h"
#include "Parser.h"
#include "Semantic.h"
#include "CodeGen.h"
#include "Utils.h"
#include <iostream>
#include <fstream>
#include <filesystem>

using namespace Scarlet;

void printUsage(const String& programName) {
    std::cout << "Usage: " << programName << " [options] <input-file>\n"
              << "Options:\n"
              << "  -o <output>     Specify output file name\n"
              << "  -S              Generate assembly instead of object file\n"
              << "  -c              Compile to object file (default)\n"
              << "  -E              Preprocess only (output to stdout)\n"
              << "  -v              Verbose output\n"
              << "  --help          Show this help message\n"
              << "  --version       Show version information\n";
}

void printVersion() {
    std::cout << "Scarlet Compiler v1.0.0\n"
              << "Built with LLVM\n";
}

struct CompilerOptions {
    String inputFile;
    String outputFile;
    bool generateAssembly = false;
    bool compileOnly = true;
    bool preprocessOnly = false;
    bool verbose = false;
};

CompilerOptions parseArguments(int argc, char* argv[]) {
    CompilerOptions options;
    
    for (int i = 1; i < argc; ++i) {
        String arg = argv[i];
        
        if (arg == "--help") {
            printUsage(argv[0]);
            exit(0);
        } else if (arg == "--version") {
            printVersion();
            exit(0);
        } else if (arg == "-o" && i + 1 < argc) {
            options.outputFile = argv[++i];
        } else if (arg == "-S") {
            options.generateAssembly = true;
        } else if (arg == "-c") {
            options.compileOnly = true;
        } else if (arg == "-E") {
            options.preprocessOnly = true;
        } else if (arg == "-v") {
            options.verbose = true;
        } else if (arg[0] == '-') {
            std::cerr << "Unknown option: " << arg << "\n";
            printUsage(argv[0]);
            exit(1);
        } else {
            if (!options.inputFile.empty()) {
                std::cerr << "Multiple input files not supported\n";
                exit(1);
            }
            options.inputFile = arg;
        }
    }
    
    if (options.inputFile.empty()) {
        std::cerr << "No input file specified\n";
        printUsage(argv[0]);
        exit(1);
    }
    
    // Set default output file if not specified
    if (options.outputFile.empty()) {
        std::filesystem::path inputPath(options.inputFile);
        String stem = inputPath.stem().string();
        
        if (options.generateAssembly) {
            options.outputFile = stem + ".s";
        } else {
            options.outputFile = stem + ".o";
        }
    }
    
    return options;
}

bool compileFile(const CompilerOptions& options) {
    try {
        Logger& logger = Logger::instance();
        if (options.verbose) {
            logger.setLevel(Logger::Level::DEBUG);
        }
        
        logger.info("Compiling {}", options.inputFile);
        
        // Read source file
        String source = readFile(options.inputFile);
        logger.debug("Read {} characters from source file", source.length());
        
        // Lexical analysis
        logger.debug("Starting lexical analysis");
        Lexer lexer(source);
        auto tokens = lexer.tokenize();
        logger.debug("Generated {} tokens", tokens.size());
        
        if (options.verbose) {
            for (const auto& token : tokens) {
                logger.debug("Token: {}", token->toString());
            }
        }
        
        // Check for lexer errors
        for (const auto& token : tokens) {
            if (token->type() == TokenType::ERROR) {
                logger.error("Lexical error: {}", token->value());
                return false;
            }
        }
        
        // Parsing
        logger.debug("Starting parsing");
        Parser parser(tokens);
        auto ast = parser.parse();
        logger.debug("Parsing completed");
        
        if (options.preprocessOnly) {
            // For now, just output the tokens
            for (const auto& token : tokens) {
                if (token->type() != TokenType::END_OF_FILE) {
                    std::cout << token->toString() << "\n";
                }
            }
            return true;
        }
        
        // Semantic analysis
        logger.debug("Starting semantic analysis");
        SemanticAnalyzer analyzer;
        if (!analyzer.analyze(static_cast<Program*>(ast.get()))) {
            logger.error("Semantic analysis failed");
            for (const auto& error : analyzer.getErrors()) {
                logger.error("  {}", error);
            }
            return false;
        }
        logger.debug("Semantic analysis completed");
        
        // Code generation
        logger.debug("Starting code generation");
        LLVMCodeGenerator codegen;
        if (!codegen.generate(static_cast<Program*>(ast.get()))) {
            logger.error("Code generation failed");
            return false;
        }
        logger.debug("Code generation completed");
        
        // Output generation
        if (options.generateAssembly) {
            if (!codegen.writeToFile(options.outputFile)) {
                logger.error("Failed to write assembly to {}", options.outputFile);
                return false;
            }
        } else {
            if (!codegen.writeToFile(options.outputFile)) {
                logger.error("Failed to write object file to {}", options.outputFile);
                return false;
            }
        }
        
        logger.info("Compilation successful: {}", options.outputFile);
        return true;
        
    } catch (const CompilerError& e) {
        Logger::instance().error("Compilation error: {}", e.what());
        return false;
    } catch (const std::exception& e) {
        Logger::instance().error("Unexpected error: {}", e.what());
        return false;
    }
}

int main(int argc, char* argv[]) {
    try {
        // Initialize LLVM
        llvm::LLVMContext context;
        llvm::InitializeAllTargetInfos();
        llvm::InitializeAllTargets();
        llvm::InitializeAllTargetMCs();
        llvm::InitializeAllAsmPrinters();
        llvm::InitializeAllAsmParsers();
        
        // Parse command line arguments
        auto options = parseArguments(argc, argv);
        
        // Compile the file
        bool success = compileFile(options);
        
        return success ? 0 : 1;
        
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << "\n";
        return 1;
    }
} 