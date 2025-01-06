/* -=============
     Includes
   =============- */

#define FLUFF_IMPLEMENTATION
#include <base.h>
#include <error.h>
#include <core/class.h>
#include <core/string.h>
#include <core/module.h>
#include <core/instance.h>
#include <core/object.h>
#include <core/config.h>

/* -==============
     Internals
   ==============- */

/* -==========
     Klass
   ==========- */

/* -=- Initializers -=- */
FLUFF_PRIVATE_API FluffKlass * _new_class(const char * name, size_t len, FluffKlass * inherits) {
    FluffKlass * self = fluff_alloc(NULL, sizeof(FluffKlass));
    FLUFF_CLEANUP(self);
    _new_string_n(&self->name, name, len);
    self->inherits = inherits;
    if (inherits) {
        self->flags         = FLUFF_HAS_FLAG(inherits->flags, FLUFF_KLASS_VIRTUAL);
        self->inherit_depth = inherits->inherit_depth + 1;
        // TODO: enforce max inherited classes limit (128)
    }
    return self;
}

FLUFF_PRIVATE_API void _free_class(FluffKlass * self) {
    while (self->property_count != 0) {
        KlassProperty * property = &self->properties[--self->property_count];
        _free_string(&property->name);
        if (property->def_value) fluff_free_object(property->def_value);
    }
    _free_string(&self->name);
    FLUFF_CLEANUP(self);
    fluff_free(self);
}

FLUFF_PRIVATE_API size_t _class_add_property(FluffKlass * self, const char * name, FluffKlass * type, FluffObject * def_value) {
    KlassProperty property = { 0 };
    property.index     = self->property_count;
    property.type      = type;
    property.def_value = def_value;

    self->properties = fluff_alloc(self->properties, sizeof(KlassProperty) * (++self->property_count));
    _new_string(&property.name, name);
    self->properties[property.index] = property;
    return property.index;
}

FLUFF_PRIVATE_API size_t _class_get_property_index(FluffKlass * self, const char * name) {
    size_t len = strlen(name);
    for (size_t i = 0; i < self->property_count; ++i) {
        if (fluff_string_equal_sn(&self->properties[i].name, name, len))
            return i;
    }
    return SIZE_MAX;
}

FLUFF_PRIVATE_API size_t _class_get_alloc_size(FluffKlass * self) {
    size_t size = sizeof(ObjectTable);
    size += sizeof(FluffObject) * (self->property_count + (self->inherits ? 1 : 0));
    return size;
}