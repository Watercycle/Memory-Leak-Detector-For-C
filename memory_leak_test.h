/*
 * This is a simple memory leak detector!  Simple because it will only catch standard
 * uses of malloc, calloc, realloc, and free.  Depending on what is used in your project,
 * you may want to extend the list of leaky functions to look for, say... fopen and fclose.
 *
 * Note: If you get any strange errors, the first thing you should try is renaming the #defines to a unique name
 * (my_malloc) and use those.  Some standard libraries, like Cygwin stdlib.h, mark these memory functions as
 * 'no_throw'.  But, with modern IDEs you should be able to replace all 'free', for example, with 'my_free', quite
 * easily.  If you really want to override the original names (in this case for Cygwin) you will have to delete the
 * no_throw definitions in Cygwin's stdlib.h and import malloc from malloc.h instead of stdlib.h.
 *
 * Usage:
 *   1) put 'include "memory_leak_test.h"' as the last include of every source file
 *   2) Whenever you want a status report, run 'leak_check_run_and_print();'
 *   3) run your program and forget about Valgrind!
 */

#ifndef MEMORY_LEAK_TEST_H
#define MEMORY_LEAK_TEST_H

#include <stdlib.h>

/* Public Methods
// ============================================================================ */

void leak_check_run_and_print(void); /* typically used with atexit() */

/* Implementation Details
// ============================================================================ */

#define malloc(sz)     _leak_check_malloc(sz, __FILE__, __LINE__)
#define calloc(sz)     _leak_check_calloc(sz, __FILE__, __LINE__)
#define realloc(p, sz) _leak_check_realloc(p,sz, __FILE__, __LINE__)
#define free(p)        _leak_check_free((void**)&p)

void* _leak_check_malloc(size_t size, char* file, int line);
void* _leak_check_calloc(size_t size, char* file, int line);
void* _leak_check_realloc(void* data, size_t size, char* file, int line);
void _leak_check_free(void** data);

#endif /* MEMORY_LEAK_TEST_H */
