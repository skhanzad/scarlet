#include <iostream>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Verifier.h>

int main() {
    std::cout << "Testing LLVM installation..." << std::endl;
    
    try {
        // Create LLVM context and module
        llvm::LLVMContext context;
        std::unique_ptr<llvm::Module> module = std::make_unique<llvm::Module>("test", context);
        llvm::IRBuilder<> builder(context);
        
        // Create a simple function
        llvm::FunctionType* funcType = llvm::FunctionType::get(
            llvm::Type::getInt32Ty(context), // Return type
            false // No varargs
        );
        
        llvm::Function* func = llvm::Function::Create(
            funcType,
            llvm::Function::ExternalLinkage,
            "test_function",
            module.get()
        );
        
        // Create basic block
        llvm::BasicBlock* entry = llvm::BasicBlock::Create(context, "entry", func);
        builder.SetInsertPoint(entry);
        
        // Create a simple return statement
        llvm::Value* constant = llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), 42);
        builder.CreateRet(constant);
        
        // Verify the function
        if (llvm::verifyFunction(*func, &llvm::errs())) {
            std::cout << "Error: Function verification failed!" << std::endl;
            return 1;
        }
        
        std::cout << "Success! LLVM is working correctly." << std::endl;
        std::cout << "Generated LLVM IR:" << std::endl;
        module->print(llvm::outs(), nullptr);
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
        return 1;
    }
} 