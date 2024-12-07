/* -=============
     Includes
   =============- */

#define FLUFF_IMPLEMENTATION
#include <base.h>
#include <parser/lexer.h>
#include <parser/interpret.h>
#include <core/config.h>

/* -==============
     Internals
   ==============- */

/* -=- Text transform -=- */
FLUFF_CONSTEXPR char to_upper(char c) {
    if (c >= 'a' && c <= 'z') c -= 32;
    return c;
}

FLUFF_CONSTEXPR char to_lower(char c) {
    if (c >= 'A' && c <= 'Z') c += 32;
    return c;
}

FLUFF_CONSTEXPR bool is_space(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

FLUFF_CONSTEXPR bool is_digit(char c) {
    return c >= '0' && c <= '9';
}

FLUFF_CONSTEXPR bool is_decimal(char c, char c_ahead) {
    return c == '.' && is_digit(c_ahead);
}

FLUFF_CONSTEXPR bool is_hexadecimal(char c) {
    c = to_lower(c);
    return c >= 'a' && c <= 'f';
}

FLUFF_CONSTEXPR bool is_hexadecimal_digit(char c) {
    return is_digit(c) || is_hexadecimal(c);
}

FLUFF_CONSTEXPR bool is_octal(char c) {
    return c >= '0' && c <= '7';
}

FLUFF_CONSTEXPR bool is_binary(char c) {
    return c == '0' || c == '1';
}

FLUFF_CONSTEXPR bool is_literal(char c) {
    c = to_lower(c);
    return c == 'x' || c == 'o' || c == 'b';
}

FLUFF_CONSTEXPR bool is_label(char c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_' || c == '$';
}

FLUFF_CONSTEXPR bool is_ascii(char c) {
    return (unsigned char)(c) < 0x7f;
}

FLUFF_CONSTEXPR bool is_unary(char c) {
    return c == '+' || c == '-';
}

FLUFF_CONSTEXPR bool is_operator(char c) {
    return c == '(' || c == '{' || c == '[' || c == ')' || c == '}' || c == ']' ||
           c == '=' || c == '+' || c == '-' || c == '*' || c == '/' || c == '%' || 
           c == ':' || c == ',' || c == '.' || c == '!' || c == '&' || c == '|' ||
           c == '^' || c == '~' || c == '<' || c == '>';
}

FLUFF_CONSTEXPR bool is_comment(char c, char c_ahead) {
    return c == '/' && (c_ahead == '/' || c_ahead == '*');
}

FLUFF_CONSTEXPR bool is_string_delimiter(char c) {
    return c == '\'' || c == '\"' || c == '`';
}

FLUFF_CONSTEXPR bool is_comment_delimiter(char c) {
    // NOTE: > be gay
    //       > do crimes
    return c == '/';
}

FLUFF_CONSTEXPR bool is_end_delimiter(char c) {
    return c == ';';
}

FLUFF_CONSTEXPR bool is_token_separator(char c) {
    return is_space(c) || is_end_delimiter(c) || is_operator(c);
}

FLUFF_CONSTEXPR bool is_number_token_separator(char c, char c_ahead) {
    return is_space(c) || is_end_delimiter(c) || (c_ahead != '.' && is_operator(c));
}

FLUFF_CONSTEXPR TokenType label_match(const char * str, size_t n) {
    // TODO: use a more efficient algorithm for comparasion
    if (n > 5 || n < 2) return TOKEN_LABEL_LITERAL;
    const struct {
        TokenType type;
        const char * str;
        size_t       len;
    } strings[] = {
        { TOKEN_AND,    "and",    3 }, 
        { TOKEN_OR,     "or",     2 }, 
        { TOKEN_NOT,    "not",    3 }, 
        { TOKEN_IF,     "if",     2 }, 
        { TOKEN_ELSE,   "else",   4 }, 
        { TOKEN_FOR,    "for",    3 }, 
        { TOKEN_WHILE,  "while",  5 }, 
        { TOKEN_IN,     "in",     2 }, 
        { TOKEN_AS,     "as",     2 }, 
        { TOKEN_IS,     "is",     2 }, 
        { TOKEN_LET,    "let",    3 }, 
        { TOKEN_CONST,  "const",  5 }, 
        { TOKEN_FUNC,   "func",   4 }, 
        { TOKEN_CLASS,  "class",  5 }, 
        { TOKEN_TRUE,   "true",   4 }, 
        { TOKEN_FALSE,  "false",  5 }, 
        { TOKEN_INT,    "int",    3 }, 
        { TOKEN_FLOAT,  "float",  5 }, 
        { TOKEN_BOOL,   "bool",   4 }, 
        { TOKEN_STRING, "string", 6 }, 
        { TOKEN_OBJECT, "object", 6 }, 
        { TOKEN_ARRAY,  "array",  5 }, 
    };
    for (size_t i = 0; i < FLUFF_LENOF(strings); ++i) {
        if (strings[i].len == n && strncmp(str, strings[i].str, n) == 0)
            return strings[i].type;
    }
    return TOKEN_LABEL_LITERAL;
}

FLUFF_CONSTEXPR int base16toi(char c) {
    c = to_lower(c);
    if (c >= 'a') return c - 'a' + 10;
    return c - '0';
}

/* -==========
     Lexer
   ==========- */

/* -=- -=- */
FLUFF_PRIVATE_API void _new_lexer(Lexer * self, FluffInterpreter * interpret, const char * str, size_t len) {
    FLUFF_CLEANUP(self);
    self->interpret = interpret;
    self->str       = str;
    self->len       = len;
}

FLUFF_PRIVATE_API void _free_lexer(Lexer * self) {
    fluff_free(self->tokens);
    FLUFF_CLEANUP(self);
}

/* -=- Parsing -=- */
#define _lexer_error(__fmt, ...)\
        fluff_error_fmt(FLUFF_COMPILE_ERROR, "%s:%zu:%zu: " __fmt, \
            self->interpret->path, self->location.line + 1, self->location.column + 1, ##__VA_ARGS__\
        )

FLUFF_PRIVATE_API void _lexer_parse(Lexer * self) {
    if (self->len == 0) return;

    while (_lexer_is_within_bounds(self)) {
        _lexer_digest(self);

        const char ch = _lexer_current_char(self);

        if (is_space(ch)) {
            _lexer_consume(self, 1);
        } else if (is_end_delimiter(ch)) {
            _lexer_consume(self, 1);
            _lexer_push(self, _make_token(TOKEN_END));
        } else if (is_comment(ch, _lexer_peek(self, 1))) {
            _lexer_parse_comment(self);
        } else if (is_digit(ch) || is_decimal(ch, _lexer_peek(self, 1))) {
            _lexer_parse_number(self);
        } else if (is_string_delimiter(ch)) {
            _lexer_parse_string(self);
        } else if (is_operator(ch)) {
            _lexer_parse_operator(self);
        } else if (is_label(ch) || !is_ascii(ch)) {
            _lexer_parse_label(self);
        } else
            _lexer_error("unexpected character '%c'", ch);

        fluff_assert(self->prev_location.index < self->location.index, 
            "loop detected, aborting (%zu vs %zu)", 
            self->prev_location.index, self->location.index
        );
    }
}

FLUFF_PRIVATE_API void _lexer_parse_comment(Lexer * self) {
    if (_lexer_peek(self, 1) == '/') {
        while (_lexer_is_within_bounds(self)) {
            if (_lexer_peek(self, 1) == '\n' || _lexer_peek(self, 1) == '\0') break;
            _lexer_consume(self, 1);
        }
    } else if (_lexer_peek(self, 1) == '*') {
        bool ended = false;
        while (_lexer_is_within_bounds(self)) {
            if (_lexer_peek(self, 1) == '*' && _lexer_peek(self, 2) == '/') {
                ended = true;
                _lexer_consume(self, 2);
                break;
            }
            _lexer_consume(self, 1);
        }

        if (!ended) _lexer_error("unterminated comment");
    }
    _lexer_consume(self, 1);
}

FLUFF_PRIVATE_API void _lexer_parse_number(Lexer * self) {
    Token token = _make_token(TOKEN_INTEGER_LITERAL);

    if (_lexer_current_char(self) == '0' && is_ascii(_lexer_peek(self, 1))) {
        _lexer_consume(self, 1);
        const char ch = _lexer_current_char(self);
        switch (ch) {
            case 'b': { _lexer_read_binary(self, &token);      break; }
            case 'o': { _lexer_read_octal(self, &token);       break; }
            case 'x': { _lexer_read_hexadecimal(self, &token); break; }
            default: {
                if (!is_token_separator(ch))
                    _lexer_error("invalid literal '%c'", ch);
                _lexer_read_decimal(self, &token);
            }
        }
    } else {
        _lexer_read_decimal(self, &token);
    }

    _lexer_push(self, token);
}

FLUFF_PRIVATE_API void _lexer_parse_label(Lexer * self) {
    Token token = _make_token(TOKEN_LABEL_LITERAL);

    while (_lexer_is_within_bounds(self)) {
        const char ch = _lexer_current_char(self);
        if (is_label(ch) || is_digit(ch)) {
            _lexer_consume(self, 1);
            continue;
        } else if (is_token_separator(ch)) {
            break;
        } else if (!is_ascii(ch)) {
            _lexer_error("non-ASCII characters are not allowed for labels");
        }

        _lexer_error("unexpected character '%c' in label", ch);
    }

    token.type = label_match(
        &self->str[self->prev_location.index], 
        self->location.index - self->prev_location.index
    );
    if (token.type == TOKEN_TRUE)  token.data.b = true;
    if (token.type == TOKEN_FALSE) token.data.b = false;
    _lexer_push(self, token);
}

FLUFF_PRIVATE_API void _lexer_parse_string(Lexer * self) {
    Token token = _make_token(TOKEN_STRING_LITERAL);

    char start_delim = _lexer_current_char(self);

    _lexer_consume(self, 1);
    _lexer_digest(self);

    bool has_end = false;
    while (_lexer_is_within_bounds(self)) {
        if (_lexer_current_char(self) == start_delim) {
            has_end = true;
            break;
        }
        _lexer_consume(self, 1);
    }

    if (!has_end) _lexer_error("unterminated string");

    _lexer_push(self, token);
    _lexer_consume(self, 1);
}

FLUFF_PRIVATE_API void _lexer_parse_operator(Lexer * self) {
    Token token = _make_token(TOKEN_NONE);

    const char ch = _lexer_current_char(self);
    switch (ch) {
        case '(': { token.type = TOKEN_LPAREN;   break; }
        case '{': { token.type = TOKEN_LBRACE;   break; }
        case '[': { token.type = TOKEN_LBRACKET; break; }
        case ')': { token.type = TOKEN_RPAREN;   break; }
        case '}': { token.type = TOKEN_RBRACE;   break; }
        case ']': { token.type = TOKEN_RBRACKET; break; }
        case '=': { token.type = TOKEN_EQUAL;    break; }
        case '+': { token.type = TOKEN_PLUS;     break; }
        case '-': { token.type = TOKEN_MINUS;    break; }
        case '*': { token.type = TOKEN_MULTIPLY; break; }
        case '%': { token.type = TOKEN_MODULO;   break; }
        case '/': { token.type = TOKEN_DIVIDE;   break; }
        case ':': { token.type = TOKEN_COLON;    break; }
        case ',': { token.type = TOKEN_COMMA;    break; }
        case '.': { token.type = TOKEN_DOT;      break; }
        case '!': { token.type = TOKEN_NOT;      break; }
        case '&': { token.type = TOKEN_BIT_AND;  break; }
        case '|': { token.type = TOKEN_BIT_OR;   break; }
        case '^': { token.type = TOKEN_BIT_XOR;  break; }
        case '~': { token.type = TOKEN_BIT_NOT;  break; }
        case '<': { token.type = TOKEN_LESS;     break; }
        case '>': { token.type = TOKEN_GREATER;  break; }
        default:  FLUFF_UNREACHABLE();
    }

    _lexer_consume(self, 1);
    _lexer_read_long_operator(self, &token, ch);
    _lexer_push(self, token);
}

/* -=- Reading functionality -=- */
FLUFF_PRIVATE_API void _lexer_read_long_operator(Lexer * self, Token * token, char prev_ch) {
    const char ch = _lexer_current_char(self);
    switch (prev_ch) {
        case '=': {
            switch (ch) {
                case '=': { token->type = TOKEN_EQUALS; break; }
                default:  return;
            }
            break;
        }
        case '-': {
            switch (ch) {
                case '>': { token->type = TOKEN_ARROW; break; }
                default:  return;
            }
            break;
        }
        case '*': {
            switch (ch) {
                case '*': { token->type = TOKEN_POWER; break; }
                default:  return;
            }
            break;
        }
        case '!': {
            switch (ch) {
                case '=': { token->type = TOKEN_NOT_EQUALS; break; }
                default:  return;
            }
            break;
        }
        case '&': {
            switch (ch) {
                case '&': { token->type = TOKEN_AND; break; }
                default:  return;
            }
            break;
        }
        case '|': {
            switch (ch) {
                case '|': { token->type = TOKEN_OR; break; }
                default:  return;
            }
            break;
        }
        case '<': {
            switch (ch) {
                case '=': { token->type = TOKEN_LESS_EQUALS; break; }
                case '<': { token->type = TOKEN_BIT_SHL;     break; }
                default:  return;
            }
            break;
        }
        case '>': {
            switch (ch) {
                case '=': { token->type = TOKEN_GREATER_EQUALS; break; }
                case '>': { token->type = TOKEN_BIT_SHR;        break; }
                default:  return;
            }
            break;
        }
        default: return;
    }

    _lexer_consume(self, 1);
}

FLUFF_PRIVATE_API void _lexer_read_decimal(Lexer * self, Token * token) {
    bool e_notation = false;
    bool e_signed   = false;
    bool neg_e      = false;
    bool ended      = false;
    bool decimal    = false;

    FluffFloat top      = 0;
    FluffFloat bot      = 0;
    FluffInt   exponent = 0;

    while (_lexer_is_within_bounds(self)) {
        const char ch = _lexer_current_char(self);
        if (is_unary(ch) && e_notation) {
            if (e_signed) break;

            e_signed = true;
            neg_e    = (ch == '-');
            _lexer_consume(self, 1);
            continue;
        }

        if (is_digit(ch) || ch == '_') {
            if (ch != '_') {
                if (e_notation) {
                    exponent = exponent * 10 + ch - '0';
                    ended    = true;
                } else if (decimal) {
                    bot = ch - '0';
                    bot /= 10;
                } else {
                    top = top * 10 + ch - '0';
                }
            }
            _lexer_consume(self, 1);
            continue;
        } else if (to_lower(ch) == 'e') {
            if (e_notation)
                _lexer_error("duplicated e-notation marker in number");
            e_notation = true;
            _lexer_consume(self, 1);
            continue;
        } else if (is_token_separator(ch)) {
            if (ch == '.') {
                if (e_notation)
                    _lexer_error("decimal number on e-notation exponent");
                if (token->type == TOKEN_DECIMAL_LITERAL)
                    _lexer_error("duplicated dot in decimal number");

                decimal     = true;
                token->type = TOKEN_DECIMAL_LITERAL;
                _lexer_consume(self, 1);
                continue;
            }

            if (e_notation && !ended)
                _lexer_error("invalid e-notation exponent");
            break;
        }

        _lexer_error("malformed number");
    }

    if (decimal) top += bot;

    while (exponent-- != 0) top *= (neg_e ? 0.1 : 10);

    if (decimal) token->data.f = top;
    else         token->data.i = (int)top;
}

FLUFF_PRIVATE_API void _lexer_read_hexadecimal(Lexer * self, Token * token) {
    _lexer_consume(self, 1);

    while (_lexer_is_within_bounds(self)) {
        const char ch = _lexer_current_char(self);
        if (is_hexadecimal_digit(ch) || ch == '_') {
            if (ch != '_') {
                token->data.i *= 16;
                token->data.i += base16toi(ch);
            }
            _lexer_consume(self, 1);
            continue;
        } else if (is_token_separator(ch)) {
            break;
        }

        _lexer_error("hexadecimal number containing non-hexadecimal character '%c'", ch);
    }
}

FLUFF_PRIVATE_API void _lexer_read_octal(Lexer * self, Token * token) {
    _lexer_consume(self, 1);

    while (_lexer_is_within_bounds(self)) {
        const char ch = _lexer_current_char(self);
        if (is_octal(ch) || ch == '_') {
            if (ch != '_') {
                token->data.i *= 8;
                token->data.i += ch - '0';
            }
            _lexer_consume(self, 1);
            continue;
        } else if (is_token_separator(ch)) {
            break;
        }

        _lexer_error("octal number containing non-octal character '%c'", ch);
    }
}

FLUFF_PRIVATE_API void _lexer_read_binary(Lexer * self, Token * token) {
    _lexer_consume(self, 1);
    while (_lexer_is_within_bounds(self)) {
        const char ch = _lexer_current_char(self);
        if (is_binary(ch) || ch == '_') {
            if (ch != '_') {
                token->data.i <<= 1;
                token->data.i |= ch - '0';
            }
            _lexer_consume(self, 1);
            continue;
        } else if (is_token_separator(ch)) {
            break;
        }

        _lexer_error("binary number containing non-binary character '%c'", ch);
    }
}

/* -=- Token management -=- */
FLUFF_PRIVATE_API void _lexer_pop(Lexer * self) {
    // TODO: this
}

FLUFF_PRIVATE_API void _lexer_push(Lexer * self, Token token) {
    if (token.type == TOKEN_NONE) return;
    token.start  = self->prev_location;
    token.length = self->location.index - self->prev_location.index;
    
    self->tokens = fluff_alloc(self->tokens, sizeof(Token) * (++self->token_count));
    self->tokens[self->token_count - 1] = token;
}

/* -=- Character reading -=- */
FLUFF_PRIVATE_API void _lexer_digest(Lexer * self) {
    self->prev_location = self->location;
}

FLUFF_PRIVATE_API void _lexer_consume(Lexer * self, size_t n) {
    _text_sect_advance(&self->location, self->str, self->len, n);
}

FLUFF_PRIVATE_API void _lexer_rewind(Lexer * self, size_t n) {
    _text_sect_rewind(&self->location, self->str, self->len, n);
}

FLUFF_PRIVATE_API char _lexer_peek(Lexer * self, int offset) {
    int pos = ((int)self->location.index) + offset;
    if (pos >= ((int)self->len) || pos < 0) return '\0';
    return self->str[pos];
}

FLUFF_PRIVATE_API char _lexer_peekp(Lexer * self, size_t index) {
    if (index >= self->len) return '\0';
    return self->str[index];
}

FLUFF_PRIVATE_API bool _lexer_is_within_bounds(Lexer * self) {
    return self->location.index < self->len;
}

FLUFF_PRIVATE_API char _lexer_current_char(Lexer * self) {
    return _lexer_peekp(self, self->location.index);
}

FLUFF_PRIVATE_API const char * _lexer_token_string(Lexer * self, size_t index) {
    return &self->str[self->tokens[index].start.index];
}

FLUFF_PRIVATE_API size_t _lexer_token_string_len(Lexer * self, size_t index) {
    return self->tokens[index].length;
}

FLUFF_PRIVATE_API void _lexer_dump(Lexer * self) {
    for (size_t i = 0; i < self->token_count; ++i) {
        const Token * token = &self->tokens[i];
        printf("[%zu]:%zu:%zu = %s -> ", 
            i, token->start.index, token->start.index + token->length, _token_type_string(token->type)
        );

        switch (token->type) {
            case TOKEN_TRUE: case TOKEN_FALSE:
                { printf("%s", FLUFF_BOOLALPHA(token->data.b)); break; }
            case TOKEN_INTEGER_LITERAL:
                { printf("%ld", token->data.i); break; }
            case TOKEN_DECIMAL_LITERAL:
                { printf("%f", token->data.f); break; }
            default: {
                printf("'%.*s'", (int)token->length, &self->str[token->start.index]);
                break;
            }
        }

        putchar('\n');
    }
}

/* -==========
     Token
   ==========- */

/* -=- Utils -=- */
#define ENUM_CASE(__n) case __n: return #__n;

FLUFF_PRIVATE_API const char * _token_category_string(TokenCategory category) {
    switch (category) {
        case TOKEN_CATEGORY_BOOL_LITERAL:      return "'true' or 'false'";
        case TOKEN_CATEGORY_INTEGER_LITERAL:   return "integer";
        case TOKEN_CATEGORY_DECIMAL_LITERAL:   return "float";
        case TOKEN_CATEGORY_STRING_LITERAL:    return "string";
        case TOKEN_CATEGORY_LABEL_LITERAL:     return "label";
        case TOKEN_CATEGORY_LPAREN:            return "'('";
        case TOKEN_CATEGORY_RPAREN:            return "')'";
        case TOKEN_CATEGORY_LBRACE:            return "'{'";
        case TOKEN_CATEGORY_RBRACE:            return "'}'";
        case TOKEN_CATEGORY_LBRACKET:          return "'['";
        case TOKEN_CATEGORY_RBRACKET:          return "']'";
        case TOKEN_CATEGORY_OPERATOR:          return "operator";
        case TOKEN_CATEGORY_OPERATOR_ARROW:    return "'->'";
        case TOKEN_CATEGORY_OPERATOR_DOT:      return "'.'";
        case TOKEN_CATEGORY_OPERATOR_ELLIPSIS: return "'...'";
        case TOKEN_CATEGORY_OPERATOR_COMMA:    return "','";
        case TOKEN_CATEGORY_CONTROL_FLOW:      return "'if', 'else', 'while' or 'for'";
        case TOKEN_CATEGORY_DECL:              return "declaration";
        case TOKEN_CATEGORY_FUNC_DECL:         return "function declaration";
        case TOKEN_CATEGORY_CLASS_DECL:        return "class declaration";
        case TOKEN_CATEGORY_END:               return ";";
        default:                               return "";
    }
}

FLUFF_PRIVATE_API const char * _token_type_string(TokenType type) {
    switch (type) {
        ENUM_CASE(TOKEN_NONE)
        ENUM_CASE(TOKEN_INTEGER_LITERAL)
        ENUM_CASE(TOKEN_DECIMAL_LITERAL)
        ENUM_CASE(TOKEN_STRING_LITERAL)
        ENUM_CASE(TOKEN_LABEL_LITERAL)
        ENUM_CASE(TOKEN_LPAREN)
        ENUM_CASE(TOKEN_RPAREN)
        ENUM_CASE(TOKEN_LBRACE)
        ENUM_CASE(TOKEN_RBRACE)
        ENUM_CASE(TOKEN_LBRACKET)
        ENUM_CASE(TOKEN_RBRACKET)
        ENUM_CASE(TOKEN_EQUAL)
        ENUM_CASE(TOKEN_PLUS)
        ENUM_CASE(TOKEN_MINUS)
        ENUM_CASE(TOKEN_MULTIPLY)
        ENUM_CASE(TOKEN_DIVIDE)
        ENUM_CASE(TOKEN_MODULO)
        ENUM_CASE(TOKEN_POWER)
        ENUM_CASE(TOKEN_COLON)
        ENUM_CASE(TOKEN_COMMA)
        ENUM_CASE(TOKEN_DOT)
        ENUM_CASE(TOKEN_ARROW)
        ENUM_CASE(TOKEN_ELLIPSIS)
        ENUM_CASE(TOKEN_EQUALS)
        ENUM_CASE(TOKEN_NOT_EQUALS)
        ENUM_CASE(TOKEN_LESS)
        ENUM_CASE(TOKEN_LESS_EQUALS)
        ENUM_CASE(TOKEN_GREATER)
        ENUM_CASE(TOKEN_GREATER_EQUALS)
        ENUM_CASE(TOKEN_AND)
        ENUM_CASE(TOKEN_OR)
        ENUM_CASE(TOKEN_NOT)
        ENUM_CASE(TOKEN_BIT_AND)
        ENUM_CASE(TOKEN_BIT_OR)
        ENUM_CASE(TOKEN_BIT_XOR)
        ENUM_CASE(TOKEN_BIT_NOT)
        ENUM_CASE(TOKEN_BIT_SHL)
        ENUM_CASE(TOKEN_BIT_SHR)
        ENUM_CASE(TOKEN_IF)
        ENUM_CASE(TOKEN_ELSE)
        ENUM_CASE(TOKEN_FOR)
        ENUM_CASE(TOKEN_WHILE)
        ENUM_CASE(TOKEN_IN)
        ENUM_CASE(TOKEN_AS)
        ENUM_CASE(TOKEN_IS)
        ENUM_CASE(TOKEN_LET)
        ENUM_CASE(TOKEN_CONST)
        ENUM_CASE(TOKEN_FUNC)
        ENUM_CASE(TOKEN_CLASS)
        ENUM_CASE(TOKEN_TRUE)
        ENUM_CASE(TOKEN_FALSE)
        ENUM_CASE(TOKEN_BOOL)
        ENUM_CASE(TOKEN_INT)
        ENUM_CASE(TOKEN_FLOAT)
        ENUM_CASE(TOKEN_STRING)
        ENUM_CASE(TOKEN_ARRAY)
        ENUM_CASE(TOKEN_END)
        default: return "";
    } 
}

FLUFF_PRIVATE_API TokenCategory _token_type_get_category(TokenType type) {
    switch (type) {
        case TOKEN_INTEGER_LITERAL: return TOKEN_CATEGORY_INTEGER_LITERAL;
        case TOKEN_DECIMAL_LITERAL: return TOKEN_CATEGORY_DECIMAL_LITERAL;
        case TOKEN_STRING_LITERAL:  return TOKEN_CATEGORY_STRING_LITERAL;

        case TOKEN_TRUE: case TOKEN_FALSE: 
            return TOKEN_CATEGORY_BOOL_LITERAL;

        case TOKEN_BOOL:  case TOKEN_INT: case TOKEN_FLOAT:  case TOKEN_STRING: 
        case TOKEN_ARRAY: case TOKEN_END: case TOKEN_OBJECT: case TOKEN_LABEL_LITERAL: 
            return TOKEN_CATEGORY_LABEL_LITERAL;

        case TOKEN_LPAREN:   return TOKEN_CATEGORY_LPAREN;
        case TOKEN_RPAREN:   return TOKEN_CATEGORY_RPAREN;
        case TOKEN_LBRACE:   return TOKEN_CATEGORY_LBRACE;
        case TOKEN_RBRACE:   return TOKEN_CATEGORY_RBRACE;
        case TOKEN_LBRACKET: return TOKEN_CATEGORY_LBRACKET;
        case TOKEN_RBRACKET: return TOKEN_CATEGORY_RBRACKET;
        
        case TOKEN_EQUAL:       case TOKEN_PLUS:       case TOKEN_MINUS:
        case TOKEN_MULTIPLY:    case TOKEN_DIVIDE:     case TOKEN_MODULO:
        case TOKEN_POWER:       case TOKEN_COLON:      case TOKEN_COMMA:
        case TOKEN_DOT:         case TOKEN_ARROW:      case TOKEN_ELLIPSIS:
        case TOKEN_EQUALS:      case TOKEN_NOT_EQUALS: case TOKEN_LESS:
        case TOKEN_LESS_EQUALS: case TOKEN_GREATER:    case TOKEN_GREATER_EQUALS:
        case TOKEN_AND:         case TOKEN_OR:         case TOKEN_NOT:
        case TOKEN_BIT_AND:     case TOKEN_BIT_OR:     case TOKEN_BIT_XOR:
        case TOKEN_BIT_NOT:     case TOKEN_BIT_SHL:    case TOKEN_BIT_SHR:
        case TOKEN_IN:          case TOKEN_AS:         case TOKEN_IS:
            return TOKEN_CATEGORY_OPERATOR;

        case TOKEN_IF: case TOKEN_ELSE: case TOKEN_FOR: case TOKEN_WHILE:
            return TOKEN_CATEGORY_CONTROL_FLOW;

        case TOKEN_LET: case TOKEN_CONST:
            return TOKEN_CATEGORY_DECL;

        case TOKEN_FUNC:  return TOKEN_CATEGORY_FUNC_DECL;
        case TOKEN_CLASS: return TOKEN_CATEGORY_CLASS_DECL;

        default: return TOKEN_CATEGORY_NONE;
    } 
}