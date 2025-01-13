/* -=============
     Includes
   =============- */

#define FLUFF_IMPLEMENTATION
#include <base.h>
#include <error.h>
#include <core/module.h>
#include <core/instance.h>
#include <core/class.h>
#include <core/config.h>

/* -==============
     Internals
   ==============- */

/* -===========
     Module
   ===========- */

/* -=- Initializers -=- */
FLUFF_API FluffModule * fluff_new_module(const char * name) {
    FluffModule * self = fluff_alloc(NULL, sizeof(FluffModule));
    _new_module(self, name);
    return self;
}

FLUFF_API void fluff_free_module(FluffModule * self) {
    _free_module(self);
    fluff_free(self);
}

/* -=- Getters -=- */
FLUFF_API const char * fluff_module_get_name(const FluffModule * self) {
    return self->name;
}

/* -=- Private -=- */
FLUFF_PRIVATE_API void _new_module(FluffModule * self, const char * name) {
    FLUFF_CLEANUP(self);
    strncpy(self->name, name, FLUFF_MAX_MODULE_NAME_LEN);
}

FLUFF_PRIVATE_API void _free_module(FluffModule * self) {
    if (self->instance) {
        fluff_instance_remove_module(self->instance, self->name);
        return;
    }
    FluffKlass * current = self->klasses;
    while (current) {
        FluffKlass * old = current;
        current = current->next_klass;
        _free_class(old);
    }
    FLUFF_CLEANUP(self);
}

FLUFF_PRIVATE_API size_t _module_add_class(FluffModule * self, FluffKlass * klass) {
    size_t i = 0;
    FluffKlass ** current = &self->klasses;
    while (* current) {
        if (strncmp(klass->name, (* current)->name, FLUFF_MAX_MODULE_NAME_LEN)) {
            fluff_push_error("module '%s' already has a class named '%s'", 
                FLUFF_STR_BUFFER_FMT(self->name), FLUFF_STR_BUFFER_FMT(klass->name)
            );
            return SIZE_MAX;
        }
        current = &(* current)->next_klass;
        ++i;
    }
    * current       = klass;
    klass->instance = self->instance;
    klass->module   = self;
    klass->index    = i;
    return i;
}