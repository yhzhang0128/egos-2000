#pragma once

#include "egos.h"
#include "syscall.h"

struct grass* grass = (void*)GRASS_STRUCT_BASE;
struct earth* earth = (void*)EARTH_STRUCT_BASE;

#define workdir_ino (*(int*)(SHELL_WORK_DIR))
#define workdir     ((char*)(SHELL_WORK_DIR + sizeof(int)))
