#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#if !defined(__has_builtin)
#define __has_builtin(x) 0
#endif

#if !__has_builtin(__builtin_expect)
#define __builtin_expect(x, y) (x)
#endif

#define LIKELY(x)   __builtin_expect(!!(x), 1)
#define UNLIKELY(x) __builtin_expect(!!(x), 0)

static void **pointers = NULL;
static size_t pointers_count = 0;
static size_t pointers_capacity = 0;
static char **locations = NULL;
pthread_mutex_t pointers_mutex = PTHREAD_MUTEX_INITIALIZER;

static void _trap(void) {
    __asm__ volatile ("int $0x03");
}

void leakdetector_check(void) {
    if (pointers_count) {
        fprintf(stderr, "Memory leak detected: %zu alive allocations\n", pointers_count);

        for (size_t i = 0; i < pointers_count; i++) {
            fprintf(stderr, "-> %p allocated at %s\n", pointers[i], locations[i]);
        }

        _trap();
    }
}

static void _setup_atexit_check(void) {
    static bool initialized = false;
    if (!initialized) {
        atexit(leakdetector_check);
        initialized = true;
    }
}

void leakdetector_add(void *ptr, size_t size, const char *file, int line) {
    _setup_atexit_check();

    pthread_mutex_lock(&pointers_mutex);

    if (pointers_count == pointers_capacity) {
        pointers_capacity = pointers_capacity ? pointers_capacity * 2 : 16;
        void **new_pointers = realloc(pointers, pointers_capacity * sizeof *new_pointers);
        if (UNLIKELY(!new_pointers)) {
            fprintf(stderr, "Failed to reallocate memory\n");
            exit(EXIT_FAILURE);
        }

        char **new_locations = realloc(locations, pointers_capacity * sizeof *new_locations);
        if (UNLIKELY(!new_locations)) {
            fprintf(stderr, "Failed to reallocate memory\n");
            exit(EXIT_FAILURE);
        }

        pointers = new_pointers;
        locations = new_locations;
    }

    pointers[pointers_count] = ptr;

    size_t length = snprintf(NULL, 0, "%s:%d (%zu bytes)", file, line, size);
    char *str = malloc(length + 1);
    if (UNLIKELY(!str)) {
        fprintf(stderr, "Failed to allocate memory\n");
        exit(EXIT_FAILURE);
    }

    snprintf(str, length + 1, "%s:%d (%zu bytes)", file, line, size);

    locations[pointers_count] = str;

    pointers_count++;
    pthread_mutex_unlock(&pointers_mutex);
}

void leakdetector_remove(void *ptr) {
    if (!ptr) {
        return;
    }

    pthread_mutex_lock(&pointers_mutex);

    bool found = false;
    for (size_t i = 0; i < pointers_count; i++) {
        if (pointers[i] == ptr) {
            pointers[i] = pointers[pointers_count - 1];
            free(locations[i]);
            locations[i] = locations[pointers_count - 1];
            pointers_count--;
            found = true;
            break;
        }
    }

    if (!found) {
        fprintf(stderr, "Freeing unallocated memory: %p\n", ptr);
        _trap();
    }

    pthread_mutex_unlock(&pointers_mutex);
}
