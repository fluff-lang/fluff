#pragma once
#ifndef FLUFF_CORE_METHOD_H
#define FLUFF_CORE_METHOD_H

/* -=============
     Includes
   =============- */

#include <base.h>
#include <core/string.h>

/* -===============
     MethodPool
   ===============- */

typedef struct FluffMethod FluffMethod;

typedef struct MethodPool {
    FluffMethod * methods;
} MethodPool;

/* -===========
     Method
   ===========- */

typedef struct FluffModule FluffModule;

// This struct represents a method.
typedef struct FluffMethod {
    FluffModule * module;

    FluffString signature;
} FluffMethod;

#endif