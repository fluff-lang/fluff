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

FLUFF_CONSTEXPR const char * _ast_node_type_get_name(ASTNodeType type) {
    switch (type) {
        case AST_NONE:           return "AST_NONE";
        case AST_OPERATOR:       return "AST_OPERATOR";
        case AST_UNARY_OPERATOR: return "AST_UNARY_OPERATOR";
        case AST_BOOL_LITERAL:   return "AST_BOOL_LITERAL";
        case AST_INT_LITERAL:    return "AST_INT_LITERAL";
        case AST_FLOAT_LITERAL:  return "AST_FLOAT_LITERAL";
        case AST_STRING_LITERAL: return "AST_STRING_LITERAL";
        case AST_ARRAY_LITERAL:  return "AST_ARRAY_LITERAL";
        case AST_LITERAL:        return "AST_LITERAL";
        case AST_SCOPE:          return "AST_SCOPE";
        case AST_IF:             return "AST_IF";
        case AST_WHILE:          return "AST_WHILE";
        case AST_FUNCTION:       return "AST_FUNCTION";
        case AST_CLASS:          return "AST_CLASS";
        default:                 return "AST_UNKNOWN";
    }
}

FLUFF_CONSTEXPR void _ast_node_dump_callback(ASTNode * self, ASTNode * root, size_t identation) {
    while (identation-- > 0) printf("  ");
    printf("<%s>\n", _ast_node_type_get_name(self->type));
}

/* -============
     ASTNode
   ============- */

FLUFF_PRIVATE_API ASTNode * _new_ast_node(AST * ast, ASTNodeType type) {
    ASTNode * self = fluff_alloc(NULL, sizeof(ASTNode));
    FLUFF_CLEANUP(self);
    self->type = type;
    return self;
}

FLUFF_PRIVATE_API ASTNode * _new_ast_node_bool(AST * ast, FluffBool v) {
    ASTNode * self = _new_ast_node(ast, AST_BOOL_LITERAL);
    self->data.bool_literal = v;
    self->hash = fluff_hash(&v, sizeof(v));
    return self;
}

FLUFF_PRIVATE_API ASTNode * _new_ast_node_int(AST * ast, FluffInt v) {
    ASTNode * self = _new_ast_node(ast, AST_INT_LITERAL);
    self->data.int_literal = v;
    self->hash = fluff_hash(&v, sizeof(v));
    return self;
}

FLUFF_PRIVATE_API ASTNode * _new_ast_node_float(AST * ast, FluffFloat v) {
    ASTNode * self = _new_ast_node(ast, AST_FLOAT_LITERAL);
    self->data.float_literal = v;
    self->hash = fluff_hash(&v, sizeof(v));
    return self;
}

FLUFF_PRIVATE_API ASTNode * _new_ast_node_string(AST * ast, const char * str) {
    return _new_ast_node_string_n(ast, str, strlen(str));
}

FLUFF_PRIVATE_API ASTNode * _new_ast_node_string_n(AST * ast, const char * str, size_t len) {
    ASTNode * self = _new_ast_node(ast, AST_STRING_LITERAL);
    _new_string_n(&self->data.string_literal, str, len);
    self->hash = fluff_hash(str, len);
    return self;
}

FLUFF_PRIVATE_API ASTNode * _new_ast_node_operator(AST * ast, ASTOperatorDataType op, ASTNode * lhs, ASTNode * rhs) {
    ASTNode * self     = _new_ast_node(ast, AST_OPERATOR);
    self->data.op.type = op;
    self->data.op.lhs  = lhs;
    self->data.op.rhs  = rhs;
    return self;
}

FLUFF_PRIVATE_API ASTNode * _new_ast_node_unary_operator(AST * ast, ASTUnaryOperatorDataType op, ASTNode * expr) {
    ASTNode * self           = _new_ast_node(ast, AST_UNARY_OPERATOR);
    self->data.unary_op.type = op;
    self->data.unary_op.expr = expr;
    return self;
}

FLUFF_PRIVATE_API void _free_ast_node(ASTNode * self) {
    if (&self->ast->root == self) return; // Avoiding free if the value is the root
    if (self->parent) --self->parent->node_count;
    switch (self->type) {
        case AST_OPERATOR: {
            _free_ast_node(self->data.op.lhs);
            _free_ast_node(self->data.op.rhs);
            break;
        }
        case AST_UNARY_OPERATOR: {
            _free_ast_node(self->data.unary_op.expr);
            break;
        }
        case AST_ARRAY_LITERAL:
        case AST_SCOPE: {
            ASTNode * current = self->data.suite.first;
            while (current) {
                ASTNode * old = current;
                current = current->next;
                _free_ast_node(old);
            }
            break;
        }
        default: break;
    }
    FLUFF_CLEANUP(self);
    fluff_free(self);
}

FLUFF_PRIVATE_API ASTNode * _ast_node_suite_push(ASTNode * self, ASTNode * node) {
    node->parent = self;
    ASTNode ** current = &self->data.suite.first;
    while (* current) current = &(* current)->next;
    * current = node;
    ++self->data.suite.count;
    return node;
}

FLUFF_PRIVATE_API void _ast_node_traverse(ASTNode * self, FluffASTNodeTraverseCallback callback) {
    _ast_node_traverse_n(self, self, 0, callback);
}

FLUFF_PRIVATE_API void _ast_node_traverse_n(ASTNode * self, ASTNode * root, size_t identation, FluffASTNodeTraverseCallback callback) {
    callback(self, root, identation);
    switch (self->type) {
        case AST_OPERATOR: {
            if (self->data.op.lhs)
                _ast_node_traverse_n(self->data.op.lhs, root, identation + 1, callback);
            if (self->data.op.rhs)
                _ast_node_traverse_n(self->data.op.rhs, root, identation + 1, callback);
            break;
        }
        case AST_UNARY_OPERATOR: {
            if (self->data.unary_op.expr)
                _ast_node_traverse_n(self->data.unary_op.expr, root, identation + 1, callback);
            break;
        }
        case AST_ARRAY_LITERAL:
        case AST_SCOPE: {
            ASTNode * node = self->data.suite.first;
            while (node) {
                _ast_node_traverse_n(node, root, identation + 1, callback);
                node = node->next;
            }
            break;
        }
        default: break;
    }
}

FLUFF_PRIVATE_API bool _ast_node_compare(const ASTNode * lhs, const ASTNode * rhs) {
    if (lhs->ast && (&lhs->ast->root == lhs || &lhs->ast->root == rhs)) return false;
    if (lhs->ast && (&lhs->ast->root == lhs || &lhs->ast->root == rhs)) return true;
    return (lhs->hash == rhs->hash && lhs->type == rhs->type &&
            !memcmp(&lhs->data, &rhs->data, sizeof(ASTNode)));
}

FLUFF_PRIVATE_API void _ast_node_dump(ASTNode * self, size_t identation) {
    _ast_node_traverse(self, _ast_node_dump_callback);
}

/* -========
     AST
   ========- */

FLUFF_PRIVATE_API void _new_ast(AST * self) {
    FLUFF_CLEANUP(self);
    self->root.ast  = self;
    self->root.type = AST_SCOPE;

    self->relative_last = &self->root;
}

FLUFF_PRIVATE_API void _free_ast(AST * self) {
    _free_ast_node(&self->root);
}

FLUFF_PRIVATE_API void _ast_dump(AST * self) {
    _ast_node_dump(&self->root, 0);
}