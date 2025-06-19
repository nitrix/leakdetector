#ifndef LEAKDETECTOR_H
#define LEAKDETECTOR_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *leakdetector_asprintf(bool must, const char *fmt, const char *file, int line, ...);
void *leakdetector_malloc(bool must, size_t size, const char *file, int line);
void *leakdetector_calloc(bool must, size_t size, const char *file, int line);
void *leakdetector_realloc(bool must, void *ptr, size_t size, const char *file, int line);
char *leakdetector_strdup(bool must, const char *str, const char *file, int line);
void leakdetector_free(void *ptr);

#define asprintf(fmt, ...) leakdetector_asprintf(false, fmt, __FILE__, __LINE__, __VA_ARGS__)
#define malloc(size) leakdetector_malloc(false, (size), __FILE__, __LINE__)
#define calloc(size) leakdetector_calloc(false, (size), __FILE__, __LINE__)
#define realloc(ptr, size) leakdetector_realloc(false, (ptr), (size), __FILE__, __LINE__)
#define strdup(str) leakdetector_strdup(false, (str), __FILE__, __LINE__)
#define free(ptr) leakdetector_free(ptr)

#define xasprintf(fmt, ...) leakdetector_asprintf(true, fmt, __FILE__, __LINE__, __VA_ARGS__)
#define xmalloc(size) leakdetector_malloc(true, (size), __FILE__, __LINE__)
#define xcalloc(size) leakdetector_calloc(true, (size), __FILE__, __LINE__)
#define xrealloc(ptr, size) leakdetector_realloc(true, (ptr), (size), __FILE__, __LINE__)
#define xstrdup(str) leakdetector_strdup(true, (str), __FILE__, __LINE__)

extern bool leakdetector;

#endif
