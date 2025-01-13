/* -=============
     Includes
   =============- */

#define FLUFF_IMPLEMENTATION
#include <base.h>
#include <error.h>
#include <core/ir.h>
#include <core/config.h>

/* -==============
     Internals
   ==============- */

#define ARG_TYPE_NONE   0x0
#define ARG_TYPE_INT    0x1
#define ARG_TYPE_FLOAT  0x2
#define ARG_TYPE_STRING 0x3

typedef struct OpcodeInfo {
    const char * name;
    uint8_t arg1, arg2;
} OpcodeInfo;

#define MAKE_OPCODE(__index, __name, __arg1, __arg2)\
        [__index] = (OpcodeInfo){ #__name, ARG_TYPE_##__arg1, ARG_TYPE_##__arg2 }, 

static OpcodeInfo op_info[0xff] = {
    MAKE_OPCODE(0x00, NOP,         NONE,   NONE)
    MAKE_OPCODE(0x01, JMP,         INT,    NONE)
    MAKE_OPCODE(0x02, JZ,          INT,    NONE)
    MAKE_OPCODE(0x03, JNZ,         INT,    NONE)
    MAKE_OPCODE(0x10, PUSH_VOID,   NONE,   NONE)
    MAKE_OPCODE(0x11, PUSH_TRUE,   NONE,   NONE)
    MAKE_OPCODE(0x12, PUSH_FALSE,  NONE,   NONE)
    MAKE_OPCODE(0x13, PUSH_INT,    INT,    NONE)
    MAKE_OPCODE(0x14, PUSH_FLOAT,  FLOAT,  NONE)
    MAKE_OPCODE(0x15, PUSH_STRING, STRING, NONE)
    MAKE_OPCODE(0x16, PUSH_OBJECT, NONE,   NONE)
    MAKE_OPCODE(0x17, PUSH_ARRAY,  INT,    NONE)
    MAKE_OPCODE(0x18, POP,         NONE,   NONE)
    MAKE_OPCODE(0x19, POPN,        INT,    NONE)
    MAKE_OPCODE(0x20, NEW_LOCAL,   NONE,   NONE)
    MAKE_OPCODE(0x21, SET_LOCAL,   NONE,   NONE)
    MAKE_OPCODE(0x22, GET_LOCAL,   NONE,   NONE)
    MAKE_OPCODE(0x23, GET_MEMBER,  NONE,   NONE)
    MAKE_OPCODE(0x24, SET_MEMBER,  NONE,   NONE)
    MAKE_OPCODE(0x25, GET_ITEM,    NONE,   NONE)
    MAKE_OPCODE(0x26, SET_ITEM,    NONE,   NONE)
    MAKE_OPCODE(0x31, ADD,         NONE,   NONE)
    MAKE_OPCODE(0x32, SUB,         NONE,   NONE)
    MAKE_OPCODE(0x33, MUL,         NONE,   NONE)
    MAKE_OPCODE(0x34, DIV,         NONE,   NONE)
    MAKE_OPCODE(0x35, MOD,         NONE,   NONE)
    MAKE_OPCODE(0x36, POW,         NONE,   NONE)
    MAKE_OPCODE(0x37, BIT_AND,     NONE,   NONE)
    MAKE_OPCODE(0x38, BIT_OR,      NONE,   NONE)
    MAKE_OPCODE(0x39, BIT_XOR,     NONE,   NONE)
    MAKE_OPCODE(0x3a, BIT_SHL,     NONE,   NONE)
    MAKE_OPCODE(0x3b, BIT_SHR,     NONE,   NONE)
    MAKE_OPCODE(0x3c, BIT_NOT,     NONE,   NONE)
    MAKE_OPCODE(0x3d, PROMOTE,     NONE,   NONE)
    MAKE_OPCODE(0x3e, NEGATE,      NONE,   NONE)
    MAKE_OPCODE(0x40, EQ,          NONE,   NONE)
    MAKE_OPCODE(0x41, NE,          NONE,   NONE)
    MAKE_OPCODE(0x42, GT,          NONE,   NONE)
    MAKE_OPCODE(0x43, GE,          NONE,   NONE)
    MAKE_OPCODE(0x44, LT,          NONE,   NONE)
    MAKE_OPCODE(0x45, LE,          NONE,   NONE)
    MAKE_OPCODE(0x46, AND,         NONE,   NONE)
    MAKE_OPCODE(0x47, OR,          NONE,   NONE)
    MAKE_OPCODE(0x48, NOT,         NONE,   NONE)
    MAKE_OPCODE(0x49, IS,          NONE,   NONE)
    MAKE_OPCODE(0x4a, AS,          NONE,   NONE)
    MAKE_OPCODE(0x70, CALL,        INT,    NONE)
};

FLUFF_CONSTEXPR size_t _ir_chunk_dump_arg(IRChunk * self, size_t i, uint8_t type) {
    size_t offset = 0;

    // NOTE: scary!
    switch (type) {
        case ARG_TYPE_INT: {
            offset += sizeof(FluffInt);
            printf("%ld", * ((FluffInt *)&self->data[i]));
            break;
        }
        case ARG_TYPE_FLOAT: {
            offset += sizeof(FluffFloat);
            printf("%f", * ((FluffFloat *)&self->data[i]));
            break;
        }
        case ARG_TYPE_STRING: {
            // TODO: theres a buffer overflow here lmfao
            offset += strlen((char *)&self->data[i]) + 1;
            printf("%.*s", (int)offset, &self->data[i]);
            break;
        }
        default: break;
    }

    return offset;
}

/* -============
     IRChunk
   ============- */

FLUFF_PRIVATE_API void _new_ir_chunk(IRChunk * self) {
    FLUFF_CLEANUP(self);
}

FLUFF_PRIVATE_API void _free_ir_chunk(IRChunk * self) {
    fluff_free(self->data);
    FLUFF_CLEANUP(self);
}

FLUFF_PRIVATE_API void _ir_chunk_append(IRChunk * self, const void * data, size_t size) {
    const size_t index = self->size;
    self->size += size;
    self->data  = fluff_alloc(self->data, self->size);
    memcpy(&self->data[index], data, size);
}

FLUFF_PRIVATE_API void _ir_chunk_append_opcode(IRChunk * self, uint8_t opcode) {
    _ir_chunk_append(self, &opcode, sizeof(opcode));
}

FLUFF_PRIVATE_API void _ir_chunk_append_int(IRChunk * self, FluffInt v) {
    _ir_chunk_append(self, &v, sizeof(v));
}

FLUFF_PRIVATE_API void _ir_chunk_append_float(IRChunk * self, FluffFloat v) {
    _ir_chunk_append(self, &v, sizeof(v));
}

FLUFF_PRIVATE_API void _ir_chunk_append_string(IRChunk * self, const char * str) {
    _ir_chunk_append(self, str, strlen(str) + 1);
}

FLUFF_PRIVATE_API void _ir_chunk_append_string_n(IRChunk * self, const char * str, size_t len) {
    _ir_chunk_append(self, str, len);
    const char n = '\0';
    _ir_chunk_append(self, &n, 1);
}

FLUFF_PRIVATE_API void _ir_chunk_append_chunk(IRChunk * self, IRChunk * chunk) {
    _ir_chunk_append(self, chunk->data, chunk->size);
}

FLUFF_PRIVATE_API void _ir_chunk_dump(IRChunk * self) {
    size_t i = 0;
    while (i < self->size) {
        uint8_t type = self->data[i];
        printf("%.2x\t%-16s ", type, op_info[type].name);
        i += sizeof(IROpcode);
        i += _ir_chunk_dump_arg(self, i, op_info[type].arg1);
        putchar('\t');
        i += _ir_chunk_dump_arg(self, i, op_info[type].arg2);
        putchar('\n');
    }
}

/* -=============
     IRBinary
   =============- */

FLUFF_PRIVATE_API IRBinary * _new_ir_binary() {
    IRBinary * self = fluff_alloc(NULL, sizeof(IRBinary));
    FLUFF_CLEANUP(self);
    return self;
}

FLUFF_PRIVATE_API void _free_ir_binary(IRBinary * self) {
    fluff_free(self);
}