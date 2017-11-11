#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>

#include <memory>

using namespace llvm;

int main()
{
    LLVMContext context;
    IRBuilder<> builder(context);
    Module* module = new Module("my_module", context);
    //function fib
    Function* ffib = Function::Create(FunctionType::get(
        Type::getInt32Ty(context), std::vector<Type*>{Type::getInt32Ty(context)}, false),
        Function::ExternalLinkage, "fib", module);

    auto arg = ffib->arg_begin();
    Value* n = &(*arg);
    n->setName("n");
    BasicBlock* startbb = BasicBlock::Create(context, "begin", ffib);
    BasicBlock* thenbb = BasicBlock::Create(context, "then", ffib);
    BasicBlock* elifbb = BasicBlock::Create(context, "elif", ffib);
    BasicBlock* elsebb = BasicBlock::Create(context, "else", ffib);
    BasicBlock* retbb = BasicBlock::Create(context, "ret", ffib);
    //start bb
    builder.SetInsertPoint(startbb);
    Value* equal0 = builder.CreateICmpEQ(n, ConstantInt::get(Type::getInt32Ty(context), 0, true),"equal0");
    builder.CreateCondBr(equal0, thenbb, elifbb);
    //then bb
    builder.SetInsertPoint(thenbb);
    Value *thenv = n;
    builder.CreateBr(retbb);
    //else if bb
    builder.SetInsertPoint(elifbb);
    Value* equal1 = builder.CreateICmpEQ(n, ConstantInt::get(Type::getInt32Ty(context), 1, true), "equal1");
    Value *elifv = n;
    builder.CreateCondBr(equal1, retbb, elsebb);
    //else bb
    builder.SetInsertPoint(elsebb);
    auto arg1 = builder.CreateNSWAdd(n, ConstantInt::get(Type::getInt32Ty(context), -1, true), "minus1");
    Value* o1 =  builder.CreateCall(ffib, std::vector<Value*>{arg1}, "fib1");
    auto arg2 = builder.CreateNSWAdd(n, ConstantInt::get(Type::getInt32Ty(context), -2, true), "minus2");
    Value* o2 = builder.CreateCall(ffib, std::vector<Value*>{arg2}, "fib2");
    Value* elsev = builder.CreateNSWAdd(o1, o2, "sum");
    builder.CreateBr(retbb);
    //return bb
    builder.SetInsertPoint(retbb);
    PHINode* pnret = builder.CreatePHI(Type::getInt32Ty(context), 3, "ret");
    pnret->addIncoming(thenv, thenbb);
    pnret->addIncoming(elifv, elifbb);
    pnret->addIncoming(elsev, elsebb);
    builder.CreateRet(pnret);
    builder.ClearInsertionPoint();
    //function main
    Function * fmain = Function::Create(FunctionType::get(
        Type::getInt32Ty(context), false), 
        Function::ExternalLinkage, "main", module);
    BasicBlock* entrybb = BasicBlock::Create(context, "entry", fmain);
    BasicBlock* loopbb = BasicBlock::Create(context, "loop", fmain);
    BasicBlock* endbb = BasicBlock::Create(context, "end", fmain);
    //entry
    builder.SetInsertPoint(entrybb);
    auto varx = builder.CreateAlloca(Type::getInt32Ty(context), nullptr, "x");
    builder.CreateStore(ConstantInt::get(Type::getInt32Ty(context), 0), varx);
    builder.CreateBr(loopbb);
    //loop x = x + fib(i), i = i + 1
    builder.SetInsertPoint(loopbb);
    PHINode* pni = builder.CreatePHI(Type::getInt32Ty(context), 2, "i");
    pni->addIncoming(ConstantInt::get(Type::getInt32Ty(context), 0), entrybb);
    auto iadd = builder.CreateNSWAdd(pni, ConstantInt::get(Type::getInt32Ty(context), 1), "addi");
    pni->addIncoming(iadd, loopbb);
    auto fibi =  builder.CreateCall(ffib, std::vector<Value*>{pni}, "fibi");
    auto xtemp = builder.CreateLoad(varx, "xtemp");
    auto sum = builder.CreateNSWAdd(xtemp, fibi, "xcur");
    builder.CreateStore(sum, varx);
    Value* ifloop = builder.CreateICmpULT(iadd, ConstantInt::get(Type::getInt32Ty(context), 10), "if_continue");
    builder.CreateCondBr(ifloop, loopbb, endbb);
    //end
    builder.SetInsertPoint(endbb);
    Value* ret = builder.CreateLoad(varx, "ret");
    builder.CreateRet(ret);
    builder.ClearInsertionPoint();

    module->print(outs(), nullptr);
    delete module;
    return 0;
}
