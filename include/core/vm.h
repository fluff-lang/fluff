#pragma once
#ifndef FLUFF_CORE_VM_H
#define FLUFF_CORE_VM_H

/* -=============
     Includes
   =============- */

#include <base.h>
#include <core/string.h>

/* -============
     VMFrame
   ============- */

typedef struct VMFrame VMFrame;

typedef struct VMFrame {

} VMFrame;

/* -=======
     VM
   =======- */

typedef struct FluffModule FluffModule;

// This struct represents a VM.
typedef struct FluffVM {
    FluffModule * module;

    VMFrame * frames;
    size_t    frame_count, frame_capacity;
} FluffVM;

FLUFF_API FluffVM * fluff_new_vm(FluffModule * module);
FLUFF_API void      fluff_free_vm(FluffVM * self);

FLUFF_PRIVATE_API void _new_vm(FluffVM * self, FluffModule * module);
FLUFF_PRIVATE_API void _free_vm(FluffVM * self);

#endif