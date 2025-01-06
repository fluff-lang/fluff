/* -=============
     Includes
   =============- */

#define FLUFF_IMPLEMENTATION
#include <base.h>
#include <error.h>
#include <core/instance.h>
#include <core/module.h>
#include <core/class.h>
#include <core/config.h>

/* -==============
     Internals
   ==============- */

FLUFF_THREAD_LOCAL FluffInstance * global_instance = NULL;

/* -=============
     Instance
   =============- */

/* -=- Initializers -=- */
FLUFF_API FluffInstance * fluff_new_instance() {
    FluffInstance * self = fluff_alloc(NULL, sizeof(FluffInstance));
    _new_instance(self);
    return self;
}

FLUFF_API void fluff_free_instance(FluffInstance * self) {
    _free_instance(self);
    fluff_free(self);
}

FLUFF_API void fluff_set_instance(FluffInstance * self) {
    global_instance = self;
}

FLUFF_API FluffInstance * fluff_get_instance() {
    fluff_assert(global_instance, "invalid global instance");
    return global_instance;
}

/* -=- Getters -=- */
FLUFF_API FluffModule * fluff_instance_add_module(FluffInstance * self, FluffModule * module) {
    size_t i = 0;
    FluffModule *  last    = NULL;
    FluffModule ** current = &self->modules;
    while (* current) {
        if (fluff_string_equal(&module->name, &(* current)->name)) {
            fluff_push_error("instance already has a module with the name '%.*s'", 
                (int)module->name.length, module->name.data
            );
            return NULL;
        }
        last    = * current;
        current = &(* current)->next_module;
        ++i;
    }
    if (last) last->next_module = module;
    * current        = module;
    module->index    = i;
    module->instance = self;
    return module;
}

FLUFF_API void fluff_instance_remove_module(FluffInstance * self, const char * name) {
    if (!self->modules) return;
    FluffModule * top    = NULL;
    FluffModule * module = self->modules;
    while (module->next_module) {
        if (fluff_string_equal_s(&module->name, name)) {
            if (top) top->next_module = module->next_module;
            // This has to be nulled out before so the module doesn't think it's inside an instance
            module->instance = NULL;
            fluff_free_module(module);
            return;
        }
        top    = module;
        module = module->next_module;
    }
}

FLUFF_API FluffModule * fluff_instance_get_module_by_name(FluffInstance * self, const char * name) {
    if (!self->modules) return NULL;
    FluffModule * module = self->modules;
    while (module->next_module) {
        if (fluff_string_equal_s(&module->name, name)) return module;
        module = module->next_module;
    }
    return NULL;
}

/* -=- Private -=- */
FLUFF_PRIVATE_API void _new_instance(FluffInstance * self) {
    FLUFF_CLEANUP(self);
    self->void_klass        = _new_class("void", 4, NULL);
    self->void_klass->index = FLUFF_KLASS_VOID;
    self->void_klass->flags = FLUFF_SET_FLAG(self->void_klass->flags, FLUFF_KLASS_PRIMITIVE);

    self->bool_klass        = _new_class("bool", 4, NULL);
    self->bool_klass->index = FLUFF_KLASS_BOOL;
    self->bool_klass->flags = FLUFF_SET_FLAG(self->bool_klass->flags, FLUFF_KLASS_PRIMITIVE);

    self->int_klass        = _new_class("int", 3, NULL);
    self->int_klass->index = FLUFF_KLASS_INT;
    self->int_klass->flags = FLUFF_SET_FLAG(self->int_klass->flags, FLUFF_KLASS_PRIMITIVE);

    self->float_klass        = _new_class("float", 5, NULL);
    self->float_klass->index = FLUFF_KLASS_FLOAT;
    self->float_klass->flags = FLUFF_SET_FLAG(self->float_klass->flags, FLUFF_KLASS_PRIMITIVE);

    self->string_klass        = _new_class("string", 6, NULL);
    self->string_klass->index = FLUFF_KLASS_STRING;
    self->string_klass->flags = FLUFF_SET_FLAG(self->string_klass->flags, FLUFF_KLASS_PRIMITIVE);

    self->object_klass        = _new_class("object", 6, NULL);
    self->object_klass->index = FLUFF_KLASS_OBJECT;
    self->object_klass->flags = FLUFF_SET_FLAG(self->object_klass->flags, FLUFF_KLASS_PRIMITIVE);

    self->array_klass        = _new_class("array", 5, self->object_klass);
    self->array_klass->index = FLUFF_KLASS_ARRAY;
    self->array_klass->flags = FLUFF_SET_FLAG(self->array_klass->flags, FLUFF_KLASS_PRIMITIVE);

    self->func_klass        = _new_class("func", 4, self->object_klass);
    self->func_klass->index = FLUFF_KLASS_FUNC;
    self->func_klass->flags = FLUFF_SET_FLAG(self->func_klass->flags, FLUFF_KLASS_PRIMITIVE);
}

FLUFF_PRIVATE_API void _free_instance(FluffInstance * self) {
    if (global_instance == self) global_instance = NULL;
    FluffModule * current = self->modules;
    while (current) {
        FluffModule * old = current;
        current = current->next_module;

        // This has to be nulled out before so the module doesn't think it's inside an instance
        old->instance = NULL;
        fluff_free_module(old);
    }
    _free_class(self->void_klass);
    _free_class(self->bool_klass);
    _free_class(self->int_klass);
    _free_class(self->float_klass);
    _free_class(self->string_klass);
    _free_class(self->object_klass);
    _free_class(self->array_klass);
    _free_class(self->func_klass);
    FLUFF_CLEANUP(self);
}