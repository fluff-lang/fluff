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
                node->type = AST_NODE_LABEL_LITERAL;
            }
            return node;
        }
        default: break;
    }
    return NULL;
}

#define MAKE_INFO(arr, prec, uprec, assoc, name, uname)\
        [TOKEN_##arr] = { prec, uprec, assoc, AST_OPERATOR_##name, AST_OPERATOR_##uname }

FLUFF_CONSTEXPR_V struct {
    int  infix_prec;
    int  prefix_prec;
    bool right_associative;

    ASTOperatorType op;
    ASTOperatorType unary_op;
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
    _free_ast(self->ast);
    FLUFF_CLEANUP(self);
}

// TODO: make ErrorPool class for supporting multiple errors
#ifdef FLUFF_DEBUG
#define _analyser_error_print_d(__line, __func, __fmt, ...)\
        fluff_panic_fmt("[at %s:%s():%d]:\n\t-> %s:%zu:%zu: " __fmt, \
            __FILE__, __func, __line, self->lexer->interpret->path, \
            self->position.line + 1, self->position.column + 1, ##__VA_ARGS__\
        )
#else
#define _analyser_error_print_d(__line, __func, __fmt, ...)\
        fluff_panic_fmt("%s:%zu:%zu: " __fmt, \
            self->lexer->interpret->path, \
            self->position.line + 1, self->position.column + 1, ##__VA_ARGS__\
        )
#endif

#define _analyser_error_print(...)\
        _analyser_error_print_d(__LINE__, __func__, __VA_ARGS__)

#define _analyser_error(__fmt, ...)\
        _analyser_error_print(__fmt, ##__VA_ARGS__)
#define _analyser_error_recoverable(__fmt, ...)\
        _analyser_error_print(__fmt, ##__VA_ARGS__)
#define _analyser_warning(__fmt, ...)\
        _analyser_error_print(__fmt, ##__VA_ARGS__)
#define _analyser_expect(__idx, ...) {\
            const TokenCategory __c[] = { __VA_ARGS__ };\
            _analyser_expect_n(self, __idx, __c, FLUFF_LENOF(__c), __LINE__, __func__);\
        }

#define TOKEN_FMT(__index)\
        _lexer_token_string_len(self->lexer, __index), _lexer_token_string(self->lexer, __index)

#define TOKEN_TO_STRING_NODE()\
        _new_ast_node_string_n(self->ast,\
            &self->lexer->str[self->token.start.index], self->token.length, self->position\
        )

#define PUSH_STATE() AnalyserState prev_state = self->state; ++self->state.call_depth;
#define POP_STATE()  self->state = prev_state;

// TODO: make AST_TYPED_LABEL
// TODO: make sure nodes are cleared up on error
FLUFF_PRIVATE_API void _analyser_read(Analyser * self) {
    if (self->lexer->token_count == 0) return;
    self->token        = * self->lexer->tokens;
    self->state.expect = TOKEN_EOF;
    ASTNode * node = _analyser_read_scope(self);
    if (node) _ast_node_append_child(&self->ast->root, node);
}

FLUFF_PRIVATE_API ASTNode * _analyser_read_token(Analyser * self) {
    if (!_analyser_is_within_bounds(self)) return NULL;
    PUSH_STATE();

    ASTNode * node = NULL;

    TokenCategory category = _token_type_get_category(self->token.type);
    switch (category) {
        case TOKEN_CATEGORY_LITERAL:
        case TOKEN_CATEGORY_LABEL:
        case TOKEN_CATEGORY_LPAREN:
        case TOKEN_CATEGORY_LBRACKET:
        case TOKEN_CATEGORY_OPERATOR:
        case TOKEN_CATEGORY_OPERATOR_ARROW:
        case TOKEN_CATEGORY_OPERATOR_DOT:
        case TOKEN_CATEGORY_OPERATOR_ELLIPSIS:
        case TOKEN_CATEGORY_FUNC_DECL:
            { node = _analyser_read_expr(self, TOKEN_END); break; }
        case TOKEN_CATEGORY_IF:
            { node = _analyser_read_if(self); break; }
        case TOKEN_CATEGORY_FOR:
            { node = _analyser_read_for(self); break; }
        case TOKEN_CATEGORY_WHILE:
            { node = _analyser_read_while(self); break; }
        case TOKEN_CATEGORY_DECL:
            { node = _analyser_read_decl(self); break; }
        case TOKEN_CATEGORY_CLASS_DECL:
            { node = _analyser_read_class(self); break; }
        case TOKEN_CATEGORY_LBRACE: {
            self->state.expect = TOKEN_RBRACE;
            _analyser_consume(self, 1);
            node = _analyser_read_scope(self);
            break;
        }
        case TOKEN_CATEGORY_CONTROL_KEYWORD:
            { node = _analyser_read_control(self); break; }
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
            _ast_node_append_child(self->state.last_scope, node);
    }

    POP_STATE();
    return node;
}

FLUFF_PRIVATE_API ASTNode * _analyser_read_scope(Analyser * self) {
    PUSH_STATE();

    ASTNode * node = _new_ast_node(self->ast, AST_NODE_SCOPE, self->position);
    self->state.last_scope = node;
    while (_analyser_is_within_bounds(self)) {
        if (self->token.type == self->state.expect) break;
        _analyser_read_token(self);
        _analyser_consume(self, 1);
    }
    _analyser_expect(self->index, _token_type_get_category(self->state.expect));

    POP_STATE();
    return node;
}

FLUFF_PRIVATE_API ASTNode * _analyser_read_if(Analyser * self) {
    PUSH_STATE();
    self->state.last_scope = NULL;

    ASTNode * node = _new_ast_node(self->ast, AST_NODE_IF, self->position);
    _analyser_consume(self, 1);
    _ast_node_append_child(node, _analyser_read_expr(self, TOKEN_LBRACE));

    _analyser_consume(self, 2);
    self->state.expect = TOKEN_RBRACE;
    _ast_node_append_child(node, _analyser_read_scope(self));

    if (_analyser_peek(self, 1).type == TOKEN_ELSE) {
        _analyser_consume(self, 2);
        _ast_node_append_child(node, _analyser_read_token(self));
    }
    
    POP_STATE();
    return node;
}

FLUFF_PRIVATE_API ASTNode * _analyser_read_for(Analyser * self) {
    return NULL;
}

FLUFF_PRIVATE_API ASTNode * _analyser_read_while(Analyser * self) {
    PUSH_STATE();
    self->state.last_scope = NULL;

    ASTNode * node = _new_ast_node(self->ast, AST_NODE_WHILE, self->position);
    _analyser_consume(self, 1);
    _ast_node_append_child(node, _analyser_read_expr(self, TOKEN_LBRACE));

    _analyser_consume(self, 2);
    self->state.expect = TOKEN_RBRACE;
    _ast_node_append_child(node, _analyser_read_scope(self));
    
    POP_STATE();
    return node;
}

FLUFF_PRIVATE_API ASTNode * _analyser_read_decl(Analyser * self) {
    PUSH_STATE();

    ASTNode * node = _new_ast_node(self->ast, AST_NODE_DECLARATION, self->position);
    node->data.decl.is_constant = (self->token.type == TOKEN_CONST);
    _analyser_consume(self, 1);
    _analyser_expect(self->index, TOKEN_CATEGORY_LABEL);

    if (_analyser_peek(self, 1).type == TOKEN_END)
        _analyser_error("cannot declare a variable without implying or specifying it's type");

    self->state.in_decl = (_analyser_peek(self, 1).type == TOKEN_COLON);
    _ast_node_append_child(node, _analyser_read_expr(self, TOKEN_END));
    _ast_node_append_child(node, self->extra_return);
    self->extra_return = NULL;

    POP_STATE();
    return node;
}

FLUFF_PRIVATE_API ASTNode * _analyser_read_func(Analyser * self) {
    PUSH_STATE();

    ASTNode * node = _new_ast_node(self->ast, AST_NODE_FUNCTION, self->position);

    _analyser_consume(self, 1);
    _analyser_expect(self->index, TOKEN_CATEGORY_LABEL, TOKEN_CATEGORY_LPAREN);
    if (self->token.type == TOKEN_LABEL_LITERAL) {
        if (self->state.in_expr)
            _analyser_error("cannot declare a named function within an expression");
        _ast_node_append_child(node, TOKEN_TO_STRING_NODE());
        _analyser_consume(self, 1);
    }
    _analyser_expect(self->index, TOKEN_CATEGORY_LPAREN);
    
    self->state.in_call = true;
    if (_analyser_peek(self, 1).type != TOKEN_RPAREN) {
        while (_analyser_is_within_bounds(self)) {
            if (self->token.type == TOKEN_RPAREN || self->token.type == self->state.expect) break;
            _analyser_consume(self, 1);
            _ast_node_append_child(node, TOKEN_TO_STRING_NODE());
            _analyser_consume(self, 1);
            if (self->token.type == TOKEN_COLON) {
                _analyser_consume(self, 1);
                _ast_node_append_child(node, _analyser_read_type(self, self->state.expect));
            }
            _analyser_expect(self->index, 
                TOKEN_CATEGORY_OPERATOR_COMMA, TOKEN_CATEGORY_RPAREN
            );
        }
    } else _analyser_consume(self, 1);
    _analyser_expect(self->index, TOKEN_CATEGORY_RPAREN);
    _analyser_consume(self, 1);
    self->state.in_call = false;
    if (self->token.type == TOKEN_ARROW) {
        _analyser_consume(self, 1);
        _ast_node_append_child(node, _analyser_read_type(self, self->state.expect));
    }

    _analyser_expect(self->index, TOKEN_CATEGORY_LBRACE);
    _analyser_consume(self, 1);
    self->state.expect = TOKEN_RBRACE;
    _ast_node_append_child(node, _analyser_read_scope(self));

    POP_STATE();
    return node;
}

FLUFF_PRIVATE_API ASTNode * _analyser_read_class(Analyser * self) {
    return NULL;
}

FLUFF_PRIVATE_API ASTNode * _analyser_read_type(Analyser * self, TokenType expect) {
    PUSH_STATE();

    bool had_error = false;

    ASTNode * node = _new_ast_node(self->ast, AST_NODE_TYPE, self->position);
    if (self->token.type == TOKEN_END) _analyser_error("missing type");
    if (self->token.type == expect) {
        self->state = prev_state;
        return node;
    }
    // TODO: let the tokens decide between consuming at the end instead of just rewinding it
    switch (self->token.type) {
        case TOKEN_VOID: {
            node->data.type = AST_TYPE_VOID;
            break;
        }
        case TOKEN_BOOL: {
            node->data.type = AST_TYPE_BOOL;
            break;
        }
        case TOKEN_INT: {
            node->data.type = AST_TYPE_INT;
            break;
        }
        case TOKEN_FLOAT: {
            node->data.type = AST_TYPE_FLOAT;
            break;
        }
        case TOKEN_STRING: {
            node->data.type = AST_TYPE_STRING;
            break;
        }
        case TOKEN_OBJECT: {
            node->data.type = AST_TYPE_OBJECT;
            break;
        }
        case TOKEN_LABEL_LITERAL: {
            node->data.type = AST_TYPE_CLASS;
            _ast_node_append_child(node, TOKEN_TO_STRING_NODE());
            break;
        }
        case TOKEN_LBRACKET: {
            _analyser_consume(self, 1);
            node->data.type = AST_TYPE_ARRAY;
            if (_analyser_peek(self, 1).type != TOKEN_RBRACKET)
                _ast_node_append_child(node, _analyser_read_type(self, expect));
            _analyser_expect(self->index, TOKEN_CATEGORY_RBRACKET);
            break;
        }
        case TOKEN_FUNC: {
            _analyser_consume(self, 1);
            _analyser_expect(self->index, TOKEN_CATEGORY_LPAREN);
            node->data.type = AST_TYPE_FUNC;
            
            self->state.in_call = true;
            if (_analyser_peek(self, 1).type != TOKEN_RPAREN) {
                while (_analyser_is_within_bounds(self)) {
                    if (self->token.type == TOKEN_RPAREN || self->token.type == expect) break;
                    _analyser_consume(self, 1);
                    _ast_node_append_child(node, _analyser_read_type(self, expect));
                    _analyser_expect(self->index, 
                        TOKEN_CATEGORY_OPERATOR_COMMA, TOKEN_CATEGORY_RPAREN
                    );
                }
            } else _analyser_consume(self, 1);
            _analyser_expect(self->index, TOKEN_CATEGORY_RPAREN);
            self->state.in_call = false;
            if (_analyser_peek(self, 1).type == TOKEN_ARROW) {
                _analyser_consume(self, 2);
                _ast_node_append_child(node, _analyser_read_type(self, expect));
                _analyser_rewind(self, 1);
            }
            break;
        }
        case TOKEN_RPAREN: {
            had_error = (!self->state.in_call);
            break;
        }
        default: {
            had_error = true;
            break;
        }
    }
    if (had_error)
        _analyser_error("expected type, got %s '%.*s'", 
            _token_category_string(_token_type_get_category(self->token.type)), 
            TOKEN_FMT(self->index)
        );

    _analyser_consume(self, 1);

    POP_STATE();
    return node;
}

FLUFF_PRIVATE_API ASTNode * _analyser_read_control(Analyser * self) {
    PUSH_STATE();

    ASTNode * node = NULL;

    switch (self->token.type) {
        case TOKEN_RETURN: {
            node = _new_ast_node(self->ast, AST_NODE_RETURN, self->position);
            if (_analyser_peek(self, 1).type != TOKEN_END) {
                _analyser_consume(self, 1);
                _ast_node_append_child(node, _analyser_read_expr(self, TOKEN_END));
            }
            break;
        }
        case TOKEN_BREAK:
            { node = _new_ast_node(self->ast, AST_NODE_BREAK, self->position); break; }
        case TOKEN_CONTINUE:
            { node = _new_ast_node(self->ast, AST_NODE_CONTINUE, self->position); break; }
        default: FLUFF_UNREACHABLE();
    }
    _analyser_consume(self, 1);
    _analyser_expect(self->index, TOKEN_CATEGORY_END);

    POP_STATE();
    return node;
}

FLUFF_PRIVATE_API ASTNode * _analyser_read_expr(Analyser * self, TokenType expect) {
    PUSH_STATE();
    ++self->state.call_depth;
    self->state.in_call = false;
    self->state.expect  = expect;
    ASTNode * node = _analyser_read_expr_pratt(self, 0);
    if (!node) _analyser_error("pratt parser returned null");
    POP_STATE();
    return node;
}

FLUFF_PRIVATE_API ASTNode * _analyser_read_expr_pratt(Analyser * self, int prec_limit) {
    if (!_analyser_is_within_bounds(self))
        _analyser_error("expected expression, got EOF");

    PUSH_STATE();

    self->state.in_decl    = false;
    self->state.last_scope = NULL;

    TokenCategory category = _token_type_get_category(self->token.type);
    ASTNode * lhs = NULL;
    switch (category) {
        case TOKEN_CATEGORY_LITERAL: 
        case TOKEN_CATEGORY_LABEL: {
            // Operand
            lhs = _new_ast_node_from_token(self->lexer, self->index, self->ast, self->position);
            break;
        }
        case TOKEN_CATEGORY_FUNC_DECL: {
            // Function declaration
            lhs = _analyser_read_func(self);
            break;
        }
        case TOKEN_CATEGORY_LPAREN: {
            // Parenthesis
            _analyser_consume(self, 1);
            PUSH_STATE();
            self->state.expect = TOKEN_RPAREN;
            lhs = _analyser_read_expr_pratt(self, 0);
            _analyser_consume(self, 1);
            _analyser_expect(self->index, TOKEN_CATEGORY_RPAREN);
            POP_STATE();
            if (self->state.in_call) return lhs;
            break;
        }
        case TOKEN_CATEGORY_OPERATOR: {
            // Unary operator
            TokenType op_type = self->token.type;
            if (token_info[op_type].unary_op == AST_OPERATOR_NONE)
                _analyser_error("operator '%.*s' is not unary", TOKEN_FMT(self->index));
            _analyser_consume(self, 1);
            ASTNode * node = _analyser_read_expr_pratt(self, token_info[op_type].prefix_prec);
            if (!node) _analyser_error("pratt parser returned null");
            lhs = _new_ast_node_unary_operator(self->ast, 
                token_info[op_type].unary_op, 
                node, self->position
            );
            break;
        }
        default: _analyser_error("expected expression, got %s '%.*s'", 
            _token_category_string(category), TOKEN_FMT(self->index)
        );
    }

    self->state.in_expr = true;

    if (prev_state.in_decl) {
        _analyser_expect(self->index + 1, TOKEN_CATEGORY_OPERATOR_COLON, TOKEN_CATEGORY_RPAREN);
        _analyser_consume(self, 1);
        if (self->token.type == TOKEN_RPAREN && !prev_state.in_call)
            _analyser_error("unexpected %s '%.*s' in type declaration", 
                _token_category_string(_token_type_get_category(self->token.type)), TOKEN_FMT(self->index)
            );
        _analyser_consume(self, 1);
        self->extra_return = _analyser_read_type(self, TOKEN_EQUAL);
        _analyser_rewind(self, 1);
    }

    while (!_analyser_is_expr_end(self)) {
        TokenType type = _analyser_peek(self, 1).type;
        _analyser_expect(self->index + 1, 
            TOKEN_CATEGORY_OPERATOR, TOKEN_CATEGORY_OPERATOR_DOT, TOKEN_CATEGORY_OPERATOR_COMMA, 
            TOKEN_CATEGORY_END, _token_type_get_category(self->state.expect), TOKEN_CATEGORY_LPAREN
        );
        if (type == self->state.expect) break;

        int prec = token_info[type].infix_prec;
        if (prec < prec_limit) break;

        if (type == TOKEN_LPAREN) {
            _analyser_consume(self, 1);
            lhs = _analyser_read_expr_call(self, lhs);
            printf("outside call %zu\n", self->state.call_depth);
            if (!lhs) _analyser_error("pratt parser returned null");
            continue;
        }
        if (type == TOKEN_COMMA) {
            printf("comma found, %d, %zu\n", self->state.in_call, self->state.call_depth);
            if (!self->state.in_call)
                _analyser_error("comma outside of function call, function/class declaration or array");
            ++self->state.comma_count;
        }

        TextSect sect = self->token.start;

        _analyser_consume(self, 2);
        ASTNode * rhs = _analyser_read_expr_pratt(self, prec);
        if (!rhs) _analyser_error("pratt parser returned null");
        if (token_info[type].right_associative)
            lhs = _new_ast_node_operator(self->ast, token_info[type].op, rhs, lhs, sect);
        else
            lhs = _new_ast_node_operator(self->ast, token_info[type].op, lhs, rhs, sect);
    }

    prev_state.comma_count = self->state.comma_count;
    POP_STATE();
    return lhs;
}

FLUFF_PRIVATE_API ASTNode * _analyser_read_expr_call(Analyser * self, ASTNode * top) {
    PUSH_STATE();

    printf("in call %zu\n", self->state.call_depth);
    self->state.in_call = true;
    ASTNode * node = _new_ast_node_call(top->ast, top, self->state.comma_count + 1, self->position);
    if (_analyser_peek(self, 1).type != TOKEN_RPAREN) {
        _ast_node_append_child(node, _analyser_read_expr_pratt(self, 0));
    } else _analyser_consume(self, 1);

    POP_STATE();
    return node;
}

FLUFF_PRIVATE_API bool _analyser_expect_n(Analyser * self, size_t idx, const TokenCategory * categories, size_t count, int line, const char * func) {
    TokenCategory category = TOKEN_CATEGORY_EOF;

    if (idx < self->lexer->token_count)
        category = _token_type_get_category(self->lexer->tokens[idx].type);

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
    if (idx >= self->lexer->token_count)
        _analyser_error_print_d(line, func, "expected %.*s; got EOF", (int)len, buf);
    else
        _analyser_error_print_d(line, func, "expected %.*s; got %s '%.*s'", 
            (int)len, buf, _token_category_string(category), TOKEN_FMT(idx)
        );
    // TODO: this looks like garbage but works perfectly so maybe fix it idk
    return false;
}

FLUFF_PRIVATE_API Token _analyser_consume(Analyser * self, size_t n) {
    self->index += n;
    if (self->index >= self->lexer->token_count) {
        n           = self->lexer->token_count - self->index;
        self->token = _make_token(TOKEN_EOF);
        // lexer->location represents the last location in the source
        self->position = self->lexer->location;
    } else {
        self->token    = self->lexer->tokens[self->index];
        self->position = self->token.start;
    }
    return self->token;
}

FLUFF_PRIVATE_API Token _analyser_rewind(Analyser * self, size_t n) {
    if (n > self->index)
        _analyser_error("unexpected rewind");
    self->index   -= n;
    self->token    = self->lexer->tokens[self->index];
    self->position = self->token.start;
    return self->token;
}

FLUFF_PRIVATE_API Token _analyser_peek(Analyser * self, int offset) {
    int pos = ((int)self->index) + offset;
    if (pos >= ((int)self->lexer->token_count) || pos < 0)
        return _make_token(TOKEN_EOF);
        // _analyser_error("unexpected EOF");
    return self->lexer->tokens[pos];
}

FLUFF_PRIVATE_API Token _analyser_peekp(Analyser * self, size_t index) {
    if (index >= self->lexer->token_count)
        return _make_token(TOKEN_EOF);
        // _analyser_error("unexpected EOF");
    return self->lexer->tokens[index];
}

FLUFF_PRIVATE_API bool _analyser_is_expr_end(Analyser * self) {
    if (!_analyser_is_within_bounds(self))
        _analyser_error("unexpected EOF");
    return (!self->state.in_statement && self->token.type == TOKEN_END) ||
           (self->state.in_statement  && self->token.type == TOKEN_LBRACE);
}

FLUFF_PRIVATE_API bool _analyser_is_within_bounds(Analyser * self) {
    return self->index < self->lexer->token_count;
}