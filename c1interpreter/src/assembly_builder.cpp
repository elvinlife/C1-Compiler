
#include "assembly_builder.h"

#include <vector>
#include <iostream>

using namespace llvm;
using namespace c1_recognizer::syntax_tree;

void assembly_builder::visit(assembly &node)
{
  for (auto &gdef : node.global_defs)
  {
    in_global = true;
    gdef->accept(*this);
  }
}

void assembly_builder::visit(func_def_syntax &node)
{
  if(functions.find(node.name) == functions.end())
  {
    current_function = Function::Create(FunctionType::get(Type::getVoidTy(context), {}, false),
                                    GlobalValue::LinkageTypes::ExternalLinkage,
                                    node.name, module.get());
    functions[node.name] = current_function;
    BasicBlock* L0bb = BasicBlock::Create(context, "L", current_function);
    builder.SetInsertPoint(L0bb);
    node.body->accept(*this);
    builder.CreateRetVoid();
    builder.ClearInsertionPoint();
  }
  else
  {
    error_flag = true;
    err.error(node.line, node.pos, "redefinition of function: "+node.name);
    return;
  }
}

void assembly_builder::visit(cond_syntax &node)
{
  lval_as_rval = false;
  node.lhs->accept(*this);
  if(!value_result) return;
  Value* l_value = value_result;
  node.rhs->accept(*this);
  if(!value_result) return;
  switch(node.op)
  {
    case relop::equal:
      value_result = builder.CreateICmpEQ(l_value, value_result);
      break;
    case relop::greater:
      value_result = builder.CreateICmpSGT(l_value, value_result);
      break;
    case relop::greater_equal:
      value_result = builder.CreateICmpSGE(l_value, value_result);
      break;
    case relop::less:
      value_result = builder.CreateICmpSLT(l_value, value_result);
      break;
    case relop::less_equal:
      value_result = builder.CreateICmpSLE(l_value, value_result);
      break;
    case relop::non_equal:
      value_result = builder.CreateICmpNE(l_value, value_result);
      break;
    default:
      error_flag = true;
      err.error(node.line, node.pos, "inivalid relation operator");
      value_result = nullptr;
      break;
  }
}

void assembly_builder::visit(binop_expr_syntax &node)
{
  if(constexpr_expected)
  {
    node.lhs->accept(*this);
    int L = const_result;
    node.rhs->accept(*this);
    int R = const_result;
    switch(node.op)
    {
      case binop::plus:
        const_result = L+R;
        break;
      case binop::minus:
        const_result = L-R;
        break;
      case binop::multiply:
        const_result = L*R;
        break;
      case binop::divide:
        if(R != 0)
          const_result = L/R;
        else
        {
          error_flag = true;
          err.error(node.line, node.pos, "const expression divide 0");
        }
        break;
      case binop::modulo:
        const_result = L%R;
        break;
      default:
        error_flag = true;
        err.error(node.line, node.pos, "invalid binary operator");
        break;
    }
  }
  else
  {
    node.lhs->accept(*this);
    if(!value_result) return;
    Value* L = value_result;
    node.rhs->accept(*this);
    if(!value_result) return;
    Value* R = value_result;
    switch(node.op)
    {
      case binop::plus:
        value_result = builder.CreateNSWAdd(L, R, "");
        break;
      case binop::minus:
        value_result = builder.CreateNSWSub(L, R, "");
        break;
      case binop::multiply:
        value_result = builder.CreateNSWMul(L, R, "");
        break;
      case binop::divide:
        value_result = builder.CreateSDiv(L, R, "");
        break;
      case binop::modulo:
        value_result = builder.CreateSRem(L, R, "");
        break;
      default:
        error_flag = true;
        err.error(node.line, node.pos, "invalid binary operator");
        value_result = nullptr;
        break;
    }
  }
}

void assembly_builder::visit(unaryop_expr_syntax &node)
{
  if(constexpr_expected)
  {
    node.rhs->accept(*this);
    if(node.op == unaryop::minus)
      const_result = -const_result;
    else if(node.op == unaryop::plus);
    else 
    {
      error_flag = true;
      err.error(node.line, node.pos, "Invalid unary operator");
    }
  }
  else
  {
    node.rhs->accept(*this);
    if(!value_result) return;
    if(node.op == unaryop::minus)
      value_result = builder.CreateNeg(value_result, "", false, true);
    else if(node.op == unaryop::plus);
    else 
    {
      error_flag = true;
      err.error(node.line, node.pos, "Invalid unary operator");
      value_result = nullptr;
    }
  }
}

void assembly_builder::visit(lval_syntax &node)
{
  if(constexpr_expected)
  {
    error_flag = true;
    err.error(node.line, node.pos, node.name+ " is not a compile-time constant");
  }
  else
  {
    auto var = lookup_variable(node.name);
    if(std::get<0>(var))
    {
      //defined as non-array, visit the index
      if(!std::get<2>(var) && node.array_index)
      {
        err.error(node.line, node.pos, "Variable "+node.name+" is not an array");
        value_result = nullptr;
        error_flag = true;
        return;
      }
      if(std::get<2>(var) && !node.array_index)
      {
        err.error(node.line, node.pos, "Variable "+node.name+" is an array, need an index");
        value_result = nullptr;
        error_flag = true;
        return;
      }
      //visit a constant value's lval
      if(lval_as_rval && std::get<1>(var))
      {
        err.error(node.line, node.pos, "Variable "+node.name+ " is a constant, can't be assigned");
        value_result = nullptr;
        error_flag = true;
        return;
      }
      if(node.array_index)
      {
        if(lval_as_rval)
        {
          lval_as_rval = false;
          node.array_index->accept(*this);
          if(!value_result) return;
          //value_result is array size
          value_result = builder.CreateGEP(std::get<0>(var), {ConstantInt::get(Type::getInt32Ty(context), 0), value_result});
        }
        else
        {
          lval_as_rval = false;
          node.array_index->accept(*this);
          if(!value_result) return;
          Value* value = builder.CreateGEP(std::get<0>(var), {ConstantInt::get(Type::getInt32Ty(context), 0), value_result});
          value_result = builder.CreateLoad(value, node.name);
        }
        lval_as_rval = true;
      }
      else
      {
        if(lval_as_rval)
          value_result = std::get<0>(var);
        else
          value_result = builder.CreateLoad(std::get<0>(var), node.name);
      }
    }
    else
    {
      err.error(node.line, node.pos, "undefined variable " + node.name);
      error_flag = true;
      value_result = nullptr;
      return;
    }
  }
}

void assembly_builder::visit(literal_syntax &node)
{
  if (constexpr_expected)
    const_result = node.number;
  else
    value_result = ConstantInt::get(Type::getInt32Ty(context), node.number);
}

void assembly_builder::visit(var_def_stmt_syntax &node)
{
  //global variable
  if(in_global)
  {
    //array variable
    if (node.array_length)
    {
      constexpr_expected = true;
      node.array_length->accept(*this);
      int arr_len = const_result;
      if (arr_len < node.initializers.size())
      {
        error_flag = true;
        err.error(node.line, node.pos, "the array initializer list is too long!");
        return;
      }
      //initializers
      std::vector<Constant*> arr_ref_init;
      for (int i = 0; i < arr_len; i++)
      {
        if(i < node.initializers.size())
        {
          node.initializers[i]->accept(*this);
          arr_ref_init.push_back(ConstantInt::get(Type::getInt32Ty(context), const_result));
        }
        else
        {
          arr_ref_init.push_back(ConstantInt::get(Type::getInt32Ty(context), 0));
        }
      }
      Constant* arr_init = ConstantArray::get(ArrayType::get(Type::getInt32Ty(context), arr_len), arr_ref_init);
      auto var = new GlobalVariable(*(module.get()),
                  ArrayType::get(Type::getInt32Ty(context), arr_len),
                  node.is_constant,
                  GlobalValue::ExternalLinkage,
                  arr_init,
                  node.name);
      constexpr_expected = false;
      if(!declare_variable(node.name, var, node.is_constant, true))
      {
        err.error(node.line, node.pos, "redefinition of global variable: "+node.name);
        error_flag = true;
        return;
      }
    }
    //non-array variable
    else
    {
      int init = 0;
      if(node.initializers.size() != 0)
      {
        constexpr_expected = true;
        node.initializers.front()->accept(*this);
        constexpr_expected = false;
        init = const_result;
      }
      auto var = new GlobalVariable(*(module.get()),
                  Type::getInt32Ty(context), 
                  node.is_constant, 
                  GlobalValue::ExternalLinkage, 
                  ConstantInt::get(Type::getInt32Ty(context), init), 
                  node.name);
      if(!declare_variable(node.name, var, node.is_constant, false))
      {
        err.error(node.line, node.pos, "redefinition of global variable: "+node.name);
        error_flag = true;
        return;
      }
    }
  }
  //local variable
  else
  {
    //array variable
    if (node.array_length)
    {
      constexpr_expected = true;
      node.array_length->accept(*this);
      int arr_len = const_result;
      if (arr_len < node.initializers.size())
      {
        error_flag = true;
        err.error(node.line, node.pos, "the array initializer list is too long!");
        return;
      }
      Value* var = builder.CreateAlloca(
                  ArrayType::get(Type::getInt32Ty(context), arr_len),
                  nullptr,
                  node.name);
      constexpr_expected = false;
      if(declare_variable(node.name, var, node.is_constant, true))
      {
        auto init = node.initializers.begin();
        lval_as_rval = false;
        for(int i = 0; i < arr_len && init != node.initializers.end(); i++, init++)
        {
          Value* elem = builder.CreateGEP(var, {ConstantInt::get(Type::getInt32Ty(context), 0), 
                        ConstantInt::get(Type::getInt32Ty(context), i)});
          node.initializers[i]->accept(*this);
          if(!value_result) return;
          builder.CreateStore(value_result, elem);
        }
        lval_as_rval = true;
      }
      else
      {
        err.error(node.line, node.pos, "redefinition of variable: "+node.name);
        error_flag = true;
        return;
      }
    }
    //non-array variable
    else
    {
      Value* var = builder.CreateAlloca(Type::getInt32Ty(context), nullptr, node.name);
      if(declare_variable(node.name, var, node.is_constant, false))
      {
        if(node.initializers.size() != 0)
        {
          lval_as_rval = false;
          node.initializers.front()->accept(*this);
          if(!value_result) return;
          builder.CreateStore(value_result,var,false);
        }
      }
      else
      {
        err.error(node.line, node.pos, "redefinition of variable: "+node.name);
        error_flag = true;
        return;
      }
    }
  }
}

void assembly_builder::visit(assign_stmt_syntax &node)
{
  lval_as_rval = true;
  node.target->accept(*this);
  if(!value_result) return;
  Value* target = value_result;
  lval_as_rval = false;
  node.value->accept(*this);
  if(!value_result) return;
  builder.CreateStore(value_result, target);
}

void assembly_builder::visit(func_call_stmt_syntax &node)
{
  if(functions.count(node.name))
  {
    auto pfunc = functions[node.name];
    builder.CreateCall(pfunc, std::vector<Value*>{}, node.name);
  }
  else
  {
    error_flag = true;
    err.error(node.line, node.pos, "function "+node.name+" undefined.");
    return;
  }
}

void assembly_builder::visit(block_syntax &node)
{
  in_global = false;
  enter_scope();
  for (auto &stmt : node.body)
    stmt->accept(*this);
  exit_scope();
}

void assembly_builder::visit(if_stmt_syntax &node)
{
  BasicBlock* then_bb = BasicBlock::Create(context, "L", current_function);
  BasicBlock* else_bb = BasicBlock::Create(context, "L", current_function);
  BasicBlock* end_bb = BasicBlock::Create(context, "L", current_function);
  node.pred->accept(*this);
  //no else body
  if(!node.else_body)
  {
    builder.CreateCondBr(value_result, then_bb, end_bb);
    builder.SetInsertPoint(then_bb);
    node.then_body->accept(*this);
  }
  //has else body
  else
  {
    builder.CreateCondBr(value_result, then_bb, else_bb);
    builder.SetInsertPoint(then_bb);
    node.then_body->accept(*this);
    builder.CreateBr(end_bb);
    builder.SetInsertPoint(else_bb);
    node.else_body->accept(*this);
  }
  builder.CreateBr(end_bb);
  builder.SetInsertPoint(end_bb);
}

void assembly_builder::visit(while_stmt_syntax &node)
{
  BasicBlock* cond_bb = BasicBlock::Create(context, "L", current_function);
  BasicBlock* body_bb = BasicBlock::Create(context, "L", current_function);
  BasicBlock* end_bb = BasicBlock::Create(context, "L", current_function);
  builder.CreateBr(cond_bb);
  builder.SetInsertPoint(cond_bb);
  node.pred->accept(*this);
  builder.CreateCondBr(value_result, body_bb, end_bb);
  builder.SetInsertPoint(body_bb);
  node.body->accept(*this);
  builder.CreateBr(cond_bb);
  builder.SetInsertPoint(end_bb);
}

void assembly_builder::visit(empty_stmt_syntax &node)
{
}
