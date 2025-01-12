#pragma once
#ifndef FLUFF_PARSER_CODEGEN_H
#define FLUFF_PARSER_CODEGEN_H

/* -=============
     Includes
   =============- */

#include <base.h>
#include <parser/text.h>
#include <parser/lexer.h>
#include <core/ir.h>

/* -==============
     Generator
   ==============- */

typedef enum GeneratorType {
    GENERATOR_NONE, 

    GENERATOR_LITERAL, 
    GENERATOR_EXPR, 
} GeneratorType;

typedef struct Generator Generator;

typedef struct GeneratorLiteral {

} GeneratorLiteral;

typedef struct Generator {
    Generator * parent;

    IRChunk chunk;
} Generator;

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

FLUFF_PRIVATE_API void _new_generator(Analyser * self, Lexer * lexer);
FLUFF_PRIVATE_API void _free_generator(Analyser * self);

#endif