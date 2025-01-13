/* -=============
     Includes
   =============- */

#define FLUFF_IMPLEMENTATION
#include <base.h>
#include <error.h>
#include <core/class.h>
#include <core/method.h>
#include <core/string.h>
#include <core/module.h>
#include <core/instance.h>
#include <core/object.h>
#include <core/vm.h>
#include <core/config.h>

/* -==============
     Internals
   ==============- */

/* -==========
     Klass
   ==========- */

/* -=- C/Dtors -=- */
FLUFF_PRIVATE_API FluffKlass * _new_class(const char * name, size_t len, FluffKlass * inherits) {
    FluffKlass * self = fluff_alloc(NULL, sizeof(FluffKlass));
    FLUFF_CLEANUP(self);
    strncpy(self->name, name, FLUFF_MIN(len, FLUFF_MAX_CLASS_NAME_LEN));
    self->inherits = inherits;
    if (inherits) {
        self->flags         = inherits->flags;
        self->inherit_depth = inherits->inherit_depth + 1;
        // TODO: enforce max inherited classes limit (128)
    }
    return self;
}

FLUFF_PRIVATE_API void _free_class(FluffKlass * self) {
    while (self->property_count != 0) {
        KlassProperty * property = &self->properties[--self->property_count];
        if (property->def_value) fluff_free_object(property->def_value);
    }
    FLUFF_CLEANUP_N(self->properties, self->property_count);
    fluff_free(self->properties);
    FLUFF_CLEANUP(self);
    fluff_free(self);
}

/* -=- Property management -=- */
FLUFF_PRIVATE_API size_t _class_add_property(FluffKlass * self, const char * name, FluffKlass * klass, FluffObject * def_value) {
    for (size_t i = 0; i < self->property_count; ++i) {
        if (strncmp(self->properties[i].name, name, FLUFF_MAX_FIELD_NAME_LEN)) {
            fluff_push_error("the class %.*s already has a property named '%s'\n", 
                FLUFF_STR_BUFFER_FMT(self->name), name
            );
            return SIZE_MAX;
        }
    }

    KlassProperty property = { 0 };
    property.index     = self->property_count;
    property.klass     = klass;
    property.def_value = def_value;
    strncpy(property.name, name, FLUFF_MAX_FIELD_NAME_LEN);

    self->properties = fluff_alloc(self->properties, sizeof(KlassProperty) * (++self->property_count));
    self->properties[property.index] = property;
    return property.index;
}

FLUFF_PRIVATE_API size_t _class_get_property_index(FluffKlass * self, const char * name) {
    for (size_t i = 0; i < self->property_count; ++i) {
        if (strncmp(self->properties[i].name, name, FLUFF_MAX_FIELD_NAME_LEN))
            return i;
    }
    return SIZE_MAX;
}

/* -=- Misc -=- */
FLUFF_PRIVATE_API size_t _class_get_alloc_size(FluffKlass * self) {
    size_t size = sizeof(ObjectTable);
    size += sizeof(FluffObject) * (self->property_count + (self->inherits ? 1 : 0));
    return size;
}