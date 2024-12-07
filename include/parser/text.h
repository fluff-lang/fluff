#pragma once
#ifndef FLUFF_PARSER_TEXT_H
#define FLUFF_PARSER_TEXT_H

/* -=============
     Includes
   =============- */

#include <base.h>

/* -=============
     TextSect
   =============- */

typedef struct TextSect {
    size_t index, line, column;
} TextSect;

FLUFF_PRIVATE_API void _text_sect_advance(TextSect * sect, const char * str, size_t len, size_t n);
FLUFF_PRIVATE_API void _text_sect_rewind(TextSect * sect, const char * str, size_t len, size_t n);

FLUFF_PRIVATE_API TextSect _text_sect_at(size_t idx, const char * str, size_t len);
FLUFF_PRIVATE_API TextSect _text_sect_at_from(TextSect from, size_t to, const char * str, size_t len);

#endif