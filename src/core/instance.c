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
    FLUFF_CLEANUP(self);
}