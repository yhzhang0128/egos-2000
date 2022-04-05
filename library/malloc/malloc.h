#pragma once

#undef  malloc
#define malloc my_alloc
void* my_alloc(unsigned int size);

#undef  free
#define free my_free
void my_free(void* ptr);
