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
        case AST_NONE:           return "NONE";
        case AST_OPERATOR:       return "OPERATOR";
        case AST_UNARY_OPERATOR: return "UNARY_OPERATOR";
        case AST_CALL:           return "CALL";
        case AST_SUBSCRIPT:      return "SUBSCRIPT";
        case AST_BOOL_LITERAL:   return "BOOL_LITERAL";
        case AST_INT_LITERAL:    return "INT_LITERAL";
        case AST_FLOAT_LITERAL:  return "FLOAT_LITERAL";
        case AST_STRING_LITERAL: return "STRING_LITERAL";
        case AST_ARRAY_LITERAL:  return "ARRAY_LITERAL";
        case AST_LABEL_LITERAL:  return "LABEL_LITERAL";
        case AST_SCOPE:          return "SCOPE";
        case AST_IF:             return "IF";
        case AST_FOR:            return "FOR";
        case AST_WHILE:          return "WHILE";
        case AST_DECLARATION:    return "DECLARATION";
        case AST_FUNCTION:       return "FUNCTION";
        case AST_CLASS:          return "CLASS";
        case AST_TYPE:           return "TYPE";
        default:                 return "UNKNOWN";
    }
}

FLUFF_CONSTEXPR void _ast_node_dump_callback(ASTNode * self, ASTNode * root, size_t identation) {
    while (identation-- > 0) printf("  ");
    if (!self) {
        printf("<<NULL>>\n");
        return;
    }
    printf("<%s", _ast_node_type_get_name(self->type));
    switch (self->type) {
        case AST_OPERATOR: {
            printf(" '");
            switch (self->data.op.type) {
                case AST_OPERATOR_ADD:
                    { printf("+"); break; }
                case AST_OPERATOR_SUB:
                    { printf("-"); break; }
                case AST_OPERATOR_MUL:
                    { printf("*"); break; }
                case AST_OPERATOR_DIV:
                    { printf("/"); break; }
                case AST_OPERATOR_MOD:
                    { printf("%%"); break; }
                case AST_OPERATOR_POW:
                    { printf("**"); break; }
                case AST_OPERATOR_EQ:
                    { printf("=="); break; }
                case AST_OPERATOR_NE:
                    { printf("!="); break; }
                case AST_OPERATOR_GT:
                    { printf(">"); break; }
                case AST_OPERATOR_GE:
                    { printf(">="); break; }
                case AST_OPERATOR_LT:
                    { printf("<"); break; }
                case AST_OPERATOR_LE:
                    { printf("<="); break; }
                case AST_OPERATOR_AND:
                    { printf("and"); break; }
                case AST_OPERATOR_OR:
                    { printf("or"); break; }
                case AST_OPERATOR_BIT_AND:
                    { printf("&"); break; }
                case AST_OPERATOR_BIT_OR:
                    { printf("|"); break; }
                case AST_OPERATOR_BIT_XOR:
                    { printf("^"); break; }
                case AST_OPERATOR_BIT_SHL:
                    { printf("<<"); break; }
                case AST_OPERATOR_BIT_SHR:
                    { printf(">>"); break; }
                case AST_OPERATOR_DOT:
                    { printf("."); break; }
                case AST_OPERATOR_EQUAL:
                    { printf("="); break; }
                case AST_OPERATOR_COMMA:
                    { printf(","); break; }
                case AST_OPERATOR_IN:
                    { printf("in"); break; }
                case AST_OPERATOR_IS:
                    { printf("is"); break; }
                case AST_OPERATOR_AS:
                    { printf("as"); break; }
                default: break;
            }
            putchar('\'');
            break;
        }
        case AST_UNARY_OPERATOR: {
            printf(" '");
            switch (self->data.unary_op.type) {
                case AST_UOPERATOR_NOT:
                    { printf("not"); break; }
                case AST_UOPERATOR_BIT_NOT:
                    { printf("~"); break; }
                case AST_UOPERATOR_PROMOTE:
                    { printf("+()"); break; }
                case AST_UOPERATOR_NEGATE:
                    { printf("-()"); break; }
                default: break;
            }
            putchar('\'');
            break;
        }
        case AST_BOOL_LITERAL: {
            if (self->data.bool_literal) printf(" true");
            else                         printf(" false");
            break;
        }
        case AST_INT_LITERAL: {
            printf(" %ld", self->data.int_literal);
            break;
        }
        case AST_FLOAT_LITERAL: {
            printf(" %f", self->data.float_literal);
            break;
        }
        case AST_TYPE:
        case AST_STRING_LITERAL: {
            printf(" '%s'", self->data.string_literal.data);
            break;
        }
        case AST_ARRAY_LITERAL: {
            printf(" (%zu items)", self->node_count);
            break;
        }
        case AST_LABEL_LITERAL: {
            printf(" [%s]", self->data.string_literal.data);
            break;
        }
        case AST_CALL: {
            printf(" (%zu arguments)", self->node_count - 1);
            break;
        }
        case AST_SCOPE: {
            break;
        }
        case AST_IF: {
            break;
        }
        case AST_WHILE: {
            break;
        }
        case AST_FUNCTION: {
            break;
        }
        case AST_DECLARATION: {
            if (self->data.decl.is_constant) printf(" (immutable)");
            else                             printf(" (mutable)");
            break;
        }
        case AST_CLASS: {
            break;
        }
        default: break;
    }
    printf("> <children: %zu", self->node_count);
    if (self->loc.line && self->parent && self->parent->loc.line != self->loc.line) {
        printf(", line: %zu", self->loc.line);
    }
    if (self->loc.column && self->parent && self->parent->loc.column != self->loc.column) {
        printf(", column: %zu", self->loc.column);
    }
    printf(">\n");
}

FLUFF_CONSTEXPR void _ast_node_solve_operator(ASTNode * self, ASTOperatorDataType op) {
    switch (self->data.op.type) {
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
        default: break;
    }
}

FLUFF_CONSTEXPR void _ast_node_solve_uoperator(ASTNode * self, ASTUnaryOperatorDataType op) {
    switch (self->data.unary_op.type) {
        case AST_UOPERATOR_NOT:
            { printf("not"); break; }
        case AST_UOPERATOR_BIT_NOT:
            { printf("bit_not"); break; }
        case AST_UOPERATOR_PROMOTE:
            { printf("promote"); break; }
        case AST_UOPERATOR_NEGATE:
            { printf("negate"); break; }
        default: break;
    }
}

FLUFF_CONSTEXPR void _ast_node_solve_callback(ASTNode * self, ASTNode * root, size_t identation) {
    if (!self) {
        printf("<nop>\n");
        return;
    }
    switch (self->type) {
        case AST_OPERATOR: {
            _ast_node_solve_operator(self, self->data.op.type);
            break;
        }
        case AST_UNARY_OPERATOR: {
            _ast_node_solve_operator(self, self->data.unary_op.type);
            break;
        }
        case AST_BOOL_LITERAL: {
            if (self->data.bool_literal) printf("push_true");
            else                         printf("push_false");
            break;
        }
        case AST_INT_LITERAL: {
            printf("push_int %ld", self->data.int_literal);
            break;
        }
        case AST_FLOAT_LITERAL: {
            printf("push_float %f", self->data.float_literal);
            break;
        }
        case AST_STRING_LITERAL: {
            printf("push_string %s", self->data.string_literal.data);
            break;
        }
        case AST_ARRAY_LITERAL: {
            printf("push_array %zu", self->node_count - 1);
            break;
        }
        case AST_LABEL_LITERAL: {
            if (self->parent && self->parent->type == AST_OPERATOR && 
                self->parent->data.op.rhs == self && self->parent->data.op.type == AST_OPERATOR_DOT)
            {
                printf("get_member");
            } else printf("get_global");
            printf(" %s", self->data.string_literal.data);
            break;
        }
        case AST_CALL: {
            printf("call %zu", self->node_count - 1);
            break;
        }
        case AST_SCOPE: {
            break;
        }
        case AST_IF: {
            break;
        }
        case AST_WHILE: {
            break;
        }
        case AST_FUNCTION: {
            printf("func placeholder");
            break;
        }
        case AST_CLASS: {
            printf("class placeholder");
            break;
        }
        default: printf("nop");
    }
    putchar('\n');
}

/* -============
     ASTNode
   ============- */

FLUFF_PRIVATE_API ASTNode * _new_ast_node(AST * ast, ASTNodeType type, TextSect loc) {
    ASTNode * self = fluff_alloc(NULL, sizeof(ASTNode));
    FLUFF_CLEANUP(self);
    self->type = type;
    self->loc  = loc;
    ++self->loc.line;
    ++self->loc.column;
    return self;
}

FLUFF_PRIVATE_API ASTNode * _new_ast_node_bool(AST * ast, FluffBool v, TextSect loc) {
    ASTNode * self = _new_ast_node(ast, AST_BOOL_LITERAL, loc);
    self->data.bool_literal = v;
    self->hash = fluff_hash(&v, sizeof(v));
    return self;
}

FLUFF_PRIVATE_API ASTNode * _new_ast_node_int(AST * ast, FluffInt v, TextSect loc) {
    ASTNode * self = _new_ast_node(ast, AST_INT_LITERAL, loc);
    self->data.int_literal = v;
    self->hash = fluff_hash(&v, sizeof(v));
    return self;
}

FLUFF_PRIVATE_API ASTNode * _new_ast_node_float(AST * ast, FluffFloat v, TextSect loc) {
    ASTNode * self = _new_ast_node(ast, AST_FLOAT_LITERAL, loc);
    self->data.float_literal = v;
    self->hash = fluff_hash(&v, sizeof(v));
    return self;
}

FLUFF_PRIVATE_API ASTNode * _new_ast_node_string(AST * ast, const char * str, TextSect loc) {
    return _new_ast_node_string_n(ast, str, strlen(str), loc);
}

FLUFF_PRIVATE_API ASTNode * _new_ast_node_string_n(AST * ast, const char * str, size_t len, TextSect loc) {
    ASTNode * self = _new_ast_node(ast, AST_STRING_LITERAL, loc);
    _new_string_n(&self->data.string_literal, str, len);
    self->hash = fluff_hash(str, len);
    return self;
}

FLUFF_PRIVATE_API ASTNode * _new_ast_node_literal(AST * ast, const char * str, size_t len, TextSect loc) {
    ASTNode * self = _new_ast_node_string_n(ast, str, len, loc);
    self->type = AST_LABEL_LITERAL;
    return self;
}

FLUFF_PRIVATE_API ASTNode * _new_ast_node_operator(AST * ast, ASTOperatorDataType op, ASTNode * lhs, ASTNode * rhs, TextSect loc) {
    ASTNode * self     = _new_ast_node(ast, AST_OPERATOR, loc);
    self->data.op.type = op;
    self->data.op.lhs  = lhs;
    self->data.op.rhs  = rhs;
    lhs->parent        = self;
    rhs->parent        = self;
    self->node_count  += 2;
    // TODO: need to hash this node too but I'm way too lazy for this
    return self;
}

FLUFF_PRIVATE_API ASTNode * _new_ast_node_unary_operator(AST * ast, ASTUnaryOperatorDataType op, ASTNode * expr, TextSect loc) {
    ASTNode * self           = _new_ast_node(ast, AST_UNARY_OPERATOR, loc);
    self->data.unary_op.type = op;
    self->data.unary_op.expr = expr;
    expr->parent             = self;
    ++self->node_count;
    // TODO: need to hash this node too but I'm way too lazy for this
    return self;
}

FLUFF_PRIVATE_API ASTNode * _new_ast_node_call(AST * ast, ASTNode * nodes, size_t count, TextSect loc) {
    ASTNode * self         = _new_ast_node(ast, AST_CALL, loc);
    self->data.suite.first = nodes;
    //ASTNode * current = nodes;
    //while (current->type == AST_OPERATOR);
    self->node_count       = count;
    nodes->parent          = self;
    return self;
}

FLUFF_PRIVATE_API void _free_ast_node(ASTNode * self) {
    if (!self) return;
    if (&self->ast->root == self) return; // Avoiding free if the value is the root
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
        case AST_IF: {
            _free_ast_node(self->data.if_cond.cond_expr);
            _free_ast_node(self->data.if_cond.if_scope);
            _free_ast_node(self->data.if_cond.else_scope);
            break;
        }
        case AST_WHILE: {
            _free_ast_node(self->data.while_cond.cond_expr);
            _free_ast_node(self->data.while_cond.scope);
            break;
        }
        case AST_CALL:
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
    size_t count = 1;
    ASTNode * last = node;
    while (last->next) {
        ++count;
        last = last->next;
    }
    return _ast_node_suite_push_n(self, node, last, count);
}

FLUFF_PRIVATE_API ASTNode * _ast_node_suite_push_n(ASTNode * self, ASTNode * node, ASTNode * last, size_t count) {
    if (!self->data.suite.first) self->data.suite.first = node;
    if (self->data.suite.last)   self->data.suite.last->next = node;
    self->data.suite.last = last;
    self->node_count += count;
    return node;
}

FLUFF_PRIVATE_API void _ast_node_traverse(ASTNode * self, ASTNodeTraverseCallback callback, bool reverse) {
    _ast_node_traverse_n(self, self, 0, callback, reverse);
}

FLUFF_PRIVATE_API void _ast_node_traverse_n(ASTNode * self, ASTNode * root, size_t identation, ASTNodeTraverseCallback callback, bool reverse) {
    if (!reverse) callback(self, root, identation);
    if (self) {
        switch (self->type) {
            case AST_OPERATOR: {
                _ast_node_traverse_n(self->data.op.lhs, root, identation + 1, callback, reverse);
                _ast_node_traverse_n(self->data.op.rhs, root, identation + 1, callback, reverse);
                break;
            }
            case AST_UNARY_OPERATOR: {
                _ast_node_traverse_n(self->data.unary_op.expr, root, identation + 1, callback, reverse);
                break;
            }
            case AST_IF: {
                _ast_node_traverse_n(self->data.if_cond.cond_expr, root, identation + 1, callback, reverse);
                _ast_node_traverse_n(self->data.if_cond.if_scope, root, identation + 1, callback, reverse);
                _ast_node_traverse_n(self->data.if_cond.else_scope, root, identation + 1, callback, reverse);
                break;
            }
            case AST_WHILE: {
                _ast_node_traverse_n(self->data.while_cond.cond_expr, root, identation + 1, callback, reverse);
                _ast_node_traverse_n(self->data.while_cond.scope, root, identation + 1, callback, reverse);
                break;
            }
            case AST_CALL:
            case AST_ARRAY_LITERAL:
            case AST_SCOPE: {
                ASTNode * node = self->data.suite.first;
                if (self->type == AST_CALL && node) {
                    _ast_node_traverse_n(node, root, identation + 1, callback, reverse);
                    node = node->next;
                }
                while (node) {
                    _ast_node_traverse_n(node, root, identation + 1, callback, reverse);
                    node = node->next;
                }
                break;
            }
            case AST_DECLARATION: {
                _ast_node_traverse_n(self->data.decl.expr, root, identation + 1, callback, reverse);
                _ast_node_traverse_n(self->data.decl.type, root, identation + 1, callback, reverse);
                break;
            }
            default: break;
        }
    }
    if (reverse) callback(self, root, identation);
}

FLUFF_PRIVATE_API bool _ast_node_compare(const ASTNode * lhs, const ASTNode * rhs) {
    if (lhs->ast && (&lhs->ast->root == lhs || &lhs->ast->root == rhs)) return false;
    if (lhs->ast && (&lhs->ast->root == lhs || &lhs->ast->root == rhs)) return true;
    return (lhs->hash == rhs->hash && lhs->type == rhs->type &&
            !memcmp(&lhs->data, &rhs->data, sizeof(ASTNode)));
}

FLUFF_PRIVATE_API void _ast_node_solve(ASTNode * self) {
    _ast_node_traverse_n(self, self, 0, _ast_node_solve_callback, true);
}

FLUFF_PRIVATE_API void _ast_node_dump(ASTNode * self, size_t identation) {
    _ast_node_traverse(self, _ast_node_dump_callback, false);
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