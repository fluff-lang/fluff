/* -=============
     Includes
   =============- */

#define FLUFF_IMPLEMENTATION
#include <base.h>
#include <parser/lexer.h>
#include <parser/ast.h>
#include <core/config.h>

/* -==============
     Internals
   ==============- */

typedef struct ASTNodeInfo {
    const char * name;
    size_t max_children;
} ASTNodeInfo;

#define MAKE_INFO(type, maxc)\
        [type] = (ASTNodeInfo){ .name = #type, .max_children = maxc },

FLUFF_CONSTEXPR_V ASTNodeInfo node_info[] = {
    MAKE_INFO(AST_NODE_NONE,           0)
    MAKE_INFO(AST_NODE_OPERATOR,       2)
    MAKE_INFO(AST_NODE_UNARY_OPERATOR, 1)
    MAKE_INFO(AST_NODE_CALL,           SIZE_MAX)
    MAKE_INFO(AST_NODE_SUBSCRIPT,      1)
    MAKE_INFO(AST_NODE_BOOL_LITERAL,   0)
    MAKE_INFO(AST_NODE_INT_LITERAL,    0)
    MAKE_INFO(AST_NODE_FLOAT_LITERAL,  0)
    MAKE_INFO(AST_NODE_STRING_LITERAL, 0)
    MAKE_INFO(AST_NODE_ARRAY_LITERAL,  0)
    MAKE_INFO(AST_NODE_LABEL_LITERAL,  0)
    MAKE_INFO(AST_NODE_SCOPE,          SIZE_MAX)
    MAKE_INFO(AST_NODE_IF,             3) // TODO: this
    MAKE_INFO(AST_NODE_FOR,            4) // TODO: this
    MAKE_INFO(AST_NODE_WHILE,          2) // TODO: this
    MAKE_INFO(AST_NODE_DECLARATION,    1) // TODO: relation this with scope
    MAKE_INFO(AST_NODE_FUNCTION,       0) // TODO: this
    MAKE_INFO(AST_NODE_CLASS,          0) // TODO: this
    MAKE_INFO(AST_NODE_TYPE,           0) // TODO: this
    MAKE_INFO(AST_NODE_ANNOTATION,     0) // TODO: this
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
    printf("|-[%zu]: %s\n", self->index, node_info[self->type].name);
    return true;
}

FLUFF_CONSTEXPR void _ast_node_solve_operator(ASTNode * self, ASTOperatorType op) {
    switch (self->data.op) {
        case AST_OPERATOR_ADD:
            { printf("add"); break; }
        case AST_OPERATOR_SUB:
            { printf("sub"); break; }
        case AST_OPERATOR_MUL:
            { printf("mul"); break; }
        case AST_OPERATOR_DIV:
            { printf("div"); break; }
        case AST_OPERATOR_MOD:
            { printf("mod"); break; }
        case AST_OPERATOR_POW:
            { printf("pow"); break; }
        case AST_OPERATOR_EQ:
            { printf("eq"); break; }
        case AST_OPERATOR_NE:
            { printf("ne"); break; }
        case AST_OPERATOR_GT:
            { printf("gt"); break; }
        case AST_OPERATOR_GE:
            { printf("ge"); break; }
        case AST_OPERATOR_LT:
            { printf("lt"); break; }
        case AST_OPERATOR_LE:
            { printf("le"); break; }
        case AST_OPERATOR_AND:
            { printf("and"); break; }
        case AST_OPERATOR_OR:
            { printf("or"); break; }
        case AST_OPERATOR_BIT_AND:
            { printf("bit_and"); break; }
        case AST_OPERATOR_BIT_OR:
            { printf("bit_or"); break; }
        case AST_OPERATOR_BIT_XOR:
            { printf("bit_xor"); break; }
        case AST_OPERATOR_BIT_SHL:
            { printf("bit_shl"); break; }
        case AST_OPERATOR_BIT_SHR:
            { printf("bit_shr"); break; }
        case AST_OPERATOR_EQUAL:
            { printf("equal"); break; }
        case AST_OPERATOR_DOT:
            { printf("<dot>"); break; }
        case AST_OPERATOR_IN:
            { printf("in"); break; }
        case AST_OPERATOR_IS:
            { printf("is"); break; }
        case AST_OPERATOR_AS:
            { printf("as"); break; }
        case AST_OPERATOR_NOT:
            { printf("not"); break; }
        case AST_OPERATOR_BIT_NOT:
            { printf("bit_not"); break; }
        case AST_OPERATOR_PROMOTE:
            { printf("promote"); break; }
        case AST_OPERATOR_NEGATE:
            { printf("negate"); break; }
        default: break;
    }
}

FLUFF_CONSTEXPR bool _ast_node_solve_callback(ASTNode * self, ASTNodeVisitor * visitor) {
    switch (self->type) {
        case AST_NODE_UNARY_OPERATOR:
        case AST_NODE_OPERATOR: {
            _ast_node_solve_operator(self, self->data.op);
            break;
        }
        case AST_NODE_BOOL_LITERAL: {
            if (self->data.bool_literal) printf("push_true");
            else                         printf("push_false");
            break;
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
            printf("push_string %s", self->data.string_literal.data);
            break;
        }
        case AST_NODE_ARRAY_LITERAL: {
            printf("push_array %zu", _ast_node_get_children_count(self));
            break;
        }
        case AST_NODE_LABEL_LITERAL: {
            // TODO: handling dot operator case
            printf("get_field");
            printf(" %s", self->data.string_literal.data);
            break;
        }
        case AST_NODE_CALL: {
            printf("call %zu", _ast_node_get_children_count(self) - 1);
            break;
        }
        case AST_NODE_SCOPE: {
            break;
        }
        case AST_NODE_IF: {
            break;
        }
        case AST_NODE_WHILE: {
            break;
        }
        case AST_NODE_FUNCTION: {
            printf("func placeholder");
            break;
        }
        case AST_NODE_CLASS: {
            printf("class placeholder");
            break;
        }
        default: printf("nop");
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
    ++self->loc.line;
    ++self->loc.column;
    //printf("allocated %p\n", self);
    return self;
}

FLUFF_PRIVATE_API ASTNode * _new_ast_node_bool(AST * ast, FluffBool v, TextSect loc) {
    ASTNode * self = _new_ast_node(ast, AST_NODE_BOOL_LITERAL, loc);
    self->data.bool_literal = v;
    self->hash = fluff_hash(&v, sizeof(v));
    return self;
}

FLUFF_PRIVATE_API ASTNode * _new_ast_node_int(AST * ast, FluffInt v, TextSect loc) {
    ASTNode * self = _new_ast_node(ast, AST_NODE_INT_LITERAL, loc);
    self->data.int_literal = v;
    self->hash = fluff_hash(&v, sizeof(v));
    return self;
}

FLUFF_PRIVATE_API ASTNode * _new_ast_node_float(AST * ast, FluffFloat v, TextSect loc) {
    ASTNode * self = _new_ast_node(ast, AST_NODE_FLOAT_LITERAL, loc);
    self->data.float_literal = v;
    self->hash = fluff_hash(&v, sizeof(v));
    return self;
}

FLUFF_PRIVATE_API ASTNode * _new_ast_node_string(AST * ast, const char * str, TextSect loc) {
    return _new_ast_node_string_n(ast, str, (str ? strlen(str) : 0), loc);
}

FLUFF_PRIVATE_API ASTNode * _new_ast_node_string_n(AST * ast, const char * str, size_t len, TextSect loc) {
    ASTNode * self = _new_ast_node(ast, AST_NODE_STRING_LITERAL, loc);
    _new_string_n(&self->data.string_literal, str, len);
    self->hash = fluff_hash(str, len);
    return self;
}

FLUFF_PRIVATE_API ASTNode * _new_ast_node_literal(AST * ast, const char * str, size_t len, TextSect loc) {
    ASTNode * self = _new_ast_node_string_n(ast, str, len, loc);
    self->type = AST_NODE_LABEL_LITERAL;
    return self;
}

FLUFF_PRIVATE_API ASTNode * _new_ast_node_operator(AST * ast, ASTOperatorType op, ASTNode * lhs, ASTNode * rhs, TextSect loc) {
    ASTNode * self = _new_ast_node(ast, AST_NODE_OPERATOR, loc);
    if (lhs) {
        lhs->sibling = rhs;
        _ast_node_append_child(self, lhs);
    }
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
    ASTNodeVisitor visitor = {
        .root     = self, 
        .reversed = true, 
    };
    _ast_node_visit(self, &visitor, _ast_node_free_callback);
    // printf("freed %p\n", self);
}

FLUFF_PRIVATE_API ASTNode * _ast_node_append_child(ASTNode * self, ASTNode * node) {
    const size_t max = node_info[self->type].max_children;
    if (max != 0 && self->last_child && self->last_child->index == max - 1)
        // FIXME: proper error handling for this function
        return NULL;

    size_t last_index = 0;
    if (self->last_child) {
        last_index                = self->last_child->index;
        self->last_child->sibling = node;
    } else self->child = node;
    
    while (node) {
        node->index  = last_index++;
        node->parent = self;
        node         = node->sibling;
    }
    self->last_child = node;
    return node;
}

FLUFF_PRIVATE_API size_t _ast_node_get_children_count(ASTNode * self) {
    return (self->parent && self->parent->last_child ? self->parent->last_child->index + 1 : 0);
}

FLUFF_PRIVATE_API bool _ast_node_visit(ASTNode * self, ASTNodeVisitor * visitor, ASTNodeVisitCallback callback) {
    if (!self) return false;

    if (!visitor->reversed && !callback(self, visitor))
        return false;
    
    ++visitor->depth;
    ASTNode * node = self->child;
    while (node) {
        ASTNode * current = node;
        node = node->sibling;

        ASTNodeVisitor cur_visitor = * visitor;
        if (!_ast_node_visit(current, &cur_visitor, callback))
            return false;
    }
    --visitor->depth;
    
    if (visitor->reversed) return callback(self, visitor);
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
        .root     = self, 
        .reversed = true, 
    };
    return _ast_node_visit(self, &visitor, _ast_node_solve_callback);
}

FLUFF_PRIVATE_API bool _ast_node_dump(ASTNode * self) {
    ASTNodeVisitor visitor = {
        .root     = self, 
        .reversed = false, 
    };
    return _ast_node_visit(self, &visitor, _ast_node_dump_callback);
}

/* -========
     AST
   ========- */

FLUFF_PRIVATE_API void _new_ast(AST * self) {
    FLUFF_CLEANUP(self);
    self->root.ast  = self;
    self->root.type = AST_NODE_SCOPE;

    self->relative_last = &self->root;
}

FLUFF_PRIVATE_API void _free_ast(AST * self) {
    _free_ast_node(&self->root);
}

FLUFF_PRIVATE_API void _ast_dump(AST * self) {
    _ast_node_dump(&self->root);
}