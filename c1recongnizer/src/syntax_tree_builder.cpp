
#include "syntax_tree_builder.h"
#include <memory>
#include <iostream>

using namespace c1_recognizer;
using namespace c1_recognizer::syntax_tree;

syntax_tree_builder::syntax_tree_builder(error_reporter &_err) : err(_err) {}

antlrcpp::Any syntax_tree_builder::visitCompilationUnit(C1Parser::CompilationUnitContext *ctx)
{
    auto result = new assembly;
    result->pos = ctx->getStart()->getCharPositionInLine();
    result->line = ctx->getStart()->getLine();
    for (auto &child : ctx->children)
    {
        if (antlrcpp::is<C1Parser::DeclContext*>(child))
        {
            auto list = visit(child).as<std::vector<var_def_stmt_syntax*>>();
            for (auto &decl : list)
                result->global_defs.push_back(ptr<var_def_stmt_syntax>(decl));
        }
        else if (antlrcpp::is<C1Parser::FuncdefContext*>(child))
        {
            auto temp = dynamic_cast<C1Parser::FuncdefContext*>(child);
            auto func = visit(temp).as<func_def_syntax*>();
            result->global_defs.push_back(ptr<func_def_syntax>(func));
        }
    }
    return result;
}

antlrcpp::Any syntax_tree_builder::visitDecl(C1Parser::DeclContext *ctx)
{
    //const declaration
    if (auto constdecl = ctx->constdecl())
    {
        auto result = visit(constdecl);
        return result;
    }
    //variable declaration
    else 
    {   
        auto vardecl = ctx->vardecl();
        auto result = visit(vardecl);
        return result;
    }
}

antlrcpp::Any syntax_tree_builder::visitConstdecl(C1Parser::ConstdeclContext *ctx)
{
    std::vector<var_def_stmt_syntax*> decl;
    auto constdefs = ctx->constdef();
    for (auto def : constdefs)
        decl.push_back(visit(def).as<var_def_stmt_syntax*>());
    return decl;
}

antlrcpp::Any syntax_tree_builder::visitConstdef(C1Parser::ConstdefContext *ctx)
{
    auto result = new var_def_stmt_syntax;
    result->is_constant = true;
    auto id = ctx->Identifier();
    result->pos = id->getSymbol()->getCharPositionInLine();
    result->line = id->getSymbol()->getLine();
    result->name = id->getSymbol()->getText();
    //const array variable declaration
    if (ctx->LeftBracket())
    {
        auto exps = ctx->exp();
        int ninit = ctx->Comma().size() + 1;
        //explicitly declare the array length
        if (exps.size() == ninit+1)
        {
            result->array_length.reset(visit(exps[0]).as<expr_syntax*>());
            size_t i = 1;
            for (i=1; i < exps.size(); i++)
            {
                auto expression = visit(exps[i]).as<expr_syntax*>();
                result->initializers.push_back(ptr<expr_syntax>(expression));
            }
        }
        //not explicitly declare the array length
        else
        {
            for(auto init : exps)
            {
                auto expression = visit(init).as<expr_syntax*>();
                result->initializers.push_back(ptr<expr_syntax>(expression));
            }
            auto length = new literal_syntax;
            length->number = ninit;
            length->line = ctx->RightBracket()->getSymbol()->getLine();
            length->pos = ctx->RightBracket()->getSymbol()->getCharPositionInLine();
            result->array_length.reset(static_cast<expr_syntax*>(length));
        }
    }
    //const variable declaration
    else
    {
        result->array_length.reset();
        auto expression = visit(ctx->exp(0)).as<expr_syntax*>();
        result->initializers.push_back(ptr<expr_syntax>(expression));
    }
    return result;
}

antlrcpp::Any syntax_tree_builder::visitVardecl(C1Parser::VardeclContext *ctx)
{
    //multi variable declaration
    std::vector<var_def_stmt_syntax*> decl;
    auto defs = ctx->vardef();
    for (auto def : defs)
        decl.push_back(visit(def).as<var_def_stmt_syntax*>());
    return decl;
}

antlrcpp::Any syntax_tree_builder::visitVardef(C1Parser::VardefContext *ctx)
{
    auto result = new var_def_stmt_syntax;
    result->is_constant = false;
    auto id = ctx->Identifier();
    result->name = id->getSymbol()->getText();
    result->pos = id->getSymbol()->getCharPositionInLine();
    result->line = id->getSymbol()->getLine();
    result->array_length.reset();
    //non-array variable declaration
    if (!ctx->LeftBracket())
    {
        if (ctx->Assign())
        {
            auto expression = visit(ctx->exp(0)).as<expr_syntax*>();
            result->initializers.push_back(ptr<expr_syntax>(expression));
        }
    }
    else
    {
        if (ctx->Assign())
        {
            auto exps = ctx->exp();
            int ninit = ctx->Comma().size() + 1;
            //explicitly declare the array length
            if (exps.size() == ninit+1)
            {
                result->array_length.reset(visit(exps[0]).as<expr_syntax*>());
                size_t i = 1;
                for (i=1; i < exps.size(); i++)
                {
                    auto expression = visit(exps[i]).as<expr_syntax*>();
                    result->initializers.push_back(ptr<expr_syntax>(expression));
                }
            }
            //not explicitly declare the array length
            else
            {
                for(auto init : exps)
                {
                    auto expression = visit(init).as<expr_syntax*>();
                    result->initializers.push_back(ptr<expr_syntax>(expression));
                }
                auto length = new literal_syntax;
                length->number = ninit;
                length->line = ctx->RightBracket()->getSymbol()->getLine();
                length->pos = ctx->RightBracket()->getSymbol()->getCharPositionInLine();    
                result->array_length.reset(static_cast<expr_syntax*>(length));
            }
        }
        else
        {
            auto expression = ctx->exp(0);
            result->array_length.reset(visit(expression).as<expr_syntax*>());
        }
    }
    return result;
}

antlrcpp::Any syntax_tree_builder::visitFuncdef(C1Parser::FuncdefContext *ctx)
{
    auto result = new func_def_syntax;
    auto block = ctx->block();
    result->name = ctx->Identifier()->getSymbol()->getText();
    result->line = ctx->getStart()->getLine();
    result->pos = ctx->getStart()->getCharPositionInLine();
    result->body.reset(visit(block).as<block_syntax*>());
    return result;
}

antlrcpp::Any syntax_tree_builder::visitBlock(C1Parser::BlockContext *ctx)
{
    auto result = new block_syntax;
    result->line = ctx->getStart()->getLine();
    result->pos = ctx->getStart()->getCharPositionInLine();
    for (auto child : ctx->children)
    {
        if (antlrcpp::is<C1Parser::DeclContext*>(child))
        {
            auto dec = dynamic_cast<C1Parser::DeclContext*>(child);
            auto list = visit(dec).as<std::vector<var_def_stmt_syntax*>>();
            for (auto decl : list)
            {
                auto stmt = static_cast<stmt_syntax*>(decl);
                result->body.push_back(ptr<stmt_syntax>(stmt));
            }
        }
        else if (antlrcpp::is<C1Parser::StmtContext*>(child))
        {
            auto temp = dynamic_cast<C1Parser::StmtContext*>(child);
            auto stmt = visit(temp).as<stmt_syntax*>();
            result->body.push_back(ptr<stmt_syntax>(stmt));
        }
    }
    return result;
}

antlrcpp::Any syntax_tree_builder::visitStmt(C1Parser::StmtContext *ctx)
{
    if (ctx->Assign())
    {
        auto result = new assign_stmt_syntax;
        result->line = ctx->getStart()->getLine();
        result->pos = ctx->getStart()->getCharPositionInLine();
        result->target.reset(visit(ctx->lval()).as<lval_syntax*>());
        result->value.reset(visit(ctx->exp()).as<expr_syntax*>());
        return static_cast<stmt_syntax*>(result); 
    }
    if (ctx->Identifier())
    {
        auto result = new func_call_stmt_syntax;
        result->line = ctx->getStart()->getLine();
        result->pos = ctx->getStart()->getCharPositionInLine();
        auto func = ctx->Identifier();
        result->name = func->getSymbol()->getText();
        return static_cast<stmt_syntax*>(result);
    }
    if (auto block = ctx->block())
    {
        auto result = visit(block).as<block_syntax*>();
        return static_cast<stmt_syntax*>(result);
    }
    if (ctx->If())
    {
        auto result = new if_stmt_syntax;
        result->pred.reset(visit(ctx->cond()).as<cond_syntax*>());
        result->line = ctx->getStart()->getLine();
        result->pos = ctx->getStart()->getCharPositionInLine();
        auto stmt = ctx->stmt();
        result->then_body.reset(visit(stmt[0]).as<stmt_syntax*>());
        result->else_body.reset();
        if (stmt.size() == 2)
            result->else_body.reset(visit(stmt[1]).as<stmt_syntax*>());
        return static_cast<stmt_syntax*>(result);
    }
    if (ctx->While())
    {
        auto result = new while_stmt_syntax;
        result->line = ctx->getStart()->getLine();
        result->pos = ctx->getStart()->getCharPositionInLine();
        result->pred.reset(visit(ctx->cond()).as<cond_syntax*>());
        result->body.reset(visit(ctx->stmt(0)).as<stmt_syntax*>());
        return static_cast<stmt_syntax*>(result);
    }
    else
    {
        auto result = new empty_stmt_syntax;
        result->line = ctx->getStart()->getLine();
        result->pos = ctx->getStart()->getCharPositionInLine();
        return static_cast<stmt_syntax*>(result);
    }
}

antlrcpp::Any syntax_tree_builder::visitLval(C1Parser::LvalContext *ctx)
{
    auto id = ctx->Identifier();
    auto result = new lval_syntax;
    result->line = id->getSymbol()->getLine();
    result->pos = id->getSymbol()->getCharPositionInLine();
    result->name = id->getSymbol()->getText();
    result->array_index.reset();
    if (auto expression = ctx->exp())
        result->array_index.reset(visit(expression).as<expr_syntax *>());
    return result;
}

antlrcpp::Any syntax_tree_builder::visitCond(C1Parser::CondContext *ctx)
{
    auto result = new cond_syntax;
    auto exps = ctx->exp();
    result->line = ctx->getStart()->getLine();
    result->pos = ctx->getStart()->getCharPositionInLine();
    result->lhs.reset(visit(exps[0]).as<expr_syntax*>());
    result->rhs.reset(visit(exps[1]).as<expr_syntax*>());
    if (ctx->Greater())
        result->op = relop::greater;
    if (ctx->Less())
        result->op = relop::less;
    if (ctx->GreaterEqual())
        result->op = relop::greater_equal;
    if (ctx->LessEqual())
        result->op = relop::less_equal;
    if (ctx->Equal())
        result->op = relop::equal;
    if (ctx->NonEqual())
        result->op = relop::non_equal;
    return result;
}

// Returns antlrcpp::Any, which is constructable from any type.
// However, you should be sure you use the same type for packing and depacking the `Any` object.
// Or a std::bad_cast exception will rise.
// This function always returns an `Any` object containing a `expr_syntax *`.
antlrcpp::Any syntax_tree_builder::visitExp(C1Parser::ExpContext *ctx)
{
    // Get all sub-contexts of type `exp`.
    auto expressions = ctx->exp();
    // Two sub-expressions presented: this indicates it's a expression of binary operator, aka `binop`.
    if (expressions.size() == 2)
    {
        auto result = new binop_expr_syntax;
        // Set line and pos.
        result->line = ctx->getStart()->getLine();
        result->pos = ctx->getStart()->getCharPositionInLine();
        // visit(some context) is equivalent to calling corresponding visit method; dispatching is done automatically
        // by ANTLR4 runtime. For this case, it's equivalent to visitExp(expressions[0]).
        // Use reset to set a new pointer to a std::shared_ptr object. DO NOT use assignment; it won't work.
        // Use `.as<Type>()' to get value from antlrcpp::Any object; notice that this Type must match the type used in
        // constructing the Any object, which is constructed from (usually pointer to some derived class of
        // syntax_node, in this case) returning value of the visit call.
        result->lhs.reset(visit(expressions[0]).as<expr_syntax *>());
        // Check if each token exists.
        // Returnd value of the calling will be nullptr (aka NULL in C) if it isn't there; otherwise non-null pointer.
        if (ctx->Plus())
            result->op = binop::plus;
        if (ctx->Minus())
            result->op = binop::minus;
        if (ctx->Multiply())
            result->op = binop::multiply;
        if (ctx->Divide())
            result->op = binop::divide;
        if (ctx->Modulo())
            result->op = binop::modulo;
        result->rhs.reset(visit(expressions[1]).as<expr_syntax *>());
        return static_cast<expr_syntax *>(result);
    }
    // Otherwise, if `+` or `-` presented, it'll be a `unaryop_expr_syntax`.
    if (ctx->Plus() || ctx->Minus())
    {
        auto result = new unaryop_expr_syntax;
        result->line = ctx->getStart()->getLine();
        result->pos = ctx->getStart()->getCharPositionInLine();
        if (ctx->Plus())
            result->op = unaryop::plus;
        if (ctx->Minus())
            result->op = unaryop::minus;
        result->rhs.reset(visit(expressions[0]).as<expr_syntax *>());
        return static_cast<expr_syntax *>(result);
    }
    // In the case that `(` exists as a child, this is an expression like `'(' expressions[0] ')'`.
    if (ctx->LeftParen())
        return visit(expressions[0]); // Any already holds expr_syntax* here, no need for dispatch and re-patch with casting.
    // If `Number` exists as a child, we can say it's a literal integer expression.
    if (auto lval = ctx->lval())
    {
        auto result = visit(lval).as<lval_syntax*>();
        return static_cast<expr_syntax *>(result);
    }
    else
    {
        auto number = ctx->Number();
        auto result = new literal_syntax;
        result->line = number->getSymbol()->getLine();
        result->pos = number->getSymbol()->getCharPositionInLine();
        auto text = number->getSymbol()->getText();
        if (text[0] == '0' && text[1] == 'x')              // Hexadecimal
            result->number = std::stoi(text, nullptr, 16); // std::stoi will eat '0x'
        else                                               // Decimal
            result->number = std::stoi(text, nullptr, 10);
        return static_cast<expr_syntax *>(result);
    }
}

ptr<syntax_tree_node> syntax_tree_builder::operator()(antlr4::tree::ParseTree *ctx)
{
    auto result = visit(ctx);
    if (result.is<syntax_tree_node *>())
        return ptr<syntax_tree_node>(result.as<syntax_tree_node *>());
    if (result.is<assembly *>())
        return ptr<syntax_tree_node>(result.as<assembly *>());
    if (result.is<global_def_syntax *>())
        return ptr<syntax_tree_node>(result.as<global_def_syntax *>());
    if (result.is<func_def_syntax *>())
        return ptr<syntax_tree_node>(result.as<func_def_syntax *>());
    if (result.is<cond_syntax *>())
        return ptr<syntax_tree_node>(result.as<cond_syntax *>());
    if (result.is<expr_syntax *>())
        return ptr<syntax_tree_node>(result.as<expr_syntax *>());
    if (result.is<binop_expr_syntax *>())
        return ptr<syntax_tree_node>(result.as<binop_expr_syntax *>());
    if (result.is<unaryop_expr_syntax *>())
        return ptr<syntax_tree_node>(result.as<unaryop_expr_syntax *>());
    if (result.is<lval_syntax *>())
        return ptr<syntax_tree_node>(result.as<lval_syntax *>());
    if (result.is<literal_syntax *>())
        return ptr<syntax_tree_node>(result.as<literal_syntax *>());
    if (result.is<stmt_syntax *>())
        return ptr<syntax_tree_node>(result.as<stmt_syntax *>());
    if (result.is<var_def_stmt_syntax *>())
        return ptr<syntax_tree_node>(result.as<var_def_stmt_syntax *>());
    if (result.is<assign_stmt_syntax *>())
        return ptr<syntax_tree_node>(result.as<assign_stmt_syntax *>());
    if (result.is<func_call_stmt_syntax *>())
        return ptr<syntax_tree_node>(result.as<func_call_stmt_syntax *>());
    if (result.is<block_syntax *>())
        return ptr<syntax_tree_node>(result.as<block_syntax *>());
    if (result.is<if_stmt_syntax *>())
        return ptr<syntax_tree_node>(result.as<if_stmt_syntax *>());
    if (result.is<while_stmt_syntax *>())
        return ptr<syntax_tree_node>(result.as<while_stmt_syntax *>());
    return nullptr;
}
