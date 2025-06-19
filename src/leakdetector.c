#include <assert.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if !defined(__has_builtin)
#define __has_builtin(x) 0
#endif

#if !__has_builtin(__builtin_expect)
#define __builtin_expect(x, y) (x)
#endif

#define LIKELY(x)   __builtin_expect(!!(x), 1)
#define UNLIKELY(x) __builtin_expect(!!(x), 0)
#define UNUSED(x) ((void)(x))

#ifdef LEAKDETECTOR
bool leakdetector = true;
#else
bool leakdetector = false;
#endif

// Support tracy.
#ifdef MEM_TRACY_ENABLE
    #include <tracy/TracyC.h>
#endif

struct entry {
    void *ptr;
    size_t size;
    const char *file;
    int line;
    struct entry *next;
};

static struct entry **buckets = NULL;
static size_t buckets_count = 0;
static size_t entries_count = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

static void trap(void) {
    #if defined(__x86_64__) || defined(__amd64__)
    __asm__ volatile ("int $0x03");
    #elif defined(__aarch64__)
    __asm__ volatile ("brk #0");
    #else
    #warning "trap() not implemented for this architecture"
    #endif
}

static void _leakdetector_check(void) {
    if (entries_count > 0) {
        fprintf(stderr, "Memory leak detected! (%zu still alive)\n", entries_count);

        for (size_t i = 0; i < buckets_count; i++) {
            const struct entry *entry = buckets[i];
            while (entry) {
                fprintf(stderr, "\t-> %p allocated at %s:%d (%zu bytes)\n", entry->ptr, entry->file, entry->line, entry->size);
                entry = entry->next;
            }
        }

        trap();
    }
}

static void _setup_atexit_check(void) {
    static bool setup = false;
    if (!setup) {
        atexit(_leakdetector_check);

        size_t initial_buckets_count = 1024;
        buckets = malloc(initial_buckets_count * sizeof *buckets);

        if (UNLIKELY(!buckets)) {
            fprintf(stderr, "Failed to allocate memory for leakdetector buckets\n");
            exit(EXIT_FAILURE);
        }

        buckets_count = initial_buckets_count;

        for (size_t i = 0; i < buckets_count; i++) {
            buckets[i] = NULL;
        }

        setup = true;
    }
}

static inline uintptr_t _hash_ptr(uintptr_t ptr) {
    ptr ^= (ptr >> 33);
    ptr *= 0xff51afd7ed558ccdULL;
    ptr ^= (ptr >> 33);
    ptr *= 0xc4ceb9fe1a85ec53ULL;
    ptr ^= (ptr >> 33);
    return ptr;
}

static void _add(void *ptr, size_t size, const char *file, int line) {
    pthread_mutex_lock(&mutex);

    _setup_atexit_check();

    uintptr_t hash = _hash_ptr((uintptr_t)ptr);
    size_t index = hash % buckets_count;

    struct entry *new_entry = malloc(sizeof *new_entry);
    if (UNLIKELY(!new_entry)) {
        fprintf(stderr, "Failed to allocate memory for leakdetector new entry\n");
        exit(EXIT_FAILURE);
    }

    new_entry->ptr = ptr;
    new_entry->size = size;
    new_entry->file = file;
    new_entry->line = line;
    new_entry->next = buckets[index];
    buckets[index] = new_entry;
    entries_count++;

    // TODO: Resize the hashmap if necessary.

    pthread_mutex_unlock(&mutex);
}

static void _remove(void *ptr) {
    if (!ptr) {
        return;
    }

    pthread_mutex_lock(&mutex);

    bool found = false;

    uintptr_t hash = _hash_ptr((uintptr_t)ptr);
    size_t index = hash % buckets_count;

    struct entry *current = buckets[index];
    struct entry *prev = NULL;

    while (current) {
        if (current->ptr == ptr) {
            if (prev) {
                prev->next = current->next;
            } else {
                buckets[index] = current->next;
            }

            free(current);
            entries_count--;

            found = true;
            break;
        }

        prev = current;
        current = current->next;
    }

    if (!found) {
        fprintf(stderr, "Freeing unallocated memory: %p\n", ptr);
        trap();
    }

    // TODO: Resize the hashmap if necessary.

    pthread_mutex_unlock(&mutex);
}

void *leakdetector_malloc(bool must, size_t size, const char *file, int line) {
    void *ptr = malloc(size);
    if (must && UNLIKELY(!ptr)) {
        fprintf(stderr, "Out of memory\n");
        abort();
    }

    #ifdef MEM_TRACY_ENABLE
    TracyCAlloc(ptr, size);
    #endif

    if (leakdetector) {
        _add(ptr, size, file, line);
    }

    return ptr;
}

char *leakdetector_asprintf(bool must, const char *fmt, const char *file, int line, ...) {
    va_list ap;
    va_start(ap, fmt);
    size_t size = vsnprintf(0, 0, fmt, ap) + 1;
    va_end(ap);

    char *str = leakdetector_malloc(must, size, file, line);
    if (!str) {
        return NULL;
    }

    va_start(ap, fmt);
    if (str) {
        vsnprintf(str, size, fmt, ap);
    }
    va_end(ap);

    return str;
}

void *leakdetector_realloc(bool must, void *ptr, size_t size, const char *file, int line) {
    if (ptr) {
        #ifdef MEM_TRACY_ENABLE
        TracyCFree(ptr);
        #endif

        if (leakdetector) {
            _remove(ptr);
        }
    }

    void *new_ptr = realloc(ptr, size);
    if (must && UNLIKELY(!new_ptr)) {
        fprintf(stderr, "Out of memory\n");
        abort();
    }

    #ifdef MEM_TRACY_ENABLE
    TracyCAlloc(new_ptr, size);
    #endif

    if (leakdetector) {
        _add(new_ptr, size, file, line);
    }

    return new_ptr;
}

char *leakdetector_strdup(bool must, const char *str, const char *file, int line) {
    assert(str);

    size_t len = strlen(str);
    char *dup = leakdetector_malloc(must, len + 1, file, line);
    if (!dup) {
        return NULL;
    }
    memcpy(dup, str, len);
    dup[len] = '\0';

    return dup;
}

void leakdetector_free(void *ptr) {
    if (ptr) {
        #ifdef MEM_TRACY_ENABLE
        TracyCFree(ptr);
        #endif

        if (leakdetector) {
            _remove(ptr);
        }
    }

    free(ptr);
}
