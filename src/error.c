/* -=============
     Includes
   =============- */

#define FLUFF_IMPLEMENTATION
#include <base.h>
#include <core/config.h>
#include <error.h>

#include <stdio.h>
#include <stdlib.h>

/* -==============
     Internals
   ==============- */

static const char * _log_type_string(uint8_t type) {
    switch (type) {
        case FLUFF_LOG_TYPE_NOTE:  return "note";
        case FLUFF_LOG_TYPE_WARN:  return "warning";
        case FLUFF_LOG_TYPE_ERROR: return "error";
        default:                   return "unknown";
    }
}

FLUFF_THREAD_LOCAL static FluffLog * global_log_buffer;
FLUFF_THREAD_LOCAL static size_t     global_log_size;
FLUFF_THREAD_LOCAL static size_t     global_log_count;

FLUFF_THREAD_LOCAL static char * global_log_msg_buffer;
FLUFF_THREAD_LOCAL static size_t global_log_msg_size;
FLUFF_THREAD_LOCAL static size_t global_log_msg_count;

/* -========
     Log
   ========- */

FLUFF_API void fluff_log_print(FluffLog * self) {
    // TODO: I'm pretty sure there's a better approach for this
    typedef void (* FormatCallback)(const char * restrict, ...);
    FormatCallback fn = (self->type < FLUFF_LOG_TYPE_WARN ? fluff_write_fmt : fluff_error_fmt);

    if (self->debug_file)
        fn("[%s:%d at %s()]:\n\t-> ", self->debug_file, self->debug_line, self->debug_func);
    if (self->file)
        fn("%s:", self->file);
    if (self->line)
        fn("%d:", self->line);
    if (self->column)
        fn("%d:", self->column);
    if (self->file || self->line || self->column)
        fn(" ");

    fn("%s: %.*s\n", 
        _log_type_string(self->type), (int)self->msg_len, self->msg
    );
}

FLUFF_API void fluff_set_log(FluffLog * log_buffer, size_t log_size) {
    global_log_buffer = log_buffer;
    global_log_size   = log_size;
    global_log_count  = 0;
}

FLUFF_API FluffLog * fluff_get_log_buffer() {
    return global_log_buffer;
}

FLUFF_API size_t fluff_get_log_size() {
    return global_log_size;
}

FLUFF_API size_t fluff_get_log_count() {
    return global_log_count;
}

FLUFF_API void fluff_set_log_msg_buffer(char * msg_buffer, size_t msg_size) {
    global_log_msg_buffer = msg_buffer;
    global_log_msg_size   = msg_size;
    global_log_msg_count  = 0;
}

FLUFF_API char * fluff_get_log_msg_buffer() {
    return global_log_msg_buffer;
}

FLUFF_API size_t fluff_get_log_msg_size() {
    return global_log_msg_size;
}

FLUFF_API size_t fluff_get_log_msg_count() {
    return global_log_msg_count;
}

FLUFF_API void fluff_logger_push_v(FluffLog log, const char * restrict fmt, va_list args) {
    if (global_log_count >= global_log_size || global_log_msg_count >= global_log_msg_size)
        return;

    char * msg = &global_log_msg_buffer[global_log_msg_count];
    
    int len = fluff_vformat(msg, global_log_msg_size - global_log_msg_count, fmt, args);
    if (len < 0) fluff_panic("error format failure");
    global_log_msg_size += len;

    log.msg     = msg;
    log.msg_len = len;

    global_log_buffer[global_log_count++] = log;
}

FLUFF_API void fluff_logger_push(FluffLog log, const char * restrict fmt, ...) {
    va_list args;
    va_start(args, fmt);
    fluff_logger_push_v(log, fmt, args);
    va_end(args);
}

FLUFF_API void fluff_logger_print() {
    FluffLog * ptr = global_log_buffer;
    while (ptr != &global_log_buffer[global_log_count])
        fluff_log_print(ptr++);
}

FLUFF_API void fluff_logger_clear() {
    global_log_count     = 0;
    global_log_msg_count = 0;
    FLUFF_CLEANUP_N(global_log_buffer, global_log_count);
    FLUFF_CLEANUP_N(global_log_msg_buffer, global_log_msg_count);
}

FLUFF_PRIVATE_API const char * _log_type_string(uint8_t type);