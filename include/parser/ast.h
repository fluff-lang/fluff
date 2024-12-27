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
    AST_NODE_NONE, 

    AST_NODE_OPERATOR, 
    AST_NODE_UNARY_OPERATOR, 
    AST_NODE_CALL, 
    AST_NODE_SUBSCRIPT, 

    AST_NODE_BOOL_LITERAL, 
    AST_NODE_INT_LITERAL, 
    AST_NODE_FLOAT_LITERAL, 
    AST_NODE_STRING_LITERAL, 
    AST_NODE_ARRAY_LITERAL, 
    AST_NODE_LABEL_LITERAL, 

    AST_NODE_SCOPE, 
    AST_NODE_IF, 
    AST_NODE_FOR, 
    AST_NODE_WHILE, 
    AST_NODE_DECLARATION, 
    AST_NODE_FUNCTION, 
    AST_NODE_CLASS, 

    AST_NODE_RETURN, 
    AST_NODE_BREAK, 
    AST_NODE_CONTINUE, 

    AST_NODE_TYPE, 
    AST_NODE_ANNOTATION, 
} ASTNodeType;

typedef enum ASTOperatorType {
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

    AST_OPERATOR_IN, 
    AST_OPERATOR_IS, 
    AST_OPERATOR_AS, 
    AST_OPERATOR_DOT, 
    AST_OPERATOR_EQUAL, 
    AST_OPERATOR_COMMA, 

    AST_OPERATOR_NOT, 
    AST_OPERATOR_BIT_NOT, 
    AST_OPERATOR_PROMOTE, 
    AST_OPERATOR_NEGATE, 
} ASTOperatorType;

typedef struct ASTDecl {
    bool is_constant;
} ASTDecl;

typedef enum ASTTypeNode {
    AST_TYPE_VOID, 
    AST_TYPE_BOOL, 
    AST_TYPE_INT, 
    AST_TYPE_FLOAT, 
    AST_TYPE_STRING, 
    AST_TYPE_OBJECT, 
    AST_TYPE_CLASS, 
    AST_TYPE_FUNC, 
    AST_TYPE_ARRAY, 
} ASTTypeNode;

typedef union ASTNodeData {
    FluffBool   bool_literal;
    FluffInt    int_literal;
    FluffFloat  float_literal;
    FluffString string_literal;

    ASTOperatorType op;

    ASTDecl     decl;
    ASTTypeNode type;
} ASTNodeData;

typedef struct ASTNode {
    AST * ast;

    ASTNode * parent;
    ASTNode * sibling;
    ASTNode * child;
    ASTNode * last_child;
    size_t    index;

    ASTNodeType type;
    ASTNodeData data;

    TextSect loc;

    uint8_t  flags;
    uint64_t hash;
} ASTNode;

// Creates a new node given [type]
FLUFF_PRIVATE_API ASTNode * _new_ast_node(AST * ast, ASTNodeType type, TextSect loc);
FLUFF_PRIVATE_API ASTNode * _new_ast_node_bool(AST * ast, FluffBool v, TextSect loc);
FLUFF_PRIVATE_API ASTNode * _new_ast_node_int(AST * ast, FluffInt v, TextSect loc);
FLUFF_PRIVATE_API ASTNode * _new_ast_node_float(AST * ast, FluffFloat v, TextSect loc);
FLUFF_PRIVATE_API ASTNode * _new_ast_node_string(AST * ast, const char * str, TextSect loc);
FLUFF_PRIVATE_API ASTNode * _new_ast_node_string_n(AST * ast, const char * str, size_t len, TextSect loc);
FLUFF_PRIVATE_API ASTNode * _new_ast_node_literal(AST * ast, const char * str, size_t len, TextSect loc);
FLUFF_PRIVATE_API ASTNode * _new_ast_node_operator(AST * ast, ASTOperatorType op, ASTNode * lhs, ASTNode * rhs, TextSect loc);
FLUFF_PRIVATE_API ASTNode * _new_ast_node_unary_operator(AST * ast, ASTOperatorType op, ASTNode * expr, TextSect loc);
FLUFF_PRIVATE_API ASTNode * _new_ast_node_call(AST * ast, ASTNode * nodes, size_t count, TextSect loc);

// Frees a node regardless of status or reference count
FLUFF_PRIVATE_API void _free_ast_node(ASTNode * self);

// Appends [node] to [self] as a child
FLUFF_PRIVATE_API ASTNode * _ast_node_append_child(ASTNode * self, ASTNode * node);

// Gets the amount of child nodes in [self]
FLUFF_PRIVATE_API size_t _ast_node_get_children_count(ASTNode * self);

// Compares [lhs] to [rhs]
FLUFF_PRIVATE_API bool _ast_node_compare(const ASTNode * lhs, const ASTNode * rhs);

/* -===================
     ASTNodeVisitor
   ===================- */

typedef struct ASTNodeVisitor {
    ASTNode * root;

    bool reversed;

    size_t depth;
} ASTNodeVisitor;

typedef bool (* ASTNodeVisitCallback)(ASTNode *, ASTNodeVisitor *);

FLUFF_PRIVATE_API bool _ast_node_visit(ASTNode * self, ASTNodeVisitor * visitor, ASTNodeVisitCallback callback);

FLUFF_PRIVATE_API bool _ast_node_solve(ASTNode * self);
FLUFF_PRIVATE_API bool _ast_node_dump(ASTNode * self);

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

FLUFF_PRIVATE_API void _ast_dump(AST * self);

#endif