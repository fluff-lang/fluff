/* -=============
     Includes
   =============- */

#define FLUFF_IMPLEMENTATION
#include <base.h>
#include <parser/analyser.h>
#include <parser/interpret.h>
#include <parser/lexer.h>
#include <core/config.h>

/* -==============
     Internals
   ==============- */

FLUFF_CONSTEXPR_V ASTNode * _new_ast_node_from_token(Lexer * self, size_t i, AST * ast) {
    switch (self->tokens[i].type) {
        case TOKEN_TRUE: case TOKEN_FALSE: 
            return _new_ast_node_bool(ast, self->tokens[i].data.b);
        case TOKEN_INTEGER_LITERAL:
            return _new_ast_node_int(ast, self->tokens[i].data.i);
        case TOKEN_DECIMAL_LITERAL:
            return _new_ast_node_float(ast, self->tokens[i].data.f);
        case TOKEN_STRING_LITERAL:
        case TOKEN_LABEL_LITERAL:
            return _new_ast_node_string_n(ast, _lexer_token_string(self, i), _lexer_token_string_len(self, i));
        default: break;
    }
    return NULL;
}

FLUFF_CONSTEXPR_V struct {
    int  infix_prec;
    int  prefix_prec;
    bool right_associative;

    ASTOperatorDataType      op;
    ASTUnaryOperatorDataType unary_op;
} token_info[] = {
    // NOTE: yes I copied half of these from C++ standard, how did you know
    [TOKEN_DOT] = { 0, 0, false, AST_OPERATOR_DOT,  AST_UOPERATOR_NONE },

    [TOKEN_NOT] = { 1, 0, true, AST_OPERATOR_NONE, AST_UOPERATOR_NOT },

    [TOKEN_BIT_NOT]  = { 1,  1, false, AST_OPERATOR_NONE, AST_UOPERATOR_BIT_NOT },
    [TOKEN_PLUS]     = { 2,  2, false, AST_OPERATOR_ADD,  AST_UOPERATOR_PROMOTE },
    [TOKEN_MINUS]    = { 2,  2, false, AST_OPERATOR_SUB,  AST_UOPERATOR_NEGATE  },
    [TOKEN_MULTIPLY] = { 3,  0, false, AST_OPERATOR_MUL,  AST_UOPERATOR_NONE    },
    [TOKEN_DIVIDE]   = { 3,  0, false, AST_OPERATOR_DIV,  AST_UOPERATOR_NONE    },
    [TOKEN_MODULO]   = { 4,  0, false, AST_OPERATOR_MOD,  AST_UOPERATOR_NONE    },

    [TOKEN_POWER] = { 4, 0, true, AST_OPERATOR_POW, AST_UOPERATOR_NONE },

    [TOKEN_BIT_SHL]        = {  5, 0, false, AST_OPERATOR_BIT_SHL, AST_UOPERATOR_NONE },
    [TOKEN_BIT_SHR]        = {  5, 0, false, AST_OPERATOR_BIT_SHR, AST_UOPERATOR_NONE },
    [TOKEN_LESS]           = {  6, 0, false, AST_OPERATOR_LT,      AST_UOPERATOR_NONE },
    [TOKEN_LESS_EQUALS]    = {  6, 0, false, AST_OPERATOR_LE,      AST_UOPERATOR_NONE },
    [TOKEN_GREATER]        = {  6, 0, false, AST_OPERATOR_GT,      AST_UOPERATOR_NONE },
    [TOKEN_GREATER_EQUALS] = {  6, 0, false, AST_OPERATOR_GE,      AST_UOPERATOR_NONE },
    [TOKEN_EQUALS]         = {  7, 0, false, AST_OPERATOR_EQ,      AST_UOPERATOR_NONE },
    [TOKEN_NOT_EQUALS]     = {  7, 0, false, AST_OPERATOR_NE,      AST_UOPERATOR_NONE },
    [TOKEN_BIT_AND]        = {  8, 0, false, AST_OPERATOR_BIT_AND, AST_UOPERATOR_NONE },
    [TOKEN_BIT_XOR]        = {  9, 0, false, AST_OPERATOR_BIT_XOR, AST_UOPERATOR_NONE },
    [TOKEN_BIT_OR]         = { 10, 0, false, AST_OPERATOR_BIT_OR,  AST_UOPERATOR_NONE },
    [TOKEN_AND]            = { 11, 0, false, AST_OPERATOR_AND,     AST_UOPERATOR_NONE },
    [TOKEN_OR]             = { 12, 0, false, AST_OPERATOR_OR,      AST_UOPERATOR_NONE },

    [TOKEN_EQUAL] = { 13, 0, true, AST_OPERATOR_DOT, AST_UOPERATOR_NONE },

    [TOKEN_COMMA]    = { 13, 0, false, AST_OPERATOR_DOT, AST_UOPERATOR_NONE },
    [TOKEN_COLON]    = { 1,  0, false, AST_OPERATOR_DOT, AST_UOPERATOR_NONE },
    [TOKEN_ARROW]    = { 1,  0, false, AST_OPERATOR_DOT, AST_UOPERATOR_NONE },
    [TOKEN_ELLIPSIS] = { 1,  0, false, AST_OPERATOR_DOT, AST_UOPERATOR_NONE },
};

FLUFF_CONSTEXPR_V TokenCategory literal_categories[] = {
    TOKEN_CATEGORY_BOOL_LITERAL, 
    TOKEN_CATEGORY_INTEGER_LITERAL, 
    TOKEN_CATEGORY_DECIMAL_LITERAL, 
    TOKEN_CATEGORY_STRING_LITERAL, 
    TOKEN_CATEGORY_LABEL_LITERAL, 
};

/* -=============
     Analyser
   =============- */

FLUFF_PRIVATE_API void _new_analyser(Analyser * self, Lexer * lexer) {
    FLUFF_CLEANUP(self);
    self->lexer = lexer;
    self->ast   = &lexer->interpret->ast;
}

FLUFF_PRIVATE_API void _free_analyser(Analyser * self) {
    FLUFF_CLEANUP(self);
}

// TODO: make ErrorPool class for supporting multiple errors
#define _analyser_error(__fmt, ...)\
        fluff_error_fmt(FLUFF_COMPILE_ERROR, "%s:%zu:%zu: " __fmt, \
            self->lexer->interpret->path, \
            self->current_token->start.line + 1, self->current_token->start.column + 1, ##__VA_ARGS__\
        )
#define _analyser_error_recoverable(__fmt, ...)\
        fluff_error_fmt(FLUFF_COMPILE_ERROR, "%s:%zu:%zu: " __fmt, \
            self->lexer->interpret->path, \
            self->current_token->start.line + 1, self->current_token->start.column + 1, ##__VA_ARGS__\
        )
#define _analyser_warning(__fmt, ...)\
        fluff_error_fmt(FLUFF_COMPILE_ERROR, "%s:%zu:%zu: " __fmt, \
            self->lexer->interpret->path, \
            self->current_token->start.line + 1, self->current_token->start.column + 1, ##__VA_ARGS__\
        )

#define TOKEN_FMT(__index)\
        _lexer_token_string_len(self->lexer, __index), _lexer_token_string(self->lexer, __index)

FLUFF_PRIVATE_API void _analyser_read(Analyser * self) {
    self->current_token = self->lexer->tokens;
    while (_analyser_is_within_bounds(self)) {
        _analyser_read_token(self);
        _analyser_consume(self, 1);
    }
}

FLUFF_PRIVATE_API void _analyser_read_token(Analyser * self) {
    switch (_token_type_get_category(self->current_token->type)) {
        case TOKEN_CATEGORY_INTEGER_LITERAL:
        case TOKEN_CATEGORY_DECIMAL_LITERAL:
        case TOKEN_CATEGORY_STRING_LITERAL:
        case TOKEN_CATEGORY_LABEL_LITERAL:
        case TOKEN_CATEGORY_LPAREN:
        case TOKEN_CATEGORY_RPAREN:
        case TOKEN_CATEGORY_LBRACE:
        case TOKEN_CATEGORY_RBRACE:
        case TOKEN_CATEGORY_LBRACKET:
        case TOKEN_CATEGORY_RBRACKET:
        case TOKEN_CATEGORY_OPERATOR:
        case TOKEN_CATEGORY_OPERATOR_ARROW:
        case TOKEN_CATEGORY_OPERATOR_DOT:
        case TOKEN_CATEGORY_OPERATOR_ELLIPSIS: {
            _analyser_read_expr(self);
            break;
        }
        case TOKEN_CATEGORY_CONTROL_FLOW:
            { /* TODO: do some shit */ break; }
        case TOKEN_CATEGORY_DECL:
            { /* TODO: do some shit */ break; }
        case TOKEN_CATEGORY_FUNC_DECL:
            { /* TODO: do some shit */ break; }
        case TOKEN_CATEGORY_CLASS_DECL:
            { /* TODO: do some shit */ break; }
        case TOKEN_CATEGORY_OPERATOR_COMMA:
            {  }
        default: break;
    }
}

FLUFF_PRIVATE_API void _analyser_read_expr(Analyser * self) {
    ASTNode * node = _analyser_read_expr_pratt(self, 0);
    if (!node) _analyser_error("pratt parser returned null");
    _ast_node_suite_push(&self->ast->root, node);
}

FLUFF_PRIVATE_API ASTNode * _analyser_read_expr_pratt(Analyser * self, int prec_limit) {
    if (!_analyser_is_within_bounds(self))
        return NULL;

    ASTNode * lhs = NULL;
    switch (_token_type_get_category(self->current_token->type)) {
        case TOKEN_CATEGORY_BOOL_LITERAL: 
        case TOKEN_CATEGORY_INTEGER_LITERAL: 
        case TOKEN_CATEGORY_DECIMAL_LITERAL: 
        case TOKEN_CATEGORY_STRING_LITERAL: 
        case TOKEN_CATEGORY_LABEL_LITERAL: {
            // Operand
            lhs = _new_ast_node_from_token(self->lexer, self->current_index, self->ast);
            break;
        }
        case TOKEN_CATEGORY_LPAREN: {
            // Parenthesis
            _analyser_consume(self, 1);
            lhs = _analyser_read_expr_pratt(self, 0);
            _analyser_expect(self, TOKEN_CATEGORY_RPAREN);
            //_analyser_consume(self, 1);
            break;
        }
        case TOKEN_CATEGORY_OPERATOR: {
            // Unary operator
            TokenType op_type = self->current_token->type;
            _analyser_consume(self, 1);

            //_analyser_expect_n(self, literal_categories, FLUFF_LENOF(literal_categories));

            return _new_ast_node_unary_operator(
                self->ast, 
                token_info[op_type].unary_op, 
                _analyser_read_expr_pratt(self, token_info[op_type].prefix_prec)
            );
        }
        default: _analyser_error("unexpected token %*s", TOKEN_FMT(self->current_index));
    }

    while (!_analyser_is_expr_end(self)) {
        Token * op = _analyser_consume(self, 1);

        if (_analyser_is_expr_end(self))
            break;

        int prec = token_info[op->type].infix_prec;
        if (prec < prec_limit) break;

        _analyser_consume(self, 1);
        _analyser_expect_n(self, literal_categories, FLUFF_LENOF(literal_categories));

        lhs = _new_ast_node_operator(self->ast, token_info[op->type].op, 
            lhs, _analyser_read_expr_pratt(self, prec)
        );
        if (self->current_token->type == TOKEN_RPAREN)
            break;
    }

    return lhs;
}

FLUFF_PRIVATE_API bool _analyser_expect(Analyser * self, TokenCategory category) {
    TokenCategory token_category = _token_type_get_category(self->current_token->type);
    if (token_category == category)
        return true;
    _analyser_error_recoverable("expected token %s, got '%*s' (%s) instead", 
        _token_category_string(category), TOKEN_FMT(self->current_index), 
        _token_category_string(token_category)
    );
    // TODO: make this pretend the token is in the desired category so the error is recoverable
    return false;
}

FLUFF_PRIVATE_API bool _analyser_expect_n(Analyser * self, TokenCategory * categories, size_t count) {
    TokenCategory category = _token_type_get_category(self->current_token->type);

    size_t buffer_len = 0;
    for (size_t i = 0; i < count; ++i) {
        if (category == categories[i]) return true;
        buffer_len += (i != 0 ? 2 : 0) + strlen(_token_category_string(categories[i]));
    }

    char buffer[buffer_len + 1];
    buffer[buffer_len] = '\0';
    size_t i = 0;
    while (i < buffer_len) {
        const char * str = _token_category_string(categories[i]);
        const size_t len = strlen(str);
        strncpy(&buffer[i], str, len);
        i += len;
    }
    _analyser_error("expected token %s, got %s", buffer, _token_category_string(category));
    // TODO: this sucks so much holy shit please fix this for the love of god
    return false;
}

FLUFF_PRIVATE_API Token * _analyser_consume(Analyser * self, size_t n) {
    self->current_index += n;
    self->current_token = &self->lexer->tokens[self->current_index];
    return self->current_token;
}

FLUFF_PRIVATE_API Token * _analyser_rewind(Analyser * self, size_t n) {
    self->current_index -= n;
    self->current_token = &self->lexer->tokens[self->current_index];
    return self->current_token;
}

FLUFF_PRIVATE_API Token * _analyser_peek(Analyser * self, int offset) {
    int pos = ((int)self->current_index) + offset;
    if (pos >= ((int)self->lexer->token_count) || pos < 0) return NULL;
    return &self->lexer->tokens[pos];
}

FLUFF_PRIVATE_API Token * _analyser_peekp(Analyser * self, size_t index) {
    if (index >= self->lexer->token_count) return NULL;
    return &self->lexer->tokens[index];
}

FLUFF_PRIVATE_API bool _analyser_is_expr_end(Analyser * self) {
    if (!_analyser_is_within_bounds(self))
        _analyser_error("unexpected EOF");
    return self->current_token->type == TOKEN_END;
}

FLUFF_PRIVATE_API bool _analyser_is_within_bounds(Analyser * self) {
    return self->current_index < self->lexer->token_count;
}