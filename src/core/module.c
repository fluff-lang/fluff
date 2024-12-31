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
    return self->name.data;
}

/* -=- Private -=- */
FLUFF_PRIVATE_API void _new_module(FluffModule * self, const char * name) {
    FLUFF_CLEANUP(self);
    _new_string(&self->name, name);
}

FLUFF_PRIVATE_API void _free_module(FluffModule * self) {
    if (self->instance) {
        fluff_instance_remove_module(self->instance, self->name.data);
        return;
    }
    FluffKlass * current = self->klasses;
    while (current) {
        FluffKlass * old = current;
        current = current->next_klass;
        _free_class(old);
    }
    _free_string(&self->name);
    FLUFF_CLEANUP(self);
}

FLUFF_PRIVATE_API size_t _module_add_class(FluffModule * self, FluffKlass * klass) {
    size_t i = 0;
    FluffKlass ** current = &self->klasses;
    while (* current) {
        if (fluff_string_equal(&klass->name, &(* current)->name)) {
            fluff_push_error("module already has a class named '%.*s'", 
                (int)self->name.length, self->name.data
            );
            return SIZE_MAX;
        }
        current = &(* current)->next_klass;
        ++i;
    }
    * current     = klass;
    klass->module = self;
    klass->index  = i;
    return i;
}

FLUFF_PRIVATE_API void _module_remove_class(FluffModule * self, size_t index) {
    if (!self->klasses) return;
    FluffKlass * top    = NULL;
    FluffKlass * klass = self->klasses;
    while (klass->next_klass) {
        if (klass->index == index) {
            top->next_klass = klass->next_klass;
            _free_class(klass);
            return;
        }
        top   = klass;
        klass = klass->next_klass;
    }
}