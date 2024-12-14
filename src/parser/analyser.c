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

FLUFF_CONSTEXPR_V ASTNode * _new_ast_node_from_token(Lexer * self, size_t i, AST * ast, TextSect loc) {
    TokenType type = self->tokens[i].type;
    switch (type) {
        case TOKEN_BOOL_LITERAL:
            return _new_ast_node_bool(ast, self->tokens[i].data.b, loc);
        case TOKEN_INTEGER_LITERAL:
            return _new_ast_node_int(ast, self->tokens[i].data.i, loc);
        case TOKEN_DECIMAL_LITERAL:
            return _new_ast_node_float(ast, self->tokens[i].data.f, loc);
        case TOKEN_STRING_LITERAL:
        case TOKEN_LABEL_LITERAL: {
            ASTNode * node = _new_ast_node_string_n(ast, 
                _lexer_token_string(self, i), _lexer_token_string_len(self, i), loc
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
    MAKE_INFO(EQUAL,          18, 0,  true,  EQUAL,   NONE),
    MAKE_INFO(DOT,            17, 0,  false, DOT,     NONE),
    MAKE_INFO(LPAREN,         16, 0,  false, NONE,    NONE),
    MAKE_INFO(AS,             15, 0,  false, AS,      NONE),
    MAKE_INFO(IS,             15, 0,  false, IS,      NONE),
    MAKE_INFO(IN,             15, 0,  false, IN,      NONE),
    MAKE_INFO(NOT,            14, 14, true,  NONE,    NOT),
    MAKE_INFO(BIT_NOT,        14, 14, true,  NONE,    BIT_NOT),
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
    self->token        = self->lexer->tokens;
    self->state.expect = TOKEN_NONE;
    ASTNode * node = _analyser_read_scope(self);
    if (node) _ast_node_suite_push(&self->ast->root, node);
}

FLUFF_PRIVATE_API ASTNode * _analyser_read_token(Analyser * self) {
    if (!_analyser_is_within_bounds(self)) return NULL;
    AnalyserState prev_state = self->state;

    ASTNode * node = NULL;

    TokenCategory category = _token_type_get_category(self->token->type);
    switch (category) {
        case TOKEN_CATEGORY_LITERAL:
        case TOKEN_CATEGORY_LABEL:
        case TOKEN_CATEGORY_LPAREN:
        case TOKEN_CATEGORY_LBRACKET:
        case TOKEN_CATEGORY_OPERATOR:
        case TOKEN_CATEGORY_OPERATOR_ARROW:
        case TOKEN_CATEGORY_OPERATOR_DOT:
        case TOKEN_CATEGORY_OPERATOR_ELLIPSIS:
            { node = _analyser_read_expr(self, TOKEN_END); break; }
        case TOKEN_CATEGORY_IF:
            { node = _analyser_read_if(self); break; }
        case TOKEN_CATEGORY_FOR:
            { node = _analyser_read_for(self); break; }
        case TOKEN_CATEGORY_WHILE:
            { node = _analyser_read_while(self); break; }
        case TOKEN_CATEGORY_DECL:
            { node = _analyser_read_decl(self); break; }
        case TOKEN_CATEGORY_FUNC_DECL:
            { node = _analyser_read_func(self); break; }
        case TOKEN_CATEGORY_CLASS_DECL:
            { node = _analyser_read_class(self); break; }
        case TOKEN_CATEGORY_LBRACE: {
            self->state.expect = TOKEN_RBRACE;
            _analyser_consume(self, 1);
            node = _analyser_read_scope(self);
            break;
        }
        case TOKEN_CATEGORY_END:
            { break; }
        case TOKEN_CATEGORY_RPAREN:
        case TOKEN_CATEGORY_RBRACE:
        case TOKEN_CATEGORY_RBRACKET: { 
            _analyser_error("extraneous %s '%.*s'", 
                _token_category_string(category), TOKEN_FMT(self->index)
            );
        }
        case TOKEN_CATEGORY_ELSE:
            _analyser_error("'else' statement out of 'if' statement"); 
        default:
            _analyser_error("unexpected token '%.*s'", TOKEN_FMT(self->index));
    }
    self->state = prev_state;

    if (category != TOKEN_CATEGORY_END) {
        if (!node) _analyser_error("ast node returned null");

        if (self->state.last_scope)
            _ast_node_suite_push(self->state.last_scope, node);
    }

    return node;
}

FLUFF_PRIVATE_API ASTNode * _analyser_read_if(Analyser * self) {
    AnalyserState prev_state = self->state;

    ASTNode * node = _new_ast_node(self->ast, AST_IF, self->token->start);
    _analyser_consume(self, 1);
    node->data.if_cond.cond_expr = _analyser_read_expr(self, TOKEN_LBRACE);
    _analyser_consume(self, 2);
    self->state.expect = TOKEN_RBRACE;
    node->data.if_cond.if_scope = _analyser_read_scope(self);
    if (self->index + 1 < self->lexer->token_count && _analyser_peek(self, 1)->type == TOKEN_ELSE) {
        _analyser_consume(self, 2);
        self->state.last_scope = NULL;
        node->data.if_cond.else_scope = _analyser_read_token(self);
    }
    
    self->state = prev_state;
    return node;
}

FLUFF_PRIVATE_API ASTNode * _analyser_read_for(Analyser * self) {
    return NULL;
}

FLUFF_PRIVATE_API ASTNode * _analyser_read_while(Analyser * self) {
    return NULL;
}

FLUFF_PRIVATE_API ASTNode * _analyser_read_decl(Analyser * self) {
    return NULL;
}

FLUFF_PRIVATE_API ASTNode * _analyser_read_func(Analyser * self) {
    return NULL;
}

FLUFF_PRIVATE_API ASTNode * _analyser_read_class(Analyser * self) {
    return NULL;
}

FLUFF_PRIVATE_API ASTNode * _analyser_read_scope(Analyser * self) {
    AnalyserState prev_state = self->state;

    ASTNode * node = _new_ast_node(self->ast, AST_SCOPE, self->token->start);
    self->state.last_scope = node;
    while (_analyser_is_within_bounds(self)) {
        if (self->token->type == self->state.expect) break;
        _analyser_read_token(self);
        if (self->index + 1 >= self->lexer->token_count) break;
        _analyser_consume(self, 1);
    }
    if (self->state.expect != TOKEN_NONE)
        _analyser_expect(self->index, _token_type_get_category(self->state.expect));

    self->state = prev_state;
    return node;
}

FLUFF_PRIVATE_API ASTNode * _analyser_read_expr(Analyser * self, TokenType expect) {
    AnalyserState prev_state = self->state;
    self->state.in_call = false;
    self->state.expect  = expect;
    ASTNode * node = _analyser_read_expr_pratt(self, 0);
    if (!node) _analyser_error("pratt parser returned null");
    self->state = prev_state;
    return node;
}

FLUFF_PRIVATE_API ASTNode * _analyser_read_expr_pratt(Analyser * self, int prec_limit) {
    if (!_analyser_is_within_bounds(self))
        return NULL;

    //printf("token %zu\n", self->index);
    AnalyserState prev_state = self->state;

    TokenCategory category = _token_type_get_category(self->token->type);
    ASTNode * lhs = NULL;
    switch (category) {
        case TOKEN_CATEGORY_LITERAL: 
        case TOKEN_CATEGORY_LABEL: {
            // Operand
            lhs = _new_ast_node_from_token(self->lexer, self->index, self->ast, self->token->start);
            break;
        }
        case TOKEN_CATEGORY_LPAREN: {
            // Parenthesis
            _analyser_consume(self, 1);
            self->state.expect  = TOKEN_RPAREN;
            self->state.in_call = false;
            lhs = _analyser_read_expr_pratt(self, 0);
            _analyser_consume(self, 1);
            _analyser_expect(self->index, TOKEN_CATEGORY_RPAREN);
            self->state = prev_state;
            if (self->state.in_call) return lhs;
            break;
        }
        case TOKEN_CATEGORY_OPERATOR: {
            // Unary operator
            TokenType op_type = self->token->type;
            _analyser_consume(self, 1);
            ASTNode * node = _analyser_read_expr_pratt(self, token_info[op_type].prefix_prec);
            if (!node) _analyser_error("pratt parser returned null");
            lhs = _new_ast_node_unary_operator(self->ast, 
                token_info[op_type].unary_op, 
                node, self->token->start
            );
            break;
        }
        default: _analyser_error("expected expression, got %s '%.*s'", 
            _token_category_string(category), TOKEN_FMT(self->index)
        );
    }

    while (!_analyser_is_expr_end(self)) {
        TokenType type = _analyser_peek(self, 1)->type;
        _analyser_expect(self->index + 1, 
            TOKEN_CATEGORY_OPERATOR, TOKEN_CATEGORY_OPERATOR_DOT, TOKEN_CATEGORY_OPERATOR_COMMA, 
            TOKEN_CATEGORY_END, _token_type_get_category(self->state.expect), TOKEN_CATEGORY_LPAREN
        );
        if (type == self->state.expect) break;

        int prec = token_info[type].infix_prec;
        //printf("prec %d vs %d on token %zu\n", prec, prec_limit, self->index + 1);
        if (prec < prec_limit) break;
        //printf("condition met at token %zu\n", self->index + 1);

        if (type == TOKEN_LPAREN) {
            _analyser_consume(self, 1);
            lhs = _analyser_read_expr_call(self, lhs);
            if (!lhs) _analyser_error("pratt parser returned null");
            continue;
        }
        if (type == TOKEN_COMMA && !self->state.in_call) {
            _analyser_error("comma outside of function call, array or class declaration");
        }

        TextSect sect = self->token->start;

        _analyser_consume(self, 2);
        ASTNode * rhs = _analyser_read_expr_pratt(self, prec);
        if (!rhs) _analyser_error("pratt parser returned null");
        if (token_info[type].right_associative)
            lhs = _new_ast_node_operator(self->ast, token_info[type].op, rhs, lhs, sect);
        else
            lhs = _new_ast_node_operator(self->ast, token_info[type].op, lhs, rhs, sect);
    }

    //printf("returned\n");
    self->state = prev_state;
    return lhs;
}

FLUFF_PRIVATE_API ASTNode * _analyser_read_expr_call(Analyser * self, ASTNode * top) {
    AnalyserState prev_state = self->state;

    TextSect sect = self->token->start;

    if (_analyser_peek(self, 1)->type != TOKEN_RPAREN) {
        self->state.in_call = true;
        top->next           = _analyser_read_expr_pratt(self, 0);
    } else _analyser_consume(self, 1);

    top = _new_ast_node_call(top->ast, top, self->state.comma_count + 1, sect);

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
    if (self->index >= self->lexer->token_count)
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