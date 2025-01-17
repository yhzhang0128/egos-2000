#pragma once

#include "egos.h"
#include "servers.h"

struct grass* grass = (void*)GRASS_STRUCT_BASE;
struct earth* earth = (void*)EARTH_STRUCT_BASE;

#define workdir_ino (*(int*)(SYSCALL_ARG + PAGE_SIZE))
#define workdir     ((char*)(SYSCALL_ARG + PAGE_SIZE + sizeof(int)))
