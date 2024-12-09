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
    TokenType type = self->tokens[i].type;
    switch (type) {
        case TOKEN_TRUE: case TOKEN_FALSE: 
            return _new_ast_node_bool(ast, self->tokens[i].data.b);
        case TOKEN_INTEGER_LITERAL:
            return _new_ast_node_int(ast, self->tokens[i].data.i);
        case TOKEN_DECIMAL_LITERAL:
            return _new_ast_node_float(ast, self->tokens[i].data.f);
        case TOKEN_STRING_LITERAL:
        case TOKEN_LABEL_LITERAL: {
            ASTNode * node = _new_ast_node_string_n(ast, 
                _lexer_token_string(self, i), _lexer_token_string_len(self, i)
            );
            if (type == TOKEN_LABEL_LITERAL) {
                node->type = AST_LABEL_LITERAL;
            }
            return node;
        }
        default: break;
    }
    return NULL;
}

#define MAKE_INFO(arr, prec, uprec, assoc, name, uname)\
        [TOKEN_##arr] = { prec, uprec, assoc, AST_OPERATOR_##name, AST_UOPERATOR_##uname }

FLUFF_CONSTEXPR_V struct {
    int  infix_prec;
    int  prefix_prec;
    bool right_associative;

    ASTOperatorDataType      op;
    ASTUnaryOperatorDataType unary_op;
} token_info[] = {
    // NOTE: yes I copied half of these from C++ standard, how did you know
    MAKE_INFO(EQUAL,          17, 0,  true,  EQUAL,   NONE),
    MAKE_INFO(DOT,            16, 0,  false, DOT,     NONE),
    MAKE_INFO(AS,             16, 0,  false, AS,      NONE),
    MAKE_INFO(IS,             16, 0,  false, IS,      NONE),
    MAKE_INFO(IN,             16, 0,  false, IN,      NONE),
    MAKE_INFO(NOT,            15, 15, true,  NONE,    NOT),
    MAKE_INFO(BIT_NOT,        15, 15, true,  NONE,    BIT_NOT),
    MAKE_INFO(MODULO,         13, 0,  false, MOD,     NONE),
    MAKE_INFO(POWER,          13, 0,  true,  POW,     NONE),
    MAKE_INFO(MULTIPLY,       12, 0,  false, MUL,     NONE),
    MAKE_INFO(DIVIDE,         12, 0,  false, DIV,     NONE),
    MAKE_INFO(PLUS,           11, 13, true,  ADD,     PROMOTE),
    MAKE_INFO(MINUS,          11, 13, true,  SUB,     NEGATE),
    MAKE_INFO(BIT_SHL,        10, 0,  false, BIT_SHL, NONE),
    MAKE_INFO(BIT_SHR,        10, 0,  false, BIT_SHR, NONE),
    MAKE_INFO(LESS,           10, 0,  false, LT,      NONE),
    MAKE_INFO(LESS_EQUALS,    10, 0,  false, LE,      NONE),
    MAKE_INFO(GREATER,        10, 0,  false, GT,      NONE),
    MAKE_INFO(GREATER_EQUALS, 9,  0,  false, GE,      NONE),
    MAKE_INFO(EQUALS,         9,  0,  false, EQ,      NONE),
    MAKE_INFO(NOT_EQUALS,     8,  0,  false, NE,      NONE),
    MAKE_INFO(BIT_AND,        7,  0,  false, BIT_AND, NONE),
    MAKE_INFO(BIT_XOR,        6,  0,  false, BIT_XOR, NONE),
    MAKE_INFO(BIT_OR,         5,  0,  false, BIT_OR,  NONE),
    MAKE_INFO(AND,            4,  0,  false, AND,     NONE),
    MAKE_INFO(OR,             3,  0,  false, OR,      NONE),
    MAKE_INFO(COMMA,          2,  0,  true,  COMMA,   NONE),

    // [TOKEN_COLON]    = { 1,  0, false, AST_OPERATOR_DOT, AST_UOPERATOR_NONE },
    // [TOKEN_ARROW]    = { 1,  0, false, AST_OPERATOR_DOT, AST_UOPERATOR_NONE },
    // [TOKEN_ELLIPSIS] = { 1,  0, false, AST_OPERATOR_DOT, AST_UOPERATOR_NONE },
};

FLUFF_CONSTEXPR_V TokenCategory literal_categories[] = {
    TOKEN_CATEGORY_LITERAL, 
    TOKEN_CATEGORY_LABEL, 
};

/* -=============
     Analyser
   =============- */

FLUFF_PRIVATE_API void _new_analyser(Analyser * self, Lexer * lexer) {
    FLUFF_CLEANUP(self);
    self->interpret = lexer->interpret;
    self->lexer     = lexer;
    self->ast       = &lexer->interpret->ast;
}

FLUFF_PRIVATE_API void _free_analyser(Analyser * self) {
    FLUFF_CLEANUP(self);
}

// TODO: make ErrorPool class for supporting multiple errors
#ifdef FLUFF_DEBUG
#define _analyser_error_print(__fmt, ...)\
        fluff_error_fmt(FLUFF_COMPILE_ERROR, "[%s:%d]:\n\t->%s:%zu:%zu: " __fmt, \
            __FILE__, __LINE__, self->lexer->interpret->path, \
            self->position.line + 1, self->position.column + 1, ##__VA_ARGS__\
        )
#else
#define _analyser_error_print(__fmt, ...)\
        fluff_error_fmt(FLUFF_COMPILE_ERROR, "%s:%zu:%zu: " __fmt, \
            self->lexer->interpret->path, \
            self->position.line + 1, self->position.column + 1, ##__VA_ARGS__\
        )
#endif

#define _analyser_error(__fmt, ...)\
        _analyser_error_print(__fmt, ##__VA_ARGS__)
#define _analyser_error_recoverable(__fmt, ...)\
        _analyser_error_print(__fmt, ##__VA_ARGS__)
#define _analyser_warning(__fmt, ...)\
        _analyser_error_print(__fmt, ##__VA_ARGS__)
#define _analyser_expect(__idx, ...) {\
            const TokenCategory __c[] = { __VA_ARGS__ };\
            _analyser_expect_n(self, __idx, __c, FLUFF_LENOF(__c));\
        }

#define TOKEN_FMT(__index)\
        _lexer_token_string_len(self->lexer, __index), _lexer_token_string(self->lexer, __index)

FLUFF_PRIVATE_API void _analyser_read(Analyser * self) {
    self->token = self->lexer->tokens;
    _analyser_read_token(self);
    while (_analyser_is_within_bounds(self)) {
        _analyser_consume(self, 1);
        _analyser_read_token(self);
    }
}

FLUFF_PRIVATE_API void _analyser_read_token(Analyser * self) {
    if (!_analyser_is_within_bounds(self)) return;
    switch (_token_type_get_category(self->token->type)) {
        case TOKEN_CATEGORY_LITERAL:
        case TOKEN_CATEGORY_LABEL:
        case TOKEN_CATEGORY_LPAREN:
        case TOKEN_CATEGORY_RPAREN:
        case TOKEN_CATEGORY_LBRACE:
        case TOKEN_CATEGORY_RBRACE:
        case TOKEN_CATEGORY_LBRACKET:
        case TOKEN_CATEGORY_RBRACKET:
        case TOKEN_CATEGORY_OPERATOR:
        case TOKEN_CATEGORY_OPERATOR_ARROW:
        case TOKEN_CATEGORY_OPERATOR_DOT:
        case TOKEN_CATEGORY_OPERATOR_ELLIPSIS:
            { _analyser_read_expr(self); break; }
        case TOKEN_CATEGORY_IF:
            { _analyser_read_if(self); break; }
        case TOKEN_CATEGORY_FOR:
            { _analyser_read_for(self); break; }
        case TOKEN_CATEGORY_WHILE:
            { _analyser_read_while(self); break; }
        case TOKEN_CATEGORY_DECL:
            { _analyser_read_decl(self); break; }
        case TOKEN_CATEGORY_FUNC_DECL:
            { _analyser_read_func(self); break; }
        case TOKEN_CATEGORY_CLASS_DECL:
            { _analyser_read_class(self); break; }
        case TOKEN_CATEGORY_END: {
            _analyser_consume(self, 1);
            break;
        }
        case TOKEN_CATEGORY_ELSE:
            _analyser_error("'else' statement out of 'if' statement"); 
        default:
            _analyser_error("unexpected token '%.*s'", TOKEN_FMT(self->index));
    }
}

FLUFF_PRIVATE_API void _analyser_read_if(Analyser * self) {

}

FLUFF_PRIVATE_API void _analyser_read_for(Analyser * self) {

}

FLUFF_PRIVATE_API void _analyser_read_while(Analyser * self) {

}

FLUFF_PRIVATE_API void _analyser_read_decl(Analyser * self) {

}

FLUFF_PRIVATE_API void _analyser_read_func(Analyser * self) {

}

FLUFF_PRIVATE_API void _analyser_read_class(Analyser * self) {

}

FLUFF_PRIVATE_API void _analyser_read_expr(Analyser * self) {
    AnalyserState prev_state = self->state;
    self->state.in_call = false;
    self->state.expect  = TOKEN_END;
    ASTNode * node = _analyser_read_expr_pratt(self, 0);
    if (!node) _analyser_error("pratt parser returned null");
    _ast_node_suite_push(&self->ast->root, node);

    self->state = prev_state;
}

FLUFF_PRIVATE_API ASTNode * _analyser_read_expr_pratt(Analyser * self, int prec_limit) {
    if (!_analyser_is_within_bounds(self))
        return NULL;

    AnalyserState prev_state = self->state;

    TokenCategory category = _token_type_get_category(self->token->type);
    ASTNode * lhs = NULL;
    switch (category) {
        case TOKEN_CATEGORY_LITERAL: 
        case TOKEN_CATEGORY_LABEL: {
            // Operand
            lhs = _new_ast_node_from_token(self->lexer, self->index, self->ast);
            break;
        }
        case TOKEN_CATEGORY_LPAREN: {
            // Parenthesis
            _analyser_consume(self, 1);
            self->state.expect = TOKEN_RPAREN;
            lhs = _analyser_read_expr_pratt(self, 0);
            _analyser_consume(self, 1);
            _analyser_expect(self->index, TOKEN_CATEGORY_RPAREN);
            self->state = prev_state;
            return lhs;
        }
        case TOKEN_CATEGORY_OPERATOR: {
            // Unary operator
            TokenType op_type = self->token->type;
            _analyser_consume(self, 1);
            ASTNode * node = _analyser_read_expr_pratt(self, token_info[op_type].prefix_prec);
            if (!node) _analyser_error("pratt parser returned null");
            lhs = _new_ast_node_unary_operator(self->ast, token_info[op_type].unary_op, node);
            break;
        }
        case TOKEN_CATEGORY_RPAREN: {
            if (self->state.in_call) {
                _analyser_rewind(self, 1);
                return NULL;
            } else _analyser_error("extraneous closing parenthesis");
        }
        default: _analyser_error("expected expression, got %s '%.*s'", 
            _token_category_string(category), TOKEN_FMT(self->index)
        );
    }

    while (!_analyser_is_expr_end(self)) {
        TokenType type = _analyser_peek(self, 1)->type;
        _analyser_expect(self->index + 1, 
            TOKEN_CATEGORY_OPERATOR, TOKEN_CATEGORY_OPERATOR_DOT, TOKEN_CATEGORY_OPERATOR_COMMA, 
            TOKEN_CATEGORY_LPAREN,   _token_type_get_category(self->state.expect)
        );
        if (type == self->state.expect) break;

        if (type == TOKEN_LPAREN) {
            if (prec_limit < 13) break;
            lhs = _analyser_read_expr_call(self, lhs);
            continue;
        }

        int prec = token_info[type].infix_prec;
        if (prec < prec_limit) break;

        _analyser_consume(self, 2);
        ASTNode * rhs = _analyser_read_expr_pratt(self, prec_limit);
        if (!rhs) _analyser_error("pratt parser returned null");
        if (token_info[type].right_associative)
            lhs = _new_ast_node_operator(self->ast, token_info[type].op, rhs, lhs);
        else
            lhs = _new_ast_node_operator(self->ast, token_info[type].op, lhs, rhs);
    }

    self->state = prev_state;
    return lhs;
}

FLUFF_PRIVATE_API ASTNode * _analyser_read_expr_call(Analyser * self, ASTNode * top) {
    AnalyserState prev_state = self->state;

    _analyser_consume(self, 1);
    self->state.in_call = true;
    self->state.expect  = TOKEN_RPAREN;
    top->next           = _analyser_read_expr_pratt(self, 0);
    _analyser_expect(self->index, TOKEN_CATEGORY_RPAREN);

    top = _new_ast_node_call(top->ast, top, self->state.comma_count + 1);

    self->state = prev_state;
    return top;
}

FLUFF_PRIVATE_API bool _analyser_expect_n(Analyser * self, size_t idx, const TokenCategory * categories, size_t count) {
    TokenCategory category = _token_type_get_category(self->lexer->tokens[idx].type);

    const char * strs[count];
    size_t       lens[count];
    size_t       len = 0;
    for (size_t i = 0; i < count; ++i) {
        if (category == categories[i]) return true;
        strs[i] = _token_category_string(categories[i]);
        lens[i] = strlen(strs[i]);
        len += lens[i] + (i > 0 ? (i + 1 < count ? 2 : 4) : 0);
    }

    char   buf[len];
    size_t j = 0;
    for (size_t i = 0; i < count; ++i) {
        if (i > 0) {
            if (i + 1 < count) {
                strcpy(&buf[j], ", ");
                j += 2;
            } else {
                strcpy(&buf[j], " or ");
                j += 4;
            }
        }
        strncpy(&buf[j], strs[i], lens[i]);
        j += lens[i];
    }
    _analyser_error("expected %.*s, got %s '%.*s'", 
        (int)len, buf, _token_category_string(category), TOKEN_FMT(idx)
    );
    // TODO: this looks like garbage but works perfectly so maybe fix it idk
    return false;
}

FLUFF_PRIVATE_API Token * _analyser_consume(Analyser * self, size_t n) {
    self->index += n;
    if (self->index > self->lexer->token_count)
        _analyser_error("unexpected EOF");
    self->token    = &self->lexer->tokens[self->index];
    self->position = self->token->start;
    return self->token;
}

FLUFF_PRIVATE_API Token * _analyser_rewind(Analyser * self, size_t n) {
    if (n > self->index)
        _analyser_error("unexpected rewind");
    self->index   -= n;
    self->token    = &self->lexer->tokens[self->index];
    self->position = self->token->start;
    return self->token;
}

FLUFF_PRIVATE_API Token * _analyser_peek(Analyser * self, int offset) {
    int pos = ((int)self->index) + offset;
    if (pos >= ((int)self->lexer->token_count) || pos < 0)
        _analyser_error("unexpected EOF");
    return &self->lexer->tokens[pos];
}

FLUFF_PRIVATE_API Token * _analyser_peekp(Analyser * self, size_t index) {
    if (index >= self->lexer->token_count) return NULL;
    return &self->lexer->tokens[index];
}

FLUFF_PRIVATE_API bool _analyser_is_expr_end(Analyser * self) {
    if (!_analyser_is_within_bounds(self))
        _analyser_error("unexpected EOF");
    return (!self->state.in_statement && self->token->type == TOKEN_END) ||
           (self->state.in_statement  && self->token->type == TOKEN_LBRACE);
}

FLUFF_PRIVATE_API bool _analyser_is_within_bounds(Analyser * self) {
    return self->index < self->lexer->token_count;
}