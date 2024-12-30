/* -=============
     Includes
   =============- */

#define FLUFF_IMPLEMENTATION
#include <base.h>
#include <parser/interpret.h>
#include <parser/lexer.h>
#include <parser/analyser.h>
#include <core/config.h>

/* -================
     Interpreter
   ================- */

FLUFF_API FluffInterpreter * fluff_new_interpreter() {
    FluffInterpreter * self = fluff_alloc(NULL, sizeof(FluffInterpreter));
    _new_ast(&self->ast);
    return self;
}

FLUFF_API void fluff_free_interpreter(FluffInterpreter * self) {
    fluff_free(self);
}

FLUFF_API FluffResult fluff_interpreter_read_string(FluffInterpreter * self, const char * source) {
    self->path = "[string]";
    return fluff_interpreter_read(self, source, strlen(source));
}

FLUFF_API FluffResult fluff_interpreter_read_file(FluffInterpreter * self, const char * path) {
    self->path = path;

    FILE * f = fopen(path, "r");
    if (!f) {
        fluff_error_fmt("failed to open file '%s': %s", path, strerror(errno));
        return FLUFF_FAILURE;
    }

    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    fseek(f, 0, SEEK_SET);

    char source[size + 1];
    fread(source, 1, size, f);
    source[size] = '\0';

    fclose(f);

    return fluff_interpreter_read(self, source, size);
}

FLUFF_PRIVATE_API FluffResult fluff_interpreter_read(FluffInterpreter * self, const char * source, size_t n) {
    Lexer lexer;
    _new_lexer(&lexer, self, source, n);
    if (_lexer_parse(&lexer) == FLUFF_FAILURE) {
        _free_lexer(&lexer);
        return FLUFF_FAILURE;
    }
    _lexer_dump(&lexer);
    
    Analyser analyser;
    _new_analyser(&analyser, &lexer);
    if (_analyser_read(&analyser) == FLUFF_FAILURE) {
        _free_lexer(&lexer);
        _free_analyser(&analyser);
        return FLUFF_FAILURE;
    }
    _ast_dump(&self->ast);
    
    _free_analyser(&analyser);
    _free_lexer(&lexer);

    return FLUFF_OK;
}