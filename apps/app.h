#pragma once

#include "mem.h"
#include "egos.h"
#include "print.h"
#include "syscall.h"
#include "servers.h"

struct earth *earth = (void*)EARTH_STRUCT;
struct grass *grass = (void*)GRASS_STRUCT;

#define tty_read earth->tty_read
