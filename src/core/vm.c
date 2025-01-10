/* -=============
     Includes
   =============- */

#define FLUFF_IMPLEMENTATION
#include <base.h>
#include <error.h>
#include <core/vm.h>
#include <core/module.h>
#include <core/instance.h>
#include <core/class.h>
#include <core/config.h>

/* -==============
     Internals
   ==============- */

/* -============
     VMFrame
   ============- */

FLUFF_PRIVATE_API void _vm_frame_push(VMFrame * self, FluffObject * obj) {
    VMEntry * entry = fluff_alloc(NULL, sizeof(VMEntry));
    _ref_object(&entry->obj, obj);
    entry->prev      = self->last_entry;
    self->last_entry = entry;
    ++self->entry_count;
}

FLUFF_PRIVATE_API void _vm_frame_pop(VMFrame * self) {
    if (self->entry_count == 0) return;
    VMEntry * entry = self->last_entry;
    self->last_entry = entry->prev;
    _free_object(&entry->obj);
    fluff_free(entry);
    --self->entry_count;
}

FLUFF_PRIVATE_API void _vm_frame_popn(VMFrame * self, size_t count) {
    while (count-- > 0 && self->last_entry) {
        VMEntry * entry  = self->last_entry;
        self->last_entry = entry->prev;
        _free_object(&entry->obj);
        fluff_free(entry);
        --self->entry_count;
    }
}

FLUFF_PRIVATE_API void _vm_frame_clear(VMFrame * self) {
    _vm_frame_popn(self, self->entry_count);
}

FLUFF_PRIVATE_API VMEntry * _vm_frame_at(VMFrame * self, int idx) {
    size_t index = (size_t)(idx < 0 ? -idx - 1 : self->entry_count - idx);
    if (index >= self->entry_count) return NULL;

    VMEntry * entry = self->last_entry;
    while (index > 0) {
        entry = entry->prev;
        --index;
    }
    return entry;
}

/* -=======
     VM
   =======- */

/* -=- Initializers -=- */
FLUFF_API FluffVM * fluff_new_vm(FluffInstance * instance, FluffModule * module) {
    FluffVM * self = fluff_alloc(NULL, sizeof(FluffVM));
    _new_vm(self, instance, module);
    return self;
}

FLUFF_API void fluff_free_vm(FluffVM * self) {
    _free_vm(self);
    fluff_free(self);
}

FLUFF_API FluffObject * fluff_vm_at(FluffVM * self, int idx) {
    return &_vm_frame_at(&self->current_frame, idx)->obj;
}

FLUFF_API FluffResult fluff_vm_push(FluffVM * self, FluffObject * obj) {
    _vm_frame_push(&self->current_frame, obj);
    return FLUFF_OK;
}

FLUFF_API FluffResult fluff_vm_push_object(FluffVM * self, FluffKlass * klass) {
    FluffObject obj;
    _new_object(&obj, self->instance, klass);
    _vm_frame_push(&self->current_frame, &obj);
    return FLUFF_OK;
}

FLUFF_API FluffResult fluff_vm_push_null_object(FluffVM * self, FluffKlass * klass) {
    FluffObject obj;
    _new_null_object(&obj, self->instance, klass);
    _vm_frame_push(&self->current_frame, &obj);
    return FLUFF_OK;
}

FLUFF_API FluffResult fluff_vm_push_bool(FluffVM * self, FluffBool v) {
    FluffObject obj;
    _new_bool_object(&obj, self->instance, v);
    _vm_frame_push(&self->current_frame, &obj);
    return FLUFF_OK;
}

FLUFF_API FluffResult fluff_vm_push_int(FluffVM * self, FluffInt v) {
    FluffObject obj;
    _new_int_object(&obj, self->instance, v);
    _vm_frame_push(&self->current_frame, &obj);
    return FLUFF_OK;
}

FLUFF_API FluffResult fluff_vm_push_float(FluffVM * self, FluffFloat v) {
    FluffObject obj;
    _new_float_object(&obj, self->instance, v);
    _vm_frame_push(&self->current_frame, &obj);
    return FLUFF_OK;
}

FLUFF_API FluffResult fluff_vm_push_string(FluffVM * self, const char * str) {
    FluffObject obj;
    _new_string_object(&obj, self->instance, str);
    _vm_frame_push(&self->current_frame, &obj);
    return FLUFF_OK;
}

FLUFF_API FluffResult fluff_vm_push_string_n(FluffVM * self, const char * str, size_t len) {
    FluffObject obj;
    _new_string_object_n(&obj, self->instance, str, len);
    _vm_frame_push(&self->current_frame, &obj);
    return FLUFF_OK;
}

FLUFF_API FluffResult fluff_vm_pop(FluffVM * self) {
    _vm_frame_pop(&self->current_frame);
    return FLUFF_OK;
}

FLUFF_API FluffResult fluff_vm_popn(FluffVM * self, size_t n) {
    _vm_frame_popn(&self->current_frame, n);
    return FLUFF_OK;
}

FLUFF_API size_t fluff_vm_top(FluffVM * self) {
    return (self->current_frame.entry_count > 0 ? self->current_frame.entry_count - 1 : 0);
}

FLUFF_API size_t fluff_vm_size(FluffVM * self) {
    return self->current_frame.entry_count;
}

FLUFF_API size_t fluff_vm_frame_top(FluffVM * self) {
    return (self->frame_count > 0 ? self->frame_count - 1 : 0);
}

FLUFF_API size_t fluff_vm_frame_size(FluffVM * self) {
    return self->frame_count;
}

FLUFF_PRIVATE_API void _new_vm(FluffVM * self, FluffInstance * instance, FluffModule * module) {
    FLUFF_CLEANUP(self);
    self->instance = instance;
    self->module   = module;
}

FLUFF_PRIVATE_API void _free_vm(FluffVM * self) {
    _vm_clear_frames(self);
    FLUFF_CLEANUP(self);
}

FLUFF_PRIVATE_API FluffResult _vm_push_frame(FluffVM * self, int preserve) {
    if (self->frame_count >= self->frame_capacity)
        self->frames = fluff_alloc(self->frames, sizeof(VMFrame) * (++self->frame_capacity));

    VMFrame * last_frame = &self->frames[self->frame_count++];

    * last_frame = self->current_frame;
    FLUFF_CLEANUP(&self->current_frame);

    while (preserve != 0) {
        _vm_frame_push(&self->current_frame, &_vm_frame_at(last_frame, preserve)->obj);
        if (preserve < 0) ++preserve;
        else              --preserve;
    }

    return FLUFF_OK;
}

FLUFF_PRIVATE_API FluffResult _vm_pop_frame(FluffVM * self, int preserve) {
    if (self->frame_count == 0) return FLUFF_OK;

    VMFrame * last_frame = &self->frames[self->frame_count - 1];

    while (preserve != 0) {
        _vm_frame_push(last_frame, &_vm_frame_at(&self->current_frame, preserve)->obj);
        if (preserve < 0) ++preserve;
        else              --preserve;
    }

    _vm_frame_clear(&self->current_frame);
    self->current_frame = self->frames[--self->frame_count];
    return FLUFF_OK;
}

FLUFF_PRIVATE_API void _vm_clear_frames(FluffVM * self) {
    while (self->frame_count > 0) {
        _vm_pop_frame(self, 0);
    }
    fluff_free(self->frames);
    self->frames         = NULL;
    self->frame_capacity = 0;
}