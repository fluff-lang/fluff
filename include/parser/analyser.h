#pragma once
#ifndef FLUFF_CORE_ANALYSER_H
#define FLUFF_CORE_ANALYSER_H

/* -=============
     Includes
   =============- */

#include <base.h>
#include <parser/text.h>
#include <parser/lexer.h>
#include <parser/ast.h>

/* -=============
     Analyser
   =============- */

typedef struct AnalyserState {
    ASTNode * last_scope;

    TokenType expect;

    bool   in_call;
    bool   in_subscript;
    bool   in_statement;
    bool   in_paren;
    bool   in_array;
    bool   in_decl;
    bool   in_expr;
    size_t comma_count;
    size_t call_depth;
} AnalyserState;

typedef struct FluffInterpreter FluffInterpreter;

typedef struct Analyser {
    FluffInterpreter * interpret;

    AST   * ast;
    Lexer * lexer;

    AnalyserState state;
    ASTNode     * extra_return;

    FluffResult result;

    Token    token;
    size_t   index;
    TextSect position;
} Analyser;

FLUFF_PRIVATE_API void _new_analyser(Analyser * self, Lexer * lexer);
FLUFF_PRIVATE_API void _free_analyser(Analyser * self);

FLUFF_PRIVATE_API FluffResult _analyser_read(Analyser * self);
FLUFF_PRIVATE_API ASTNode   * _analyser_read_token(Analyser * self);
FLUFF_PRIVATE_API ASTNode   * _analyser_read_scope(Analyser * self);
FLUFF_PRIVATE_API ASTNode   * _analyser_read_if(Analyser * self);
FLUFF_PRIVATE_API ASTNode   * _analyser_read_for(Analyser * self);
FLUFF_PRIVATE_API ASTNode   * _analyser_read_while(Analyser * self);
FLUFF_PRIVATE_API ASTNode   * _analyser_read_decl(Analyser * self);
FLUFF_PRIVATE_API ASTNode   * _analyser_read_func(Analyser * self);
FLUFF_PRIVATE_API ASTNode   * _analyser_read_class(Analyser * self);
FLUFF_PRIVATE_API ASTNode   * _analyser_read_control(Analyser * self);
FLUFF_PRIVATE_API ASTNode   * _analyser_read_array(Analyser * self, TokenType expect);
FLUFF_PRIVATE_API ASTNode   * _analyser_read_type(Analyser * self, TokenType expect);
FLUFF_PRIVATE_API ASTNode   * _analyser_read_expr(Analyser * self, TokenType expect);
FLUFF_PRIVATE_API ASTNode   * _analyser_read_expr_pratt(Analyser * self, int prec_limit);
FLUFF_PRIVATE_API ASTNode   * _analyser_read_expr_call(Analyser * self, ASTNode * top);
FLUFF_PRIVATE_API ASTNode   * _analyser_read_expr_subscript(Analyser * self, ASTNode * top);

FLUFF_PRIVATE_API bool  _analyser_expect_n(Analyser * self, size_t idx, const TokenCategory * categories, size_t count, int line, const char * func);
FLUFF_PRIVATE_API Token _analyser_consume(Analyser * self, size_t n);
FLUFF_PRIVATE_API Token _analyser_pconsume(Analyser * self, size_t n);
FLUFF_PRIVATE_API Token _analyser_rewind(Analyser * self, size_t n);
FLUFF_PRIVATE_API Token _analyser_prewind(Analyser * self, size_t n);
FLUFF_PRIVATE_API Token _analyser_peek(Analyser * self, int offset);
FLUFF_PRIVATE_API Token _analyser_peekp(Analyser * self, size_t p);
FLUFF_PRIVATE_API bool  _analyser_is_eof(Analyser * self);
FLUFF_PRIVATE_API bool  _analyser_is_expr_end(Analyser * self);
FLUFF_PRIVATE_API bool  _analyser_is_within_bounds(Analyser * self);

#endif