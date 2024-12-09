#pragma once
#ifndef FLUFF_PARSER_AST_H
#define FLUFF_PARSER_AST_H

// TODO: make ASTNode pointers relative so AST_INT_LITERAL doesn't take a gazillion bytes in the heap

/* -=============
     Includes
   =============- */

#include <base.h>
#include <core/string.h>
#include <parser/text.h>

/* -===========
     Macros
   ===========- */

#define AST_CONSTANT_BOOL   0x0
#define AST_CONSTANT_INT    0x1
#define AST_CONSTANT_FLOAT  0x2
#define AST_CONSTANT_STRING 0x3

#define AST_NODE_VISITED   0x1
#define AST_NODE_FOLDABLE  0x2
#define AST_NODE_REFERENCE 0x4

/* -================
     ASTConstant
   ================- */

typedef struct ASTConstant {
    uint8_t type;
    size_t  uses;
} ASTConstant;

/* -============
     ASTNode
   ============- */

typedef struct AST     AST;
typedef struct ASTNode ASTNode;

typedef enum ASTNodeType {
    AST_NONE, 

    AST_OPERATOR, 
    AST_UNARY_OPERATOR, 
    AST_CALL, 
    AST_SUBSCRIPT, 

    AST_BOOL_LITERAL, 
    AST_INT_LITERAL, 
    AST_FLOAT_LITERAL, 
    AST_STRING_LITERAL, 
    AST_ARRAY_LITERAL, 
    AST_LABEL_LITERAL, 

    AST_SCOPE, 
    AST_IF, 
    AST_WHILE, 
    AST_FUNCTION, 
    AST_CLASS, 
} ASTNodeType;

typedef enum ASTOperatorDataType {
    AST_OPERATOR_NONE, 

    AST_OPERATOR_ADD, 
    AST_OPERATOR_SUB, 
    AST_OPERATOR_MUL, 
    AST_OPERATOR_DIV, 
    AST_OPERATOR_MOD, 
    AST_OPERATOR_POW, 

    AST_OPERATOR_EQ, 
    AST_OPERATOR_NE, 
    AST_OPERATOR_GT, 
    AST_OPERATOR_GE, 
    AST_OPERATOR_LT, 
    AST_OPERATOR_LE, 
    AST_OPERATOR_AND, 
    AST_OPERATOR_OR, 

    AST_OPERATOR_BIT_AND, 
    AST_OPERATOR_BIT_OR, 
    AST_OPERATOR_BIT_XOR, 
    AST_OPERATOR_BIT_SHL, 
    AST_OPERATOR_BIT_SHR, 

    AST_OPERATOR_DOT, 
    AST_OPERATOR_EQUAL, 
} ASTOperatorDataType;

typedef struct ASTOperatorData {
    ASTOperatorDataType type;
    ASTNode * lhs;
    ASTNode * rhs;
} ASTOperatorData;

typedef enum ASTUnaryOperatorDataType {
    AST_UOPERATOR_NONE, 
    AST_UOPERATOR_NOT, 
    AST_UOPERATOR_BIT_NOT, 
    AST_UOPERATOR_PROMOTE, 
    AST_UOPERATOR_NEGATE, 
} ASTUnaryOperatorDataType;

typedef struct ASTUnaryOperatorData {
    ASTUnaryOperatorDataType type;
    ASTNode * expr;
} ASTUnaryOperatorData;

typedef struct ASTSuiteData {
    ASTNode * first;
    ASTNode * last;
} ASTSuiteData;

typedef struct ASTIfData {
    ASTNode    * cond_expr;
    ASTSuiteData if_scope;
    ASTSuiteData else_scope;
} ASTIfData;

typedef struct ASTWhileData {
    ASTNode    * cond_expr;
    ASTSuiteData scope;
} ASTWhileData;

typedef union ASTNodeData {
    FluffBool   bool_literal;
    FluffInt    int_literal;
    FluffFloat  float_literal;
    FluffString string_literal;

    ASTOperatorData      op;
    ASTUnaryOperatorData unary_op;

    ASTSuiteData suite;

    ASTIfData    if_cond;
    ASTWhileData while_cond;
} ASTNodeData;

typedef struct ASTNode {
    AST     * ast;
    ASTNode * parent;
    size_t    node_count;

    ASTNode * next;

    ASTNode * relative_next;
    ASTNode * relative_prev;

    ASTNodeType type;
    ASTNodeData data;

    uint8_t  flags;
    uint64_t hash;
} ASTNode;

// Creates a new node given [type]
FLUFF_PRIVATE_API ASTNode * _new_ast_node(AST * ast, ASTNodeType type);

// Creates a new node given [type]
FLUFF_PRIVATE_API ASTNode * _new_ast_node_bool(AST * ast, FluffBool v);

// Creates a new node given [type]
FLUFF_PRIVATE_API ASTNode * _new_ast_node_int(AST * ast, FluffInt v);
FLUFF_PRIVATE_API ASTNode * _new_ast_node_float(AST * ast, FluffFloat v);
FLUFF_PRIVATE_API ASTNode * _new_ast_node_string(AST * ast, const char * str);
FLUFF_PRIVATE_API ASTNode * _new_ast_node_string_n(AST * ast, const char * str, size_t len);
FLUFF_PRIVATE_API ASTNode * _new_ast_node_literal(AST * ast, const char * str, size_t len);
FLUFF_PRIVATE_API ASTNode * _new_ast_node_operator(AST * ast, ASTOperatorDataType op, ASTNode * lhs, ASTNode * rhs);
FLUFF_PRIVATE_API ASTNode * _new_ast_node_unary_operator(AST * ast, ASTUnaryOperatorDataType op, ASTNode * expr);
FLUFF_PRIVATE_API ASTNode * _new_ast_node_call(AST * ast, ASTNode * nodes, size_t count);

// Frees a node regardless of status or reference count
FLUFF_PRIVATE_API void _free_ast_node(ASTNode * self);

// Links the node [a] to node [b]
FLUFF_PRIVATE_API void _ast_node_link(ASTNode * a, ASTNode * b);

// Unlinks a node and remove all their associated references
FLUFF_PRIVATE_API void _ast_node_unlink(ASTNode * self);

FLUFF_PRIVATE_API ASTNode * _ast_node_suite_push(ASTNode * self, ASTNode * node);
FLUFF_PRIVATE_API ASTNode * _ast_node_suite_push_n(ASTNode * self, ASTNode * node, ASTNode * last, size_t count);

// Adds one to a node's reference count
FLUFF_PRIVATE_API void _ast_node_ref(ASTNode * self);

// Subtracts one from a node's reference count, and if it reaches 0, free's the node.
FLUFF_PRIVATE_API void _ast_node_deref(ASTNode * self);

typedef void (* FluffASTNodeTraverseCallback)(ASTNode *, ASTNode *, size_t);

FLUFF_PRIVATE_API void _ast_node_traverse(ASTNode * self, FluffASTNodeTraverseCallback callback, bool reverse);
FLUFF_PRIVATE_API void _ast_node_traverse_n(ASTNode * self, ASTNode * root, size_t identation, FluffASTNodeTraverseCallback callback, bool reverse);
FLUFF_PRIVATE_API bool _ast_node_compare(const ASTNode * lhs, const ASTNode * rhs);

FLUFF_PRIVATE_API void _ast_node_solve(ASTNode * self);
FLUFF_PRIVATE_API void _ast_node_dump(ASTNode * self, size_t identation);

/* -========
     AST
   ========- */

typedef struct AST {
    ASTNode root;
    ASTNode * relative_last;
    size_t    relative_count;

    ASTConstant * first_constant;
    ASTConstant * last_constant;
} AST;

FLUFF_PRIVATE_API void _new_ast(AST * self);
FLUFF_PRIVATE_API void _free_ast(AST * self);

FLUFF_PRIVATE_API void _ast_add_node(AST * self, ASTNode * node);
FLUFF_PRIVATE_API void _ast_remove_node(AST * self, ASTNode * node);

FLUFF_PRIVATE_API size_t _ast_push_constant(AST * self, void * data, size_t len);
FLUFF_PRIVATE_API size_t _ast_find_constant(AST * self, void * data, size_t len);
FLUFF_PRIVATE_API size_t _ast_add_constant_bool(AST * self, FluffBool v);
FLUFF_PRIVATE_API size_t _ast_add_constant_int(AST * self, FluffInt v);
FLUFF_PRIVATE_API size_t _ast_add_constant_float(AST * self, FluffFloat v);
FLUFF_PRIVATE_API size_t _ast_add_constant_string(AST * self, const char * str);
FLUFF_PRIVATE_API size_t _ast_add_constant_string_n(AST * self, const char * str, size_t len);

FLUFF_PRIVATE_API void _ast_dump(AST * self);

#endif