#ifndef LEAKDETECTOR_H
#define LEAKDETECTOR_H

#include <stdlib.h>

void leakdetector_add(void *ptr, size_t size, const char *file, int line);
void leakdetector_remove(void *ptr);

#endif
