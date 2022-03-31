#pragma once

#include "egos.h"
#include "memory.h"
#include "syscall.h"
#include "servers.h"

struct earth *earth = (void*)GRASS_STACK_TOP;
struct grass *grass = (void*)APPS_STACK_TOP;
