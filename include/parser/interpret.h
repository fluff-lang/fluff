#pragma once
#ifndef FLUFF_PARSER_INTERPRETER_H
#define FLUFF_PARSER_INTERPRETER_H

/* -=============
     Includes
   =============- */

#include <base.h>
#include <core/string.h>
#include <parser/lexer.h>
#include <parser/ast.h>

/* -================
     Interpreter
   ================- */

typedef struct FluffModule FluffModule;

typedef struct FluffInterpreter {
    const char * path;

    FluffModule * module;

    AST ast;
} FluffInterpreter;

FLUFF_API FluffInterpreter * fluff_new_interpreter(FluffModule * module);
FLUFF_API void               fluff_free_interpreter(FluffInterpreter * self);

FLUFF_API FluffResult fluff_interpreter_read_string(FluffInterpreter * self, const char * source);
FLUFF_API FluffResult fluff_interpreter_read_file(FluffInterpreter * self, const char * path);

FLUFF_PRIVATE_API FluffResult fluff_interpreter_read(FluffInterpreter * self, const char * source, size_t n);

#endif