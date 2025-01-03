/* -=============
     Includes
   =============- */

#define FLUFF_IMPLEMENTATION
#include <base.h>
#include <error.h>
#include <parser/lexer.h>
#include <parser/ast.h>
#include <core/config.h>

/* -==============
     Internals
   ==============- */

// This throws any type of log with some extra information
#define _ast_log_d(__type, __line, __func, ...)\
        fluff_push_log_d(__type,\
            self->interpret->path, self->position.line + 1, self->position.column + 1,\
            __FILE__, __func, __line, __VA_ARGS__\
        )

// This throws any type of log
#define _ast_log(__type, ...)\
        _ast_log_d(__type, __LINE__, __func__, __VA_ARGS__)

// This throws errors
#define _ast_error(...)\
        { _ast_log(FLUFF_LOG_TYPE_ERROR, __VA_ARGS__); visitor->result = FLUFF_FAILURE; return false; }

// This throws recoverable errors
#define _ast_error_recoverable(...)\
        { _ast_log(FLUFF_LOG_TYPE_ERROR, __VA_ARGS__); visitor->result = FLUFF_MAYBE_FAILURE; }

// This throws warnings
#define _ast_warning(...)\
        { _ast_log(FLUFF_LOG_TYPE_WARN, __VA_ARGS__); }

#define APPEND_OPCODE(name) printf(#name "\n")

FLUFF_CONSTEXPR bool _solve_operator_pre(ASTNode * self, ASTNodeVisitor * visitor) {
    switch (self->data.op) {
        case AST_OPERATOR_ADD:
            { APPEND_OPCODE(add); break; }
        case AST_OPERATOR_SUB:
            { APPEND_OPCODE(sub); break; }
        case AST_OPERATOR_MUL:
            { APPEND_OPCODE(mul); break; }
        case AST_OPERATOR_DIV:
            { APPEND_OPCODE(div); break; }
        case AST_OPERATOR_MOD:
            { APPEND_OPCODE(mod); break; }
        case AST_OPERATOR_POW:
            { APPEND_OPCODE(pow); break; }
        case AST_OPERATOR_EQ:
            { APPEND_OPCODE(eq); break; }
        case AST_OPERATOR_NE:
            { APPEND_OPCODE(ne); break; }
        case AST_OPERATOR_GT:
            { APPEND_OPCODE(gt); break; }
        case AST_OPERATOR_GE:
            { APPEND_OPCODE(ge); break; }
        case AST_OPERATOR_LT:
            { APPEND_OPCODE(lt); break; }
        case AST_OPERATOR_LE:
            { APPEND_OPCODE(le); break; }
        case AST_OPERATOR_AND:
            { APPEND_OPCODE(and); break; }
        case AST_OPERATOR_OR:
            { APPEND_OPCODE(or); break; }
        case AST_OPERATOR_BIT_AND:
            { APPEND_OPCODE(bit_and); break; }
        case AST_OPERATOR_BIT_OR:
            { APPEND_OPCODE(bit_or); break; }
        case AST_OPERATOR_BIT_XOR:
            { APPEND_OPCODE(bit_xor); break; }
        case AST_OPERATOR_BIT_SHL:
            { APPEND_OPCODE(bit_shl); break; }
        case AST_OPERATOR_BIT_SHR:
            { APPEND_OPCODE(bit_shr); break; }
        case AST_OPERATOR_IS:
            { APPEND_OPCODE(is); break; }
        case AST_OPERATOR_AS:
            { APPEND_OPCODE(as); break; }
        case AST_OPERATOR_DOT:
            { APPEND_OPCODE(dot); break; }
        case AST_OPERATOR_EQUAL:
            { APPEND_OPCODE(equal); break; }
        case AST_OPERATOR_NOT:
            { APPEND_OPCODE(not); break; }
        case AST_OPERATOR_BIT_NOT:
            { APPEND_OPCODE(bit_not); break; }
        case AST_OPERATOR_PROMOTE:
            { APPEND_OPCODE(promote); break; }
        case AST_OPERATOR_NEGATE:
            { APPEND_OPCODE(negate); break; }
        default: APPEND_OPCODE(nop);
    }
    return true;
}

FLUFF_CONSTEXPR bool _solve_operator_post(ASTNode * self, ASTNodeVisitor * visitor) {
    // TODO: equal operator
    if (self->data.op == AST_OPERATOR_SUBSCRIPT)
        APPEND_OPCODE(get_element);
    return true;
}

FLUFF_CONSTEXPR bool _solve_call_post(ASTNode * self, ASTNodeVisitor * visitor) {
    if (!self->parent) return false;
    printf("call %d", 
        (self->parent->last_child ? (int)self->parent->last_child->index + 1 : 0)
    );
    return true;
}

FLUFF_CONSTEXPR bool _solve_literal_pre(ASTNode * self, ASTNodeVisitor * visitor) {
    switch (self->type) {
        case AST_NODE_BOOL_LITERAL: {
            if (self->data.bool_literal) APPEND_OPCODE(push_true);
            else                         APPEND_OPCODE(push_false);
        }
        case AST_NODE_INT_LITERAL: {
            printf("push_int %ld", self->data.int_literal);
            break;
        }
        case AST_NODE_FLOAT_LITERAL: {
            printf("push_float %f", self->data.float_literal);
            break;
        }
        case AST_NODE_STRING_LITERAL: {
            printf("push_string '%s'", self->data.string_literal.data);
            break;
        }
        default: FLUFF_UNREACHABLE();
    }
    return true;
}

FLUFF_CONSTEXPR bool _solve_array_post(ASTNode * self, ASTNodeVisitor * visitor) {
    if (!self->parent) return false;
    printf("push_array %d", 
        (self->parent->last_child ? (int)self->parent->last_child->index + 1 : 0)
    );
    return true;
}

FLUFF_CONSTEXPR bool _solve_label_pre(ASTNode * self, ASTNodeVisitor * visitor) {
    printf("get_local '%.*s'", (int)self->data.string_literal.length, self->data.string_literal.data);
    return true;
}

typedef struct ASTNodeInfo {
    const char * name;
    size_t max_children;

    ASTNodeVisitCallback pre_call;
    ASTNodeVisitCallback post_call;
} ASTNodeInfo;

#define MAKE_INFO(type, maxc, pre, post)\
        [AST_NODE_##type] = (ASTNodeInfo){ #type, maxc, pre, post },

FLUFF_CONSTEXPR_V ASTNodeInfo node_info[] = {
    MAKE_INFO(NONE,           0,        NULL,                NULL)
    MAKE_INFO(OPERATOR,       2,        _solve_operator_pre, _solve_operator_post)
    MAKE_INFO(UNARY_OPERATOR, 1,        NULL,                NULL)
    MAKE_INFO(CALL,           SIZE_MAX, NULL,                _solve_call_post)
    MAKE_INFO(BOOL_LITERAL,   0,        _solve_literal_pre,  NULL)
    MAKE_INFO(INT_LITERAL,    0,        _solve_literal_pre,  NULL)
    MAKE_INFO(FLOAT_LITERAL,  0,        _solve_literal_pre,  NULL)
    MAKE_INFO(STRING_LITERAL, 0,        _solve_literal_pre,  NULL)
    MAKE_INFO(ARRAY_LITERAL,  0,        NULL,                _solve_array_post)
    MAKE_INFO(LABEL_LITERAL,  0,        _solve_label_pre,    NULL)
    MAKE_INFO(SCOPE,          SIZE_MAX, NULL, NULL)
    MAKE_INFO(IF,             3,        NULL, NULL)
    MAKE_INFO(FOR,            4,        NULL, NULL)
    MAKE_INFO(WHILE,          2,        NULL, NULL)
    MAKE_INFO(DECLARATION,    2,        NULL, NULL)
    MAKE_INFO(FUNCTION,       0,        NULL, NULL)
    MAKE_INFO(CLASS,          0,        NULL, NULL)
    MAKE_INFO(RETURN,         1,        NULL, NULL)
    MAKE_INFO(BREAK,          0,        NULL, NULL)
    MAKE_INFO(CONTINUE,       0,        NULL, NULL)
    MAKE_INFO(TYPE,           0,        NULL, NULL)
    MAKE_INFO(ANNOTATION,     0,        NULL, NULL)
};

FLUFF_CONSTEXPR bool _ast_node_dump_callback(ASTNode * self, ASTNodeVisitor * visitor) {
    char tree[visitor->depth];
    size_t ident = 0;
    ASTNode * node = self;
    while (node != NULL && ident < visitor->depth) {
        tree[ident++] = (node->parent && node->parent->sibling ? '|' : ' ');
        node = node->parent;
    }
    while (ident != 0) {
        printf("%c ", tree[--ident]);
    }
    printf("|-[%zu]: %s ", self->index, node_info[self->type].name);

    switch (self->type) {
        case AST_NODE_BOOL_LITERAL: {
            printf("<%s>", FLUFF_BOOLALPHA(self->data.bool_literal));
            break;
        }
        case AST_NODE_INT_LITERAL: {
            printf("<%ld>", self->data.int_literal);
            break;
        }
        case AST_NODE_FLOAT_LITERAL: {
            printf("<%f>", self->data.float_literal);
            break;
        }
        case AST_NODE_STRING_LITERAL:
        case AST_NODE_LABEL_LITERAL: {
            printf("<%.*s>", (int)self->data.string_literal.length, self->data.string_literal.data);
            break;
        }
        default: break;
    }

    putchar('\n');
    return true;
}

FLUFF_CONSTEXPR bool _ast_node_free_callback(ASTNode * self, ASTNodeVisitor * visitor) {
    if (!self) return false;
    if (&self->ast->root == self) return true; // Avoiding free if the value is the root
    FLUFF_CLEANUP(self);
    fluff_free(self);
    return true;
}

FLUFF_CONSTEXPR bool _ast_node_solve_pre_callback(ASTNode * self, ASTNodeVisitor * visitor) {
    if (node_info[self->type].pre_call) {
        bool res = node_info[self->type].pre_call(self, visitor);
        if (res) putchar('\n');
        return res;
    }
    return true;
}

FLUFF_CONSTEXPR bool _ast_node_solve_post_callback(ASTNode * self, ASTNodeVisitor * visitor) {
    if (node_info[self->type].post_call) {
        bool res = node_info[self->type].post_call(self, visitor);
        if (res) putchar('\n');
        return res;
    }
    return true;
}

/* -============
     ASTNode
   ============- */

// TODO: proper error handling for these functions
// TODO: proper hashing for nodes

FLUFF_PRIVATE_API ASTNode * _new_ast_node(AST * ast, ASTNodeType type, TextSect loc) {
    ASTNode * self = fluff_alloc(NULL, sizeof(ASTNode));
    FLUFF_CLEANUP(self);
    self->type = type;
    self->loc  = loc;

    // Increases the line and column by 1 so we have "line 1 col 1" instead of "line 0 col 0" 
    ++self->loc.line;
    ++self->loc.column;
    //printf("allocated %p\n", self);
    return self;
}

FLUFF_PRIVATE_API ASTNode * _new_ast_node_bool(AST * ast, FluffBool v, TextSect loc) {
    ASTNode * self = _new_ast_node(ast, AST_NODE_BOOL_LITERAL, loc);
    self->data.bool_literal = v;

    self->flags = FLUFF_SET_FLAG(self->flags, AST_NODE_FOLDABLE);
    self->hash  = fluff_hash(&v, sizeof(v));
    return self;
}

FLUFF_PRIVATE_API ASTNode * _new_ast_node_int(AST * ast, FluffInt v, TextSect loc) {
    ASTNode * self = _new_ast_node(ast, AST_NODE_INT_LITERAL, loc);
    self->data.int_literal = v;

    self->flags = FLUFF_SET_FLAG(self->flags, AST_NODE_FOLDABLE);
    self->hash  = fluff_hash(&v, sizeof(v));
    return self;
}

FLUFF_PRIVATE_API ASTNode * _new_ast_node_float(AST * ast, FluffFloat v, TextSect loc) {
    ASTNode * self = _new_ast_node(ast, AST_NODE_FLOAT_LITERAL, loc);
    self->data.float_literal = v;

    self->flags = FLUFF_SET_FLAG(self->flags, AST_NODE_FOLDABLE);
    self->hash  = fluff_hash(&v, sizeof(v));
    return self;
}

FLUFF_PRIVATE_API ASTNode * _new_ast_node_string(AST * ast, const char * str, TextSect loc) {
    return _new_ast_node_string_n(ast, str, (str ? strlen(str) : 0), loc);
}

FLUFF_PRIVATE_API ASTNode * _new_ast_node_string_n(AST * ast, const char * str, size_t len, TextSect loc) {
    ASTNode * self = _new_ast_node(ast, AST_NODE_STRING_LITERAL, loc);
    _new_string_n(&self->data.string_literal, str, len);
    self->flags = FLUFF_SET_FLAG(self->flags, AST_NODE_FOLDABLE);
    self->hash  = fluff_hash(str, len);
    return self;
}

FLUFF_PRIVATE_API ASTNode * _new_ast_node_literal(AST * ast, const char * str, size_t len, TextSect loc) {
    ASTNode * self = _new_ast_node_string_n(ast, str, len, loc);
    self->type  = AST_NODE_LABEL_LITERAL;
    self->flags = FLUFF_UNSET_FLAG(self->flags, AST_NODE_FOLDABLE);
    return self;
}

FLUFF_PRIVATE_API ASTNode * _new_ast_node_operator(AST * ast, ASTOperatorType op, ASTNode * lhs, ASTNode * rhs, TextSect loc) {
    ASTNode * self = _new_ast_node(ast, AST_NODE_OPERATOR, loc);
    // Appends each arm to the operator
    _ast_node_append_child(self, lhs);
    _ast_node_append_child(self, rhs);

    // Makes the current node foldable according to lhs and rhs
    self->flags = FLUFF_SET_FLAG(self->flags, 
        (lhs ? FLUFF_HAS_FLAG(lhs->flags, AST_NODE_FOLDABLE) : 0) | 
        (rhs ? FLUFF_HAS_FLAG(rhs->flags, AST_NODE_FOLDABLE) : 0)
    );
    return self;
}

FLUFF_PRIVATE_API ASTNode * _new_ast_node_unary_operator(AST * ast, ASTOperatorType op, ASTNode * expr, TextSect loc) {
    ASTNode * self = _new_ast_node_operator(ast, op, expr, NULL, loc);
    self->type = AST_NODE_UNARY_OPERATOR;
    return self;
}

FLUFF_PRIVATE_API ASTNode * _new_ast_node_call(AST * ast, ASTNode * nodes, size_t count, TextSect loc) {
    // TODO: okay so what do I even do with "count" now that I rewrote the AST?????
    ASTNode * self = _new_ast_node(ast, AST_NODE_CALL, loc);
    _ast_node_append_child(self, nodes);
    nodes->parent = self;
    return self;
}

FLUFF_PRIVATE_API void _free_ast_node(ASTNode * self) {
    // Freeing backwards so we don't have a "use after free" exploit
    ASTNodeVisitor visitor = {
        .root      = self, 
        .post_call = _ast_node_free_callback, 
    };
    _ast_node_visit(self, &visitor);
    // printf("freed %p\n", self);
}

FLUFF_PRIVATE_API ASTNode * _ast_node_append_child(ASTNode * self, ASTNode * node) {
    if (!node) return NULL;

    // If there is too many nodes, immediately quit
    const size_t max = node_info[self->type].max_children;
    if (max != 0 && self->last_child && self->last_child->index == max - 1)
        return NULL;

    node->parent = self;

    // Sets the child to be the last one and sets their index up
    size_t last_index = 0;
    if (self->last_child) {
        last_index                = self->last_child->index + 1;
        self->last_child->sibling = node;
    } else self->child = node;
    
    node->index      = last_index;
    self->last_child = node;
    return node;
}

FLUFF_PRIVATE_API size_t _ast_node_get_children_count(ASTNode * self) {
    return (self->parent && self->parent->last_child ? self->parent->last_child->index + 1 : 0);
}

FLUFF_PRIVATE_API bool _ast_node_visit(ASTNode * self, ASTNodeVisitor * visitor) {
    if (!self) return false;

    if (visitor->pre_call && !visitor->pre_call(self, visitor))
        return false;
    
    ++visitor->depth;
    ASTNode * node = self->child;
    while (node) {
        ASTNode * current = node;
        node = node->sibling;

        ASTNodeVisitor cur_visitor = * visitor;
        if (!_ast_node_visit(current, &cur_visitor))
            return false;
    }
    --visitor->depth;
    
    if (visitor->post_call) return visitor->post_call(self, visitor);
    return true;
}

FLUFF_PRIVATE_API bool _ast_node_compare(const ASTNode * lhs, const ASTNode * rhs) {
    if (lhs->ast && rhs->ast && (&lhs->ast->root == lhs || &lhs->ast->root == rhs))
        return false;
    return (lhs->hash == rhs->hash && lhs->type == rhs->type &&
            !memcmp(&lhs->data, &rhs->data, sizeof(ASTNode)));
}

/* -===================
     ASTNodeVisitor
   ===================- */

FLUFF_PRIVATE_API bool _ast_node_solve(ASTNode * self) {
    ASTNodeVisitor visitor = {
        .root      = self, 
        .pre_call  = _ast_node_solve_pre_callback, 
        .post_call = _ast_node_solve_post_callback, 
        .depth     = 0, 
    };
    return _ast_node_visit(self, &visitor);
}

FLUFF_PRIVATE_API bool _ast_node_dump(ASTNode * self) {
    ASTNodeVisitor visitor = {
        .root     = self, 
        .pre_call = _ast_node_dump_callback, 
        .depth    = 0, 
    };
    return _ast_node_visit(self, &visitor);
}

/* -========
     AST
   ========- */

FLUFF_PRIVATE_API void _new_ast(AST * self, FluffInterpreter * interpret) {
    FLUFF_CLEANUP(self);
    self->root.ast  = self;
    self->root.type = AST_NODE_SCOPE;

    self->relative_last = &self->root;
    
    self->interpret = interpret;    
}

FLUFF_PRIVATE_API void _free_ast(AST * self) {
    _free_ast_node(&self->root);
}

FLUFF_PRIVATE_API void _ast_dump(AST * self) {
    _ast_node_dump(&self->root);
}