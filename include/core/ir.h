#pragma once
#ifndef FLUFF_CORE_IR_H
#define FLUFF_CORE_IR_H

/* -=============
     Includes
   =============- */

#include <base.h>
#include <core/string.h>

// TODO: make a constant table inside the IR so we use less memory
// TODO: better IR structure

/* -===========
     Macros
   ===========- */

#define IR_OP_NOP         0x00
#define IR_OP_JMP         0x01
#define IR_OP_JZ          0x02
#define IR_OP_JNZ         0x03
#define IR_OP_PUSH_VOID   0x10
#define IR_OP_PUSH_TRUE   0x11
#define IR_OP_PUSH_FALSE  0x12
#define IR_OP_PUSH_INT    0x13
#define IR_OP_PUSH_FLOAT  0x14
#define IR_OP_PUSH_STRING 0x15
#define IR_OP_PUSH_OBJECT 0x16
#define IR_OP_PUSH_ARRAY  0x17
#define IR_OP_ADD         0x21
#define IR_OP_SUB         0x22
#define IR_OP_MUL         0x23
#define IR_OP_DIV         0x24
#define IR_OP_MOD         0x25
#define IR_OP_POW         0x26

#define _make_ir_opcode(__type) (IROpcode){ .data = { .op = __type } }

/* -=============
     IROpcode
   =============- */

// This struct represents an opcode inside the IR.
typedef union IROpcode {
    struct {
        uint8_t op : 8;

        // TODO: use this
        // uint8_t arg1 : 4;
        // uint8_t arg2 : 4;
    } data;
    uint8_t word;
} IROpcode;

/* -============
     IRChunk
   ============- */

// This struct represents a chunk inside the IR.
typedef struct IRChunk {
    uint8_t * data;
    size_t    size;
} IRChunk;

FLUFF_PRIVATE_API void _new_ir_chunk(IRChunk * self);
FLUFF_PRIVATE_API void _free_ir_chunk(IRChunk * self);

FLUFF_PRIVATE_API void _ir_chunk_append(IRChunk * self, const void * data, size_t size);
FLUFF_PRIVATE_API void _ir_chunk_append_opcode(IRChunk * self, IROpcode opcode);
FLUFF_PRIVATE_API void _ir_chunk_append_int(IRChunk * self, FluffInt v);
FLUFF_PRIVATE_API void _ir_chunk_append_float(IRChunk * self, FluffFloat v);
FLUFF_PRIVATE_API void _ir_chunk_append_string(IRChunk * self, const char * str);
FLUFF_PRIVATE_API void _ir_chunk_append_string_n(IRChunk * self, const char * str, size_t len);
FLUFF_PRIVATE_API void _ir_chunk_append_chunk(IRChunk * self, IRChunk * chunk);

FLUFF_PRIVATE_API void _ir_chunk_dump(IRChunk * self);

/* -=============
     IRBinary
   =============- */

// This struct represents a binary inside the IR.
typedef struct IRBinary {
    IRChunk main_chunk;
} IRBinary;

FLUFF_PRIVATE_API IRBinary * _new_ir_binary();
FLUFF_PRIVATE_API void       _free_ir_binary(IRBinary * self);

#endif