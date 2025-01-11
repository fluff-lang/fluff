/* -=============
     Includes
   =============- */

#define FLUFF_IMPLEMENTATION
#include <base.h>
#include <error.h>
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
    MAKE_INFO(EQUAL,          19, 0,  true,  EQUAL,     NONE),
    MAKE_INFO(DOT,            18, 0,  false, DOT,       NONE),
    MAKE_INFO(LBRACKET,       17, 0,  false, SUBSCRIPT, NONE),
    MAKE_INFO(LPAREN,         17, 0,  false, NONE,      NONE),
    MAKE_INFO(IN,             16, 0,  false, IN,        NONE),
    MAKE_INFO(AS,             15, 0,  false, AS,        NONE),
    MAKE_INFO(IS,             15, 0,  false, IS,        NONE),
    MAKE_INFO(NOT,            14, 14, true,  NONE,      NOT),
    MAKE_INFO(BIT_NOT,        14, 14, true,  NONE,      BIT_NOT),
    MAKE_INFO(MODULO,         13, 0,  false, MOD,       NONE),
    MAKE_INFO(POWER,          13, 0,  true,  POW,       NONE),
    MAKE_INFO(MULTIPLY,       12, 0,  false, MUL,       NONE),
    MAKE_INFO(DIVIDE,         12, 0,  false, DIV,       NONE),
    MAKE_INFO(PLUS,           11, 13, true,  ADD,       PROMOTE),
    MAKE_INFO(MINUS,          11, 13, true,  SUB,       NEGATE),
    MAKE_INFO(BIT_SHL,        10, 0,  false, BIT_SHL,   NONE),
    MAKE_INFO(BIT_SHR,        10, 0,  false, BIT_SHR,   NONE),
    MAKE_INFO(LESS,           10, 0,  false, LT,        NONE),
    MAKE_INFO(LESS_EQUALS,    10, 0,  false, LE,        NONE),
    MAKE_INFO(GREATER,        10, 0,  false, GT,        NONE),
    MAKE_INFO(GREATER_EQUALS, 9,  0,  false, GE,        NONE),
    MAKE_INFO(EQUALS,         9,  0,  false, EQ,        NONE),
    MAKE_INFO(NOT_EQUALS,     8,  0,  false, NE,        NONE),
    MAKE_INFO(BIT_AND,        7,  0,  false, BIT_AND,   NONE),
    MAKE_INFO(BIT_XOR,        6,  0,  false, BIT_XOR,   NONE),
    MAKE_INFO(BIT_OR,         5,  0,  false, BIT_OR,    NONE),
    MAKE_INFO(AND,            4,  0,  false, AND,       NONE),
    MAKE_INFO(OR,             3,  0,  false, OR,        NONE),
    MAKE_INFO(COMMA,          2,  0,  true,  COMMA,     NONE),

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

// This throws any type of log with some extra information
#define _analyser_log_d(__type, __line, __func, ...)\
        fluff_push_log_d(__type,\
            self->interpret->path, self->position.line + 1, self->position.column + 1,\
            __FILE__, __func, __line, __VA_ARGS__\
        )

// This throws any type of log
#define _analyser_log(__type, ...)\
        _analyser_log_d(__type, __LINE__, __func__, __VA_ARGS__)

// This throws errors
#define _analyser_error(...)\
        { _analyser_log(FLUFF_LOG_TYPE_ERROR, __VA_ARGS__); self->result = FLUFF_FAILURE; }

// This throws recoverable errors
#define _analyser_error_recoverable(...)\
        { _analyser_log(FLUFF_LOG_TYPE_ERROR, __VA_ARGS__); self->result = FLUFF_MAYBE_FAILURE; }

// This throws warnings
#define _analyser_warning(...)\
        { _analyser_log(FLUFF_LOG_TYPE_WARN, __VA_ARGS__); }

// This expects a token
#define _analyser_expect(__idx, ...) {\
            const TokenCategory __c[] = { __VA_ARGS__ };\
            _analyser_expect_n(self, __idx, __c, FLUFF_LENOF(__c), __LINE__, __func__);\
            if (self->result != FLUFF_OK)\
                FLUFF_BREAKPOINT();\
        }

// This makes the function fail and return
#define _analyser_failure()\
        POP_STATE(); return NULL;

// This checks the current result and fails depending on it
#define _analyser_check(__node, ...)\
        if (self->result != FLUFF_OK) { __VA_ARGS__; _free_ast_node(__node); _analyser_failure(); }

#define TOKEN_FMT(__index)\
        _lexer_token_string_len(self->lexer, __index), _lexer_token_string(self->lexer, __index)

#define TOKEN_TO_STRING_NODE()\
        _new_ast_node_string_n(self->ast,\
            &self->lexer->str[self->token.start.index], self->token.length, self->position\
        )

#define PUSH_STATE() AnalyserState prev_state = self->state; ++self->state.call_depth;
#define POP_STATE()  self->state = prev_state;

// TODO: make AST_TYPED_LABEL
// TODO: make sure certain types of nodes don't always accept NULL nodes to append
FLUFF_PRIVATE_API FluffResult _analyser_read(Analyser * self) {
    // Checkes if there is no nodes to look for
    if (self->lexer->token_count == 0) return FLUFF_OK;

    // Initializes the first token
    self->token = * self->lexer->tokens;
    self->index = 0;

    // Default result
    self->result = FLUFF_OK;

    // Scopes requires an expect, there is no '{}' so we just use EOF instead
    self->state.expect = TOKEN_EOF;

    // Reads the current scope and checkes for any errors
    // In case there isn't any, it will simply not append anything and return the result
    ASTNode * node = _analyser_read_scope(self);
    _ast_node_append_child(&self->ast->root, node);
    return self->result;
}

FLUFF_PRIVATE_API ASTNode * _analyser_read_token(Analyser * self) {
    // Checkes if we are out of bounds before reading
    if (!_analyser_is_within_bounds(self)) return NULL;
    PUSH_STATE();

    ASTNode * node = NULL;

    TokenCategory category = _token_type_get_category(self->token.type);
    switch (category) {
        case TOKEN_CATEGORY_FUNC_DECL: {
            // Functions
            // If the function has a name, then it's a named function
            if (_analyser_peek(self, 1).type == TOKEN_LABEL_LITERAL) {
                node = _analyser_read_func(self);
                break;
            }
            // Else, do a purposeful fallthrough, so we treat it as an expression
        }
        case TOKEN_CATEGORY_LITERAL:
        case TOKEN_CATEGORY_LABEL:
        case TOKEN_CATEGORY_LPAREN:
        case TOKEN_CATEGORY_LBRACKET:
        case TOKEN_CATEGORY_OPERATOR:
        case TOKEN_CATEGORY_OPERATOR_ARROW:
        case TOKEN_CATEGORY_OPERATOR_DOT:
        case TOKEN_CATEGORY_OPERATOR_ELLIPSIS: {
            // Operands
            node = _analyser_read_expr(self, TOKEN_END);
            break;
        }
        case TOKEN_CATEGORY_IF: {
            // If statements
            node = _analyser_read_if(self);
            break;
        }
        case TOKEN_CATEGORY_FOR: {
            // For loops
            node = _analyser_read_for(self);
            break;
        }
        case TOKEN_CATEGORY_WHILE: {
            // While loops
            node = _analyser_read_while(self);
            break;
        }
        case TOKEN_CATEGORY_DECL: {
            // Declarations (let, const)
            node = _analyser_read_decl(self);
            break;
        }
        case TOKEN_CATEGORY_CLASS_DECL: {
            // Class declarations
            node = _analyser_read_class(self);
            break;
        }
        case TOKEN_CATEGORY_LBRACE: {
            // Left brace
            self->state.expect = TOKEN_RBRACE;
            _analyser_consume(self, 1);
            node = _analyser_read_scope(self);
            break;
        }
        case TOKEN_CATEGORY_CONTROL_KEYWORD: {
            // Any control keyword, like 'return', 'break' or 'continue'
            node = _analyser_read_control(self);
            break;
        }
        case TOKEN_CATEGORY_END: {
            // Does nothing, it's ';' after all
            break;
        }
        case TOKEN_CATEGORY_RPAREN:
        case TOKEN_CATEGORY_RBRACE:
        case TOKEN_CATEGORY_RBRACKET: { 
            // If any of these are found, then there is an extraneous token
            _analyser_error("extraneous %s '%.*s'", 
                _token_category_string(category), TOKEN_FMT(self->index)
            );
            _analyser_failure();
        }
        case TOKEN_CATEGORY_ELSE: {
            // You can't have an else statement without an if statement, duh
            _analyser_error("'else' statement out of 'if' statement");
            _analyser_failure();
        }
        default: {
            // If none of these tokens are valid then it will throw an error
            _analyser_error("unexpected token '%.*s'", TOKEN_FMT(self->index));
            _analyser_failure();
        }
    }

    _analyser_check(node);

    // Remember, end statements return NULL!
    if (category != TOKEN_CATEGORY_END && self->result == FLUFF_OK) {
        _ast_node_append_child(self->state.last_scope, node);
    }

    POP_STATE();
    return node;
}

FLUFF_PRIVATE_API ASTNode * _analyser_read_scope(Analyser * self) {
    PUSH_STATE();

    ASTNode * node = _new_ast_node(self->ast, AST_NODE_SCOPE, self->position);
    
    // TODO: don't create a new scope in case we are appending to root
    // Sets the latest scope to the current node
    self->state.last_scope = node;
    while (_analyser_is_within_bounds(self)) {
        // If the current token is the expected token, stop immediately
        if (self->token.type == self->state.expect) break;
        
        // Reads the current token, if there was any errors, stop immediately
        _analyser_read_token(self);
        _analyser_check(node);

        // Consumes 1 for the next token
        _analyser_consume(self, 1);
    }
    // We expect it to only break on the expected token, if not then it's certainly EOF
    _analyser_expect(self->index, _token_type_get_category(self->state.expect));
    _analyser_check(node);

    POP_STATE();
    return node;
}

FLUFF_PRIVATE_API ASTNode * _analyser_read_if(Analyser * self) {
    PUSH_STATE();

    // Makes sure that the nodes do not append to the last scope
    self->state.last_scope = NULL;

    ASTNode * node = _new_ast_node(self->ast, AST_NODE_IF, self->position);

    // Checkes for the next token, which should be an expression
    // No _analyser_expect() because _analyser_read_expr() already expects for us
    _analyser_consume(self, 1);
    _ast_node_append_child(node, _analyser_read_expr(self, TOKEN_LBRACE));
    _analyser_check(node);

    // Checkes for the next token, which should be an open brace
    _analyser_expect(self->index + 1, TOKEN_CATEGORY_LBRACE);
    _analyser_check(node);

    // In case it is, consume out of the expression and the left brace
    // If we parse the left brace too, this will create 2 scopes unnecessarily
    _analyser_consume(self, 2);
    self->state.expect = TOKEN_RBRACE;
    _ast_node_append_child(node, _analyser_read_scope(self));
    _analyser_check(node);

    // Checkes for the next token, which can be an else statement
    if (_analyser_peek(self, 1).type == TOKEN_ELSE) {
        _analyser_consume(self, 2);
        
        // If the token is matched, then it will append and check it normally
        // If the token is unmatched, then it will fallthrough and return an error
        _analyser_expect(self->index, TOKEN_CATEGORY_IF, TOKEN_CATEGORY_LBRACE);
        _analyser_check(node);
        if (self->result == FLUFF_OK) {
            _ast_node_append_child(node, _analyser_read_token(self));
            _analyser_check(node);
        }
    }
    
    POP_STATE();
    return node;
}

FLUFF_PRIVATE_API ASTNode * _analyser_read_for(Analyser * self) {
    // TODO: this
    PUSH_STATE();
    _analyser_failure();
}

FLUFF_PRIVATE_API ASTNode * _analyser_read_while(Analyser * self) {
    PUSH_STATE();

    // Makes sure the nodes don't get appended to the last scope
    self->state.last_scope = NULL;

    ASTNode * node = _new_ast_node(self->ast, AST_NODE_WHILE, self->position);
    
    // Checkes for the next token, which should be an expression
    _analyser_consume(self, 1);
    _ast_node_append_child(node, _analyser_read_expr(self, TOKEN_LBRACE));
    _analyser_check(node);

    // Checkes for the next token, which should be an open brace
    _analyser_expect(self->index + 1, TOKEN_CATEGORY_LBRACE);
    _analyser_check(node);

    // In case it is, consume out of the expression and the left brace
    // If we parse the left brace too, this will create 2 scopes unnecessarily
    _analyser_consume(self, 2);
    self->state.expect = TOKEN_RBRACE;
    _ast_node_append_child(node, _analyser_read_scope(self));
    _analyser_check(node);
    
    POP_STATE();
    return node;
}

FLUFF_PRIVATE_API ASTNode * _analyser_read_decl(Analyser * self) {
    PUSH_STATE();

    ASTNode * node = _new_ast_node(self->ast, AST_NODE_DECLARATION, self->position);
    
    // Checkes if the token is "let" or "const", since "const" is radically different than "let"
    node->data.decl.is_constant = (self->token.type == TOKEN_CONST);

    // Checkes if the token is a label, because we need to append to one, duh
    _analyser_consume(self, 1);
    _analyser_expect(self->index, TOKEN_CATEGORY_LABEL);
    _analyser_check(node);

    // Checkes if the next token ends the expression
    if (_analyser_peek(self, 1).type == TOKEN_END) {
        // TODO: check if this will only throw a warning
        _analyser_warning("cannot declare a variable without implying or specifying it's type");
    } else {
        // Checkes if there is any type declaration after the label
        self->state.in_decl = (_analyser_peek(self, 1).type == TOKEN_COLON);
        _ast_node_append_child(node, _analyser_read_expr(self, TOKEN_END));
        _analyser_check(node);
        if (self->state.in_decl) {
            // The type is considered an extra return and is appended AFTER the expression
            _ast_node_append_child(node, self->extra_return);
            self->extra_return = NULL;
        }
    }

    // TODO: maybe checking twice is a bad idea?
    _analyser_check(node);

    POP_STATE();
    return node;
}

FLUFF_PRIVATE_API ASTNode * _analyser_read_func(Analyser * self) {
    PUSH_STATE();

    ASTNode * node = _new_ast_node(self->ast, AST_NODE_FUNCTION, self->position);

    // Checkes if the next token is either a label or a left parenthesis
    _analyser_consume(self, 1);
    _analyser_expect(self->index, TOKEN_CATEGORY_LABEL, TOKEN_CATEGORY_LPAREN);
    _analyser_check(node);

    if (self->token.type == TOKEN_LABEL_LITERAL) {
        if (self->state.in_expr) {
            _analyser_error("cannot declare a named function within an expression");
            _analyser_check(node);
        }

        // In case it's a label, append it to the current node as the function's name
        _ast_node_append_child(node, TOKEN_TO_STRING_NODE());
        _analyser_check(node);
        _analyser_consume(self, 1);

        // TODO: functions with names should be considered as anonymous outside of classes
        //       and the global scope
    }

    _analyser_expect(self->index, TOKEN_CATEGORY_LPAREN);
    _analyser_check(node);
    
    // In case the function has arguments
    if (_analyser_peek(self, 1).type != TOKEN_RPAREN) {
        // Makes sure the type parser knows we are inside a call type
        self->state.in_call = true;
        while (_analyser_is_within_bounds(self)) {
            // Checkes if the function has ended
            if (self->token.type == TOKEN_RPAREN || self->token.type == self->state.expect)
                break;
            _analyser_consume(self, 1);

            // Checkes if we have a label, this will be the argument name
            // If yes, append it to the function node 
            _analyser_expect(self->index, TOKEN_CATEGORY_LABEL);
            _analyser_check(node);

            _ast_node_append_child(node, TOKEN_TO_STRING_NODE());
            _analyser_check(node);

            _analyser_consume(self, 1);

            // Checkes if we have a colon
            // If yes, we found a type
            if (self->token.type == TOKEN_COLON) {
                _analyser_consume(self, 1);
                _ast_node_append_child(node, _analyser_read_type(self, self->state.expect));
                _analyser_check(node);
            }

            // Checkes if we found the argument end or the function end
            _analyser_expect(self->index, 
                TOKEN_CATEGORY_OPERATOR_COMMA, TOKEN_CATEGORY_RPAREN
            );
            _analyser_check(node);
        }
        // Toggles the call flag off
        self->state.in_call = false;
    } else {
        _analyser_consume(self, 1);
    }

    // Checkes if we found the function parameters end
    _analyser_expect(self->index, TOKEN_CATEGORY_RPAREN);
    _analyser_check(node);

    // Checkes if the next node is an arrow, that is our function return type
    _analyser_consume(self, 1);
    if (self->token.type == TOKEN_ARROW) {
        _analyser_consume(self, 1);
        _ast_node_append_child(node, _analyser_read_type(self, self->state.expect));
        _analyser_check(node);
    }

    // TODO: classes don't need well-defined functions 
    // Checkes if there is a function scope
    _analyser_expect(self->index, TOKEN_CATEGORY_LBRACE);
    _analyser_check(node);

    _analyser_consume(self, 1);
    self->state.expect = TOKEN_RBRACE;
    _ast_node_append_child(node, _analyser_read_scope(self));
    _analyser_check(node);
    // _analyser_consume(self, 1);

    POP_STATE();
    return node;
}

FLUFF_PRIVATE_API ASTNode * _analyser_read_class(Analyser * self) {
    // TODO: this
    PUSH_STATE();
    _analyser_failure();
}

FLUFF_PRIVATE_API ASTNode * _analyser_read_type(Analyser * self, TokenType expect) {
    PUSH_STATE();

    bool had_error = false;

    ASTNode * node = _new_ast_node(self->ast, AST_NODE_TYPE, self->position);

    // Checkes if we actually have a type
    if (self->token.type == TOKEN_END) {
        _analyser_error("missing type");
        _analyser_check(node);
    }

    // Checkes if the type expression ended
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
            _analyser_check(node);
            break;
        }
        case TOKEN_LBRACKET: {
            _analyser_consume(self, 1);
            node->data.type = AST_TYPE_ARRAY;
            if (_analyser_peek(self, 1).type != TOKEN_RBRACKET) {
                _ast_node_append_child(node, _analyser_read_type(self, expect));
                _analyser_check(node);
            }
            _analyser_expect(self->index, TOKEN_CATEGORY_RBRACKET);
            _analyser_check(node);
            break;
        }
        case TOKEN_FUNC: {
            _analyser_consume(self, 1);
            _analyser_expect(self->index, TOKEN_CATEGORY_LPAREN);
            _analyser_check(node);
            node->data.type = AST_TYPE_FUNC;
            
            self->state.in_call = true;
            if (_analyser_peek(self, 1).type != TOKEN_RPAREN) {
                while (_analyser_is_within_bounds(self)) {
                    if (self->token.type == TOKEN_RPAREN || self->token.type == expect) break;
                    _analyser_consume(self, 1);

                    _ast_node_append_child(node, _analyser_read_type(self, expect));
                    _analyser_check(node);

                    _analyser_expect(self->index, 
                        TOKEN_CATEGORY_OPERATOR_COMMA, TOKEN_CATEGORY_RPAREN
                    );
                    _analyser_check(node);
                }
            } else {
                _analyser_consume(self, 1);
            }

            _analyser_expect(self->index, TOKEN_CATEGORY_RPAREN);
            _analyser_check(node);

            self->state.in_call = false;
            if (_analyser_peek(self, 1).type == TOKEN_ARROW) {
                _analyser_consume(self, 2);
                _ast_node_append_child(node, _analyser_read_type(self, expect));
                _analyser_check(node);
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
    if (had_error) {
        _analyser_error("expected type, got %s '%.*s'", 
            _token_category_string(_token_type_get_category(self->token.type)), 
            TOKEN_FMT(self->index)
        );
        _analyser_check(node);
    }

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

FLUFF_PRIVATE_API ASTNode * _analyser_read_array(Analyser * self, TokenType expect) {
    PUSH_STATE();

    ASTNode * node = _new_ast_node(self->ast, AST_NODE_ARRAY_LITERAL, self->position);

    self->state.in_array = true;

    _analyser_consume(self, 1);
    if (self->token.type != TOKEN_RBRACKET) {
        // Appends an expression to the array
        _ast_node_append_child(node, _analyser_read_expr(self, TOKEN_RBRACKET));
        _analyser_check(node);
    } else {
        _analyser_consume(self, 1);
    }

    POP_STATE();
    return node;
}

FLUFF_PRIVATE_API ASTNode * _analyser_read_expr(Analyser * self, TokenType expect) {
    PUSH_STATE();
    self->state.in_call      = false;
    self->state.in_subscript = false;
    self->state.expect       = expect;
    ASTNode * node = _analyser_read_expr_pratt(self, 0);
    _analyser_check(node);
    POP_STATE();
    return node;
}

FLUFF_PRIVATE_API ASTNode * _analyser_read_expr_pratt(Analyser * self, int prec_limit) {
    // TODO: make arrays and calls not use AST_OPERATOR_COMMA so we use less memory
    PUSH_STATE();

    if (!_analyser_is_within_bounds(self)) {
        _analyser_error("expected expression, got EOF");
        _analyser_failure();
    }

    self->state.in_decl    = false;
    self->state.last_scope = NULL;

    TokenCategory category = _token_type_get_category(self->token.type);
    ASTNode * lhs = NULL;
    switch (category) {
        case TOKEN_CATEGORY_LITERAL: 
        case TOKEN_CATEGORY_LABEL: {
            // Operand
            lhs = _new_ast_node_from_token(self->lexer, self->index, self->ast, self->position);
            _analyser_check(lhs);
            break;
        }
        case TOKEN_CATEGORY_FUNC_DECL: {
            // Function declaration
            lhs = _analyser_read_func(self);
            _analyser_check(lhs);
            break;
        }
        case TOKEN_CATEGORY_LPAREN: {
            // Parenthesis
            _analyser_consume(self, 1);

            PUSH_STATE();
            self->state.expect = TOKEN_RPAREN;
            lhs = _analyser_read_expr_pratt(self, 0);
            _analyser_check(lhs);

            _analyser_consume(self, 1);
            _analyser_expect(self->index, TOKEN_CATEGORY_RPAREN);
            _analyser_check(lhs);

            POP_STATE();
            if (self->state.in_call) return lhs;
            break;
        }
        case TOKEN_CATEGORY_LBRACKET: {
            // Arrays
            lhs = _analyser_read_array(self, self->state.expect);
            _analyser_check(lhs);
            _analyser_consume(self, 1);
            _analyser_expect(self->index, TOKEN_CATEGORY_RBRACKET);
            _analyser_consume(self, 1);
            break;
        }
        case TOKEN_CATEGORY_OPERATOR: {
            // Unary operator
            TokenType op_type = self->token.type;
            if (token_info[op_type].unary_op == AST_OPERATOR_NONE) {
                _analyser_error("operator '%.*s' is not unary", TOKEN_FMT(self->index));
                _analyser_check(lhs);
            }
            _analyser_consume(self, 1);

            ASTNode * rhs = _analyser_read_expr_pratt(self, token_info[op_type].prefix_prec);
            _analyser_check(rhs, _free_ast_node(lhs));

            lhs = _new_ast_node_unary_operator(self->ast, 
                token_info[op_type].unary_op, 
                rhs, self->position
            );
            _analyser_check(lhs);
            break;
        }
        default: {
            _analyser_error("expected expression, got %s '%.*s'", 
                _token_category_string(category), TOKEN_FMT(self->index)
            );
            _analyser_check(lhs);
        }
    }

    self->state.in_expr = true;

    if (prev_state.in_decl) {
        _analyser_error("sorry, type checking is disabled temporarily due to implementation issues");
        _analyser_check(lhs);

        _analyser_expect(self->index + 1, TOKEN_CATEGORY_OPERATOR_COLON, TOKEN_CATEGORY_RPAREN);
        _analyser_check(lhs);

        _analyser_consume(self, 1);
        if (self->token.type == TOKEN_RPAREN && !prev_state.in_call) {
            _analyser_error("unexpected %s '%.*s' in type declaration", 
                _token_category_string(_token_type_get_category(self->token.type)), TOKEN_FMT(self->index)
            );
            _analyser_check(lhs);
        }
        _analyser_consume(self, 1);

        self->extra_return = _analyser_read_type(self, TOKEN_EQUAL);
        _analyser_check(self->extra_return, _free_ast_node(lhs));
        _analyser_rewind(self, 1);
    }

    while (!_analyser_is_expr_end(self)) {
        // Checks if everything is OK before parsing
        _analyser_check(lhs);

        TokenType type = _analyser_peek(self, 1).type;
        _analyser_expect(self->index + 1, 
            TOKEN_CATEGORY_OPERATOR, TOKEN_CATEGORY_OPERATOR_DOT, TOKEN_CATEGORY_OPERATOR_COMMA, 
            TOKEN_CATEGORY_END, _token_type_get_category(self->state.expect), 
            TOKEN_CATEGORY_LPAREN, TOKEN_CATEGORY_LBRACKET
        );
        _analyser_check(lhs);
        if (type == self->state.expect) break;

        int prec = token_info[type].infix_prec;
        if (prec < prec_limit) break;

        ASTNode * rhs = NULL;

        if (type == TOKEN_LPAREN) {
            _analyser_consume(self, 1);
            lhs = _analyser_read_expr_call(self, lhs);
            _analyser_check(lhs);
            continue;
        } else if (type == TOKEN_LBRACKET) {
            _analyser_consume(self, 1);
            lhs = _analyser_read_expr_subscript(self, lhs);
            _analyser_check(lhs);
            continue;
        } else {
            if (type == TOKEN_COMMA) {
                if (!self->state.in_call && !self->state.in_array) {
                    _analyser_error_recoverable(
                        "comma outside of function call or array"
                    );
                }
                ++self->state.comma_count;
            }

            TextSect sect = self->token.start;

            _analyser_consume(self, 2);
            rhs = _analyser_read_expr_pratt(self, prec);
            _analyser_check(rhs, _free_ast_node(lhs));

            if (token_info[type].right_associative)
                lhs = _new_ast_node_operator(self->ast, token_info[type].op, rhs, lhs, sect);
            else
                lhs = _new_ast_node_operator(self->ast, token_info[type].op, lhs, rhs, sect);
        }
    }

    prev_state.comma_count = self->state.comma_count;
    POP_STATE();
    return lhs;
}

FLUFF_PRIVATE_API ASTNode * _analyser_read_expr_call(Analyser * self, ASTNode * top) {
    PUSH_STATE();

    self->state.in_call = true;
    ASTNode * node = _new_ast_node_call(top->ast, top, self->state.comma_count + 1, self->position);
    if (_analyser_peek(self, 1).type != TOKEN_RPAREN) {
        _ast_node_append_child(node, _analyser_read_expr_pratt(self, 0));
        _analyser_check(node);
    } else _analyser_consume(self, 1);

    POP_STATE();
    return node;
}

FLUFF_PRIVATE_API ASTNode * _analyser_read_expr_subscript(Analyser * self, ASTNode * top) {
    PUSH_STATE();

    _analyser_consume(self, 1);

    self->state.in_subscript = true;
    self->state.expect       = TOKEN_RBRACKET;
    ASTNode * node = _analyser_read_expr_pratt(self, 0);
    _analyser_check(node);
    _analyser_consume(self, 1);
    _analyser_expect(self->index, TOKEN_CATEGORY_RBRACKET);

    node = _new_ast_node_operator(top->ast, AST_OPERATOR_SUBSCRIPT, top, node, self->position);

    POP_STATE();
    return node;
}

FLUFF_PRIVATE_API bool _analyser_expect_n(Analyser * self, size_t idx, const TokenCategory * categories, size_t count, int line, const char * func) {
    // TODO: this looks like garbage but works perfectly so maybe fix it idk
    // TODO: pretend the current token is the expected one so we can be more resilient on parsing

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
        _analyser_log_d(FLUFF_LOG_TYPE_ERROR, line, func, "expected %.*s; got EOF", (int)len, buf);
    else
        _analyser_log_d(FLUFF_LOG_TYPE_ERROR, line, func, "expected %.*s; got %s '%.*s'", 
            (int)len, buf, _token_category_string(category), TOKEN_FMT(idx)
        );
    self->result = FLUFF_FAILURE;
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
        fluff_panic("unexpected rewind");
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