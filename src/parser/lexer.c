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

FLUFF_CONSTEXPR TokenType label_match(Token * token, const char * str, size_t n) {
    // NOTE: this function may seem very hardcoded but it actually performs the best.

    /*
        keywords by order of size:
            2 = or, as, is, in, if
            3 = and, not, let, for, int
            4 = func, self, true, null, else, void, bool
            5 = const, class, false, while, break, float, array, super
            6 = string, object, return
            8 = continue
    */

    switch (n) {
        case 2: {
            switch (str[0]) {
                case 'i': {
                    switch (str[1]) {
                        case 's': return TOKEN_IS;
                        case 'n': return TOKEN_IN;
                        case 'f': return TOKEN_IF;
                        default:  break;
                    }
                    break;
                }
                case 'a': {
                    if (str[1] == 's') return TOKEN_AS;
                    break;
                }
                case 'o': {
                    if (str[1] == 'r') return TOKEN_OR;
                    break;
                }
                default: break;
            }
            break;
        }
        case 3: {
            switch (str[0]) {
                case 'a': {
                    if (str[1] == 'n' && str[2] == 'd') return TOKEN_AND;
                    break;
                }
                case 'n': {
                    if (str[1] == 'o' && str[2] == 't') return TOKEN_NOT;
                    break;
                }
                case 'l': {
                    if (str[1] == 'e' && str[2] == 't') return TOKEN_LET;
                    break;
                }
                case 'f': {
                    if (str[1] == 'o' && str[2] == 'r') return TOKEN_FOR;
                    break;
                }
                case 'i': {
                    if (str[1] == 'n' && str[2] == 't') return TOKEN_INT;
                    break;
                }
                default: break;
            }
            break;
        }
        case 4: {
            switch (str[0]) {
                case 'f': {
                    if (str[1] == 'u' && str[2] == 'n' && str[3] == 'c')
                        return TOKEN_FUNC;
                    break;
                }
                case 't': {
                    if (str[1] == 'r' && str[2] == 'u' && str[3] == 'e') {
                        token->data.b = true;
                        return TOKEN_BOOL_LITERAL;
                    }
                    break;
                }
                case 'e': {
                    if (str[1] == 'l' && str[2] == 's' && str[3] == 'e')
                        return TOKEN_ELSE;
                    break;
                }
                case 'b': {
                    if (str[1] == 'o' && str[2] == 'o' && str[3] == 'l')
                        return TOKEN_BOOL;
                    break;
                }
                case 's': {
                    if (str[1] == 'e' && str[2] == 'l' && str[3] == 'f')
                        return TOKEN_SELF;
                    break;
                }
                case 'n': {
                    if (str[1] == 'u' && str[2] == 'l' && str[3] == 'l')
                        return TOKEN_NULL;
                    break;
                }
                case 'v': {
                    if (str[1] == 'o' && str[2] == 'i' && str[3] == 'd')
                        return TOKEN_VOID;
                    break;
                }
                default: break;
            }
            break;
        }
        case 5: {
            switch (str[0]) {
                case 'c': {
                    if (str[1] == 'o' && str[2] == 'n' && str[3] == 's' && str[4] == 't')
                        return TOKEN_CONST;
                    if (str[1] == 'l' && str[2] == 'a' && str[3] == 's' && str[4] == 's')
                        return TOKEN_CLASS;
                    break;
                }
                case 'b': {
                    if (str[1] == 'r' && str[2] == 'e' && str[3] == 'a' && str[4] == 'k')
                        return TOKEN_BREAK;
                    break;
                }
                case 'f': {
                    if (str[1] == 'a' && str[2] == 'l' && str[3] == 's' && str[4] == 'e') {
                        token->data.b = false;
                        return TOKEN_BOOL_LITERAL;
                    }
                    if (str[1] == 'l' && str[2] == 'o' && str[3] == 'a' && str[4] == 't')
                        return TOKEN_FLOAT;
                    break;
                }
                case 'w': {
                    if (str[1] == 'h' && str[2] == 'i' && str[3] == 'l' && str[4] == 'e')
                        return TOKEN_WHILE;
                    break;
                }
                case 'a': {
                    if (str[1] == 'r' && str[2] == 'r' && str[3] == 'a' && str[4] == 'y')
                        return TOKEN_ARRAY;
                    break;
                }
                case 's': {
                    if (str[1] == 'u' && str[2] == 'p' && str[3] == 'e' && str[4] == 'r')
                        return TOKEN_SUPER;
                    break;
                }
                default: break;
            }
            break;
        }
        case 6: {
            switch (str[0]) {
                case 's': {
                    if (str[1] == 't' && str[2] == 'r' && str[3] == 'i' && str[4] == 'n' && str[5] == 'g')
                        return TOKEN_STRING;
                    break;
                }
                case 'o': {
                    if (str[1] == 'b' && str[2] == 'j' && str[3] == 'e' && str[4] == 'c' && str[5] == 't')
                        return TOKEN_OBJECT;
                    break;
                }
                case 'r': {
                    if (str[1] == 'e' && str[2] == 't' && str[3] == 'u' && str[4] == 'r' && str[5] == 'n')
                        return TOKEN_RETURN;
                    break;
                }
                default: break;
            }
            break;
        }
        case 8: {
            if (str[0] == 'c' && str[1] == 'o' && str[2] == 'n' && str[3] == 't' && 
                str[4] == 'i' && str[5] == 'n' && str[6] == 'u' && str[7] == 'e')
                return TOKEN_CONTINUE;
        }
        default: break;
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
        fluff_panic_fmt("%s:%zu:%zu: " __fmt, \
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

    token.type = label_match(&token, 
        &self->str[self->prev_location.index], 
        self->location.index - self->prev_location.index
    );
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
        printf("[%zu]:%zu..%zu = %s -> ", 
            i, token->start.index, token->start.index + token->length, _token_type_string(token->type)
        );

        switch (token->type) {
            case TOKEN_BOOL_LITERAL:
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
        case TOKEN_CATEGORY_LITERAL:           return "literal";
        case TOKEN_CATEGORY_LABEL:             return "label";
        case TOKEN_CATEGORY_LPAREN:            return "parenthesis";
        case TOKEN_CATEGORY_RPAREN:            return "closing parenthesis";
        case TOKEN_CATEGORY_LBRACE:            return "brace";
        case TOKEN_CATEGORY_RBRACE:            return "closing brace";
        case TOKEN_CATEGORY_LBRACKET:          return "bracket";
        case TOKEN_CATEGORY_RBRACKET:          return "closing bracket";
        case TOKEN_CATEGORY_OPERATOR:          return "operator";
        case TOKEN_CATEGORY_OPERATOR_ARROW:    return "arrow";
        case TOKEN_CATEGORY_OPERATOR_COLON:    return "colon";
        case TOKEN_CATEGORY_OPERATOR_DOT:      return "dot operator";
        case TOKEN_CATEGORY_OPERATOR_ELLIPSIS: return "ellipsis";
        case TOKEN_CATEGORY_OPERATOR_COMMA:    return "comma";
        case TOKEN_CATEGORY_IF:                return "'if' statement";
        case TOKEN_CATEGORY_ELSE:              return "'else' statement";
        case TOKEN_CATEGORY_FOR:               return "'for' statement";
        case TOKEN_CATEGORY_WHILE:             return "'while' statement";
        case TOKEN_CATEGORY_DECL:              return "declaration";
        case TOKEN_CATEGORY_FUNC_DECL:         return "function declaration";
        case TOKEN_CATEGORY_CLASS_DECL:        return "class declaration";
        case TOKEN_CATEGORY_CONTROL_KEYWORD:   return "keyword";
        case TOKEN_CATEGORY_TYPE_KEYWORD:      return "keyword";
        case TOKEN_CATEGORY_END:               return "semicolon";
        case TOKEN_CATEGORY_EOF:               return "eof";
        default:                               return "";
    }
}

FLUFF_PRIVATE_API const char * _token_type_string(TokenType type) {
    switch (type) {
        ENUM_CASE(TOKEN_NONE)
        ENUM_CASE(TOKEN_BOOL_LITERAL)
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
        ENUM_CASE(TOKEN_BREAK)
        ENUM_CASE(TOKEN_CONTINUE)
        ENUM_CASE(TOKEN_RETURN)
        ENUM_CASE(TOKEN_SELF)
        ENUM_CASE(TOKEN_SUPER)
        ENUM_CASE(TOKEN_NULL)
        ENUM_CASE(TOKEN_LET)
        ENUM_CASE(TOKEN_CONST)
        ENUM_CASE(TOKEN_FUNC)
        ENUM_CASE(TOKEN_CLASS)
        ENUM_CASE(TOKEN_VOID)
        ENUM_CASE(TOKEN_BOOL)
        ENUM_CASE(TOKEN_INT)
        ENUM_CASE(TOKEN_FLOAT)
        ENUM_CASE(TOKEN_STRING)
        ENUM_CASE(TOKEN_OBJECT)
        ENUM_CASE(TOKEN_ARRAY)
        ENUM_CASE(TOKEN_END)
        ENUM_CASE(TOKEN_EOF)
        default: return "";
    } 
}

FLUFF_PRIVATE_API TokenCategory _token_type_get_category(TokenType type) {
    switch (type) {
        case TOKEN_INTEGER_LITERAL: case TOKEN_DECIMAL_LITERAL: case TOKEN_STRING_LITERAL:
            return TOKEN_CATEGORY_LITERAL;

        case TOKEN_BOOL_LITERAL: 
            return TOKEN_CATEGORY_LITERAL;

        case TOKEN_VOID:   case TOKEN_BOOL:  case TOKEN_INT:    case TOKEN_FLOAT:
        case TOKEN_STRING: case TOKEN_ARRAY: case TOKEN_OBJECT:
            return TOKEN_CATEGORY_TYPE_KEYWORD;

        case TOKEN_LABEL_LITERAL: 
            return TOKEN_CATEGORY_LABEL;

        case TOKEN_LPAREN:   return TOKEN_CATEGORY_LPAREN;
        case TOKEN_RPAREN:   return TOKEN_CATEGORY_RPAREN;
        case TOKEN_LBRACE:   return TOKEN_CATEGORY_LBRACE;
        case TOKEN_RBRACE:   return TOKEN_CATEGORY_RBRACE;
        case TOKEN_LBRACKET: return TOKEN_CATEGORY_LBRACKET;
        case TOKEN_RBRACKET: return TOKEN_CATEGORY_RBRACKET;
        
        case TOKEN_EQUAL:       case TOKEN_PLUS:       case TOKEN_MINUS:
        case TOKEN_MULTIPLY:    case TOKEN_DIVIDE:     case TOKEN_MODULO:
        case TOKEN_POWER:       case TOKEN_ARROW:      case TOKEN_ELLIPSIS:
        case TOKEN_EQUALS:      case TOKEN_NOT_EQUALS: case TOKEN_LESS:
        case TOKEN_LESS_EQUALS: case TOKEN_GREATER:    case TOKEN_GREATER_EQUALS:
        case TOKEN_AND:         case TOKEN_OR:         case TOKEN_NOT:
        case TOKEN_BIT_AND:     case TOKEN_BIT_OR:     case TOKEN_BIT_XOR:
        case TOKEN_BIT_NOT:     case TOKEN_BIT_SHL:    case TOKEN_BIT_SHR:
        case TOKEN_IN:          case TOKEN_AS:         case TOKEN_IS:
            return TOKEN_CATEGORY_OPERATOR;
        
        case TOKEN_COLON: return TOKEN_CATEGORY_OPERATOR_COLON;
        case TOKEN_COMMA: return TOKEN_CATEGORY_OPERATOR_COMMA;
        case TOKEN_DOT:   return TOKEN_CATEGORY_OPERATOR_DOT;

        case TOKEN_IF:    return TOKEN_CATEGORY_IF;
        case TOKEN_ELSE:  return TOKEN_CATEGORY_ELSE;
        case TOKEN_FOR:   return TOKEN_CATEGORY_FOR;
        case TOKEN_WHILE: return TOKEN_CATEGORY_WHILE;

        case TOKEN_LET: case TOKEN_CONST:
            return TOKEN_CATEGORY_DECL;

        case TOKEN_BREAK: case TOKEN_CONTINUE: case TOKEN_RETURN:
            return TOKEN_CATEGORY_CONTROL_KEYWORD;

        case TOKEN_FUNC:  return TOKEN_CATEGORY_FUNC_DECL;
        case TOKEN_CLASS: return TOKEN_CATEGORY_CLASS_DECL;
        case TOKEN_END:   return TOKEN_CATEGORY_END;
        case TOKEN_EOF:   return TOKEN_CATEGORY_EOF;

        default: return TOKEN_CATEGORY_NONE;
    } 
}