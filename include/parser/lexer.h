#pragma once
#ifndef FLUFF_CORE_LEXER_H
#define FLUFF_CORE_LEXER_H

/* -=============
     Includes
   =============- */

#include <base.h>
#include <parser/text.h>

/* -===========
     Macros
   ===========- */

#define _make_token(__type) ((Token){ .type = __type, .data = { 0 } })

/* -==========
     Token
   ==========- */

typedef enum TokenCategory {
    TOKEN_CATEGORY_NONE, 
    
    TOKEN_CATEGORY_BOOL_LITERAL, 
    TOKEN_CATEGORY_INTEGER_LITERAL, 
    TOKEN_CATEGORY_DECIMAL_LITERAL, 
    TOKEN_CATEGORY_STRING_LITERAL, 
    TOKEN_CATEGORY_LABEL_LITERAL, 

    TOKEN_CATEGORY_LPAREN, 
    TOKEN_CATEGORY_RPAREN, 
    TOKEN_CATEGORY_LBRACE, 
    TOKEN_CATEGORY_RBRACE, 
    TOKEN_CATEGORY_LBRACKET, 
    TOKEN_CATEGORY_RBRACKET, 

    TOKEN_CATEGORY_OPERATOR, 
    TOKEN_CATEGORY_OPERATOR_ARROW, 
    TOKEN_CATEGORY_OPERATOR_DOT, 
    TOKEN_CATEGORY_OPERATOR_ELLIPSIS, 
    TOKEN_CATEGORY_OPERATOR_COMMA, 
    TOKEN_CATEGORY_CONTROL_FLOW, 
    TOKEN_CATEGORY_DECL, 
    TOKEN_CATEGORY_FUNC_DECL, 
    TOKEN_CATEGORY_CLASS_DECL, 
    TOKEN_CATEGORY_END, 
} TokenCategory;

FLUFF_PRIVATE_API const char * _token_category_string(TokenCategory category);

typedef enum TokenType {
    TOKEN_NONE, 
    
    TOKEN_INTEGER_LITERAL, 
    TOKEN_DECIMAL_LITERAL, 
    TOKEN_STRING_LITERAL, 
    TOKEN_LABEL_LITERAL, 

    TOKEN_LPAREN, 
    TOKEN_RPAREN, 
    TOKEN_LBRACE, 
    TOKEN_RBRACE, 
    TOKEN_LBRACKET, 
    TOKEN_RBRACKET, 

    TOKEN_EQUAL, 
    TOKEN_PLUS, 
    TOKEN_MINUS, 
    TOKEN_MULTIPLY, 
    TOKEN_DIVIDE, 
    TOKEN_MODULO, 
    TOKEN_POWER, 
    TOKEN_COLON, 
    TOKEN_COMMA, 
    TOKEN_DOT, 
    TOKEN_ARROW, 
    TOKEN_ELLIPSIS, 

    TOKEN_EQUALS, 
    TOKEN_NOT_EQUALS, 
    TOKEN_LESS, 
    TOKEN_LESS_EQUALS, 
    TOKEN_GREATER, 
    TOKEN_GREATER_EQUALS, 

    TOKEN_AND, 
    TOKEN_OR, 
    TOKEN_NOT, 
    TOKEN_BIT_AND, 
    TOKEN_BIT_OR, 
    TOKEN_BIT_XOR, 
    TOKEN_BIT_NOT, 
    TOKEN_BIT_SHL, 
    TOKEN_BIT_SHR, 

    TOKEN_IF, 
    TOKEN_ELSE, 
    TOKEN_FOR, 
    TOKEN_WHILE, 
    TOKEN_IN, 

    TOKEN_AS, 
    TOKEN_IS, 

    TOKEN_LET, 
    TOKEN_CONST, 
    TOKEN_FUNC, 
    TOKEN_CLASS, 

    TOKEN_TRUE, 
    TOKEN_FALSE, 
    TOKEN_BOOL, 
    TOKEN_INT, 
    TOKEN_FLOAT, 
    TOKEN_STRING, 
    TOKEN_ARRAY, 
    TOKEN_OBJECT, 

    TOKEN_END, 
} TokenType;

FLUFF_PRIVATE_API const char *  _token_type_string(TokenType type);
FLUFF_PRIVATE_API TokenCategory _token_type_get_category(TokenType type);

typedef struct Token {
    TokenType type;
    
    TextSect start;
    size_t   length;

    union {
        FluffInt   i;
        FluffFloat f;
        FluffBool  b;
    } data;
} Token;

/* -==========
     Lexer
   ==========- */

typedef struct FluffInterpreter FluffInterpreter;

typedef struct Lexer {
    FluffInterpreter * interpret;
    
    const char * str;
    size_t       len;

    Token * tokens;
    size_t  token_count;

    TextSect prev_location;
    TextSect location;
} Lexer;

FLUFF_PRIVATE_API void _new_lexer(Lexer * self, FluffInterpreter * interpret, const char * str, size_t len);
FLUFF_PRIVATE_API void _free_lexer(Lexer * self);

FLUFF_PRIVATE_API void _lexer_parse(Lexer * self);
FLUFF_PRIVATE_API void _lexer_parse_comment(Lexer * self);
FLUFF_PRIVATE_API void _lexer_parse_number(Lexer * self);
FLUFF_PRIVATE_API void _lexer_parse_label(Lexer * self);
FLUFF_PRIVATE_API void _lexer_parse_string(Lexer * self);
FLUFF_PRIVATE_API void _lexer_parse_operator(Lexer * self);

FLUFF_PRIVATE_API void _lexer_read_long_operator(Lexer * self, Token * token, char prev_ch);
FLUFF_PRIVATE_API void _lexer_read_decimal(Lexer * self, Token * token);
FLUFF_PRIVATE_API void _lexer_read_hexadecimal(Lexer * self, Token * token);
FLUFF_PRIVATE_API void _lexer_read_octal(Lexer * self, Token * token);
FLUFF_PRIVATE_API void _lexer_read_binary(Lexer * self, Token * token);

FLUFF_PRIVATE_API void _lexer_pop(Lexer * self);
FLUFF_PRIVATE_API void _lexer_push(Lexer * self, Token token);

FLUFF_PRIVATE_API void _lexer_digest(Lexer * self);
FLUFF_PRIVATE_API void _lexer_consume(Lexer * self, size_t n);
FLUFF_PRIVATE_API void _lexer_rewind(Lexer * self, size_t n);
FLUFF_PRIVATE_API char _lexer_peek(Lexer * self, int offset);
FLUFF_PRIVATE_API char _lexer_peekp(Lexer * self, size_t index);
FLUFF_PRIVATE_API bool _lexer_is_within_bounds(Lexer * self);
FLUFF_PRIVATE_API char _lexer_current_char(Lexer * self);

FLUFF_PRIVATE_API const char * _lexer_token_string(Lexer * self, size_t index);
FLUFF_PRIVATE_API size_t       _lexer_token_string_len(Lexer * self, size_t index);

FLUFF_PRIVATE_API void _lexer_dump(Lexer * self);

#endif