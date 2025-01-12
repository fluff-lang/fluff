#pragma once
#ifndef FLUFF_ERROR_H
#define FLUFF_ERROR_H

/* -=============
     Includes
   =============- */

#include <base.h>

/* -===========
     Macros
   ===========- */

#define FLUFF_LOG_TYPE_NONE  0x0
#define FLUFF_LOG_TYPE_NOTE  0x1
#define FLUFF_LOG_TYPE_WARN  0x2
#define FLUFF_LOG_TYPE_ERROR 0x3

#define fluff_push_log_d(__type, __file, __line, __column, __dfile, __dfunc, __dline, ...)\
        fluff_logger_push((FluffLog){\
            .type = __type,\
            .file = __file,\
            .line = __line,\
            .column = __column,\
            .debug_file = __dfile,\
            .debug_func = __dfunc,\
            .debug_line = __dline,\
        }, __VA_ARGS__)

#ifdef FLUFF_DEBUG
#   define fluff_push_log(__type, __file, __line, __column, ...)\
           fluff_push_log_d(__type, __file, __line, __column, __FILE__, __func__, __LINE__, __VA_ARGS__)
#else
#   define fluff_push_log(__type, __file, __line, __column, ...)\
           fluff_push_log_d(__type, __file, __line, __column, NULL, NULL, 0, __VA_ARGS__)
#endif

#define fluff_push_note(...)  fluff_push_log(FLUFF_LOG_TYPE_NOTE, NULL, 0, 0, __VA_ARGS__)
#define fluff_push_warn(...)  fluff_push_log(FLUFF_LOG_TYPE_WARN, NULL, 0, 0, __VA_ARGS__)
#define fluff_push_error(...) fluff_push_log(FLUFF_LOG_TYPE_ERROR, NULL, 0, 0, __VA_ARGS__)

#define fluff_alloc_or(__ptr)\
            if (!(__ptr)) { fluff_push_error("out of memory"); return NULL; }

/* -========
     Log
   ========- */

// This struct represents a log.
typedef struct FluffLog {
    uint8_t type;

    const char * msg;
    size_t       msg_len;

    const char * file;
    int line, column;

    const char * debug_file;
    const char * debug_func;
    int debug_line;
} FluffLog;

FLUFF_API void fluff_log_print(FluffLog * self);

FLUFF_API void       fluff_set_log(FluffLog * log_buffer, size_t log_size);
FLUFF_API FluffLog * fluff_get_log_buffer();
FLUFF_API size_t     fluff_get_log_size();
FLUFF_API size_t     fluff_get_log_count();

FLUFF_API void   fluff_set_log_msg_buffer(char * msg_buffer, size_t msg_size);
FLUFF_API char * fluff_get_log_msg_buffer();
FLUFF_API size_t fluff_get_log_msg_size();
FLUFF_API size_t fluff_get_log_msg_count();

FLUFF_API void fluff_logger_push_v(FluffLog log, const char * restrict fmt, va_list args);
FLUFF_API void fluff_logger_push(FluffLog log, const char * restrict fmt, ...);
FLUFF_API void fluff_logger_print();
FLUFF_API void fluff_logger_clear();

#endif