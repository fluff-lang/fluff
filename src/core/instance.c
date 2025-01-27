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

/* -=- Getters -=- */
FLUFF_API FluffModule * fluff_instance_add_module(FluffInstance * self, FluffModule * module) {
    size_t i = 0;
    FluffModule *  last    = NULL;
    FluffModule ** current = &self->modules;
    while (* current) {
        if (!strncmp(module->name, (* current)->name, FLUFF_MAX_MODULE_NAME_LEN)) {
            fluff_push_error("instance already has a module with the name '%.*s'", 
                FLUFF_STR_BUFFER_FMT(module->name)
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
        if (!strncmp(module->name, name, FLUFF_MAX_MODULE_NAME_LEN)) {
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
        if (!strncmp(module->name, name, FLUFF_MAX_MODULE_NAME_LEN))
            return module;
        module = module->next_module;
    }
    return NULL;
}

FLUFF_API FluffModule * fluff_instance_get_core_module(FluffInstance * self) {
    return &self->core_module;
}

FLUFF_API FluffKlass * fluff_instance_get_core_class(FluffInstance * self, uint8_t type) {
    return fluff_module_get_class_by_id(&self->core_module, type);
}

/* -=- Private -=- */
FLUFF_PRIVATE_API void _new_instance(FluffInstance * self) {
    FLUFF_CLEANUP(self);
    _new_module(&self->core_module, "CORE");
    _instance_add_void_class(self);
    _instance_add_bool_class(self);
    _instance_add_int_class(self);
    _instance_add_float_class(self);
    _instance_add_string_class(self);
    _instance_add_object_class(self);
    _instance_add_array_class(self);
    _instance_add_func_class(self);
    self->core_module.instance = self;
}

FLUFF_PRIVATE_API void _free_instance(FluffInstance * self) {
    FluffModule * current = self->modules;
    while (current) {
        FluffModule * old = current;
        current = current->next_module;

        // This has to be nulled out before so the module doesn't think it's inside an instance
        old->instance = NULL;
        fluff_free_module(old);
    }
    _free_module(&self->core_module);
    FLUFF_CLEANUP(self);
}

FLUFF_PRIVATE_API void _instance_add_void_class(FluffInstance * self) {
    FluffKlass * klass = _new_common_class("void", 4, NULL);
    klass->index       = FLUFF_KLASS_VOID;
    klass->instance    = self;
    klass->flags       = FLUFF_SET_FLAG(klass->flags, FLUFF_KLASS_PRIMITIVE);
    _module_add_class(&self->core_module, klass);
}

FLUFF_PRIVATE_API void _instance_add_bool_class(FluffInstance * self) {
    FluffKlass * klass = _new_common_class("bool", 4, NULL);
    klass->index       = FLUFF_KLASS_BOOL;
    klass->instance    = self;
    klass->flags       = FLUFF_SET_FLAG(klass->flags, FLUFF_KLASS_PRIMITIVE);
    _module_add_class(&self->core_module, klass);
}

FLUFF_PRIVATE_API void _instance_add_int_class(FluffInstance * self) {
    FluffKlass * klass = _new_common_class("int", 3, NULL);
    klass->index       = FLUFF_KLASS_INT;
    klass->instance    = self;
    klass->flags       = FLUFF_SET_FLAG(klass->flags, FLUFF_KLASS_PRIMITIVE);
    _module_add_class(&self->core_module, klass);
}

FLUFF_PRIVATE_API void _instance_add_float_class(FluffInstance * self) {
    FluffKlass * klass = _new_common_class("float", 5, NULL);
    klass->index       = FLUFF_KLASS_FLOAT;
    klass->instance    = self;
    klass->flags       = FLUFF_SET_FLAG(klass->flags, FLUFF_KLASS_PRIMITIVE);
    _module_add_class(&self->core_module, klass);
}

FLUFF_PRIVATE_API void _instance_add_string_class(FluffInstance * self) {
    FluffKlass * klass = _new_common_class("string", 6, NULL);
    klass->index       = FLUFF_KLASS_STRING;
    klass->instance    = self;
    klass->flags       = FLUFF_SET_FLAG(klass->flags, FLUFF_KLASS_PRIMITIVE);
    _module_add_class(&self->core_module, klass);
}

FLUFF_PRIVATE_API void _instance_add_object_class(FluffInstance * self) {
    FluffKlass * klass = _new_common_class("object", 6, NULL);
    klass->index       = FLUFF_KLASS_OBJECT;
    klass->instance    = self;
    klass->flags       = FLUFF_SET_FLAG(klass->flags, FLUFF_KLASS_PRIMITIVE);
    _module_add_class(&self->core_module, klass);
}

FLUFF_PRIVATE_API void _instance_add_array_class(FluffInstance * self) {
    FluffKlass * klass = _new_common_class("array", 5, NULL);
    klass->index       = FLUFF_KLASS_ARRAY;
    klass->instance    = self;
    klass->flags       = FLUFF_SET_FLAG(klass->flags, FLUFF_KLASS_PRIMITIVE);
    klass->flags       = FLUFF_SET_FLAG(klass->flags, FLUFF_KLASS_GENERIC_BASE);
    _module_add_class(&self->core_module, klass);
}

FLUFF_PRIVATE_API void _instance_add_func_class(FluffInstance * self) {
    FluffKlass * klass = _new_common_class("func", 4, NULL);
    klass->index       = FLUFF_KLASS_FUNC;
    klass->instance    = self;
    klass->flags       = FLUFF_SET_FLAG(klass->flags, FLUFF_KLASS_PRIMITIVE);
    klass->flags       = FLUFF_SET_FLAG(klass->flags, FLUFF_KLASS_GENERIC_BASE);
    _module_add_class(&self->core_module, klass);
}