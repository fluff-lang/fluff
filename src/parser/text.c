/* -=============
     Includes
   =============- */

#define FLUFF_IMPLEMENTATION
#include <parser/text.h>

#include <stdio.h>
#include <stdlib.h>

/* -=============
     TextSect
   =============- */

FLUFF_PRIVATE_API void _text_sect_advance(TextSect * self, const char * str, size_t len, size_t n) {
    if (self->index == len) return;

    const size_t off = len - self->index;
    n = (n < off ? n : off);

    for (size_t i = self->index; i < self->index + n; ++i) {
        if (str[i] == '\n') {
            ++self->line;
            self->column = 0;
        } else ++self->column;
    }

    self->index += n;
}

FLUFF_PRIVATE_API void _text_sect_rewind(TextSect * self, const char * str, size_t len, size_t n) {
    if (self->index == 0) return;
    n = (n < self->index ? n : self->index);

    size_t i = self->index;
    size_t j = n;
    while (i > 0) {
        if (str[i] == '\n') {
            if (j > 0) {
                --self->line;
                --j;
                self->column = self->index;
            } else break;
        }
        --i;
    }
    self->column -= i;
    self->index  -= n;
}

FLUFF_PRIVATE_API TextSect _text_sect_at(size_t idx, const char * str, size_t len) {
    TextSect sect;
    sect.line   = 0;
    sect.column = 0;
    sect.index  = (idx < len ? idx : len);
    while (sect.index <= idx) {
        if (str[sect.index] == '\n') {
            ++sect.line;
            sect.column = 0;
        } else ++sect.column;
        ++sect.index;
    }
    return sect;
}

FLUFF_PRIVATE_API TextSect _text_sect_at_from(TextSect from, size_t to, const char * str, size_t len) {
    TextSect sect = from;
    to = (to < len ? to : len);
    while (sect.index <= to) {
        if (str[sect.index] == '\n') {
            ++sect.line;
            sect.column = 0;
        } else ++sect.column;
        ++sect.index;
    }
    return sect;
}