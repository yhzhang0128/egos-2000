#pragma once

#include "egos.h"
#include "malloc.h"
#include "syscall.h"
#include "servers.h"

struct grass *grass = (void*)APPS_STACK_TOP;
struct earth *earth = (void*)GRASS_STACK_TOP;
