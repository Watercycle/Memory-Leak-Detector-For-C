#include "memory_leak_test.h"

/* no infinite loops for us! */
#ifdef malloc
    #undef malloc
    #undef calloc
    #undef free
    #undef realloc
#endif

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <time.h>

typedef struct MallocInfo
{
    struct MallocInfo *next;    /* doubly linked list */
    int freed;                  /* set to 1 once free() is called */
    int line;                   /* the line the malloc occured on */
    char* filename;             /* file malloc occured in (e.g. main.c) */
    float time;                 /* processor time elapsed up to the malloc */
    size_t size;                /* malloc size allocated in bytes */
} MallocInfo;

static MallocInfo* alloc_head = NULL;
static MallocInfo* alloc_tail = NULL;

/* Pointer Arithemtic: ptr + num => ptr + (num * (sizeof(*ptr))). */
void* _leak_check_malloc(size_t size, char* file, int line)
{
    MallocInfo* info_data = malloc(size + sizeof(MallocInfo)); /* store our data on the end */
    if (info_data == NULL) return NULL; /* malloc failed to get us memory! >:( */

    /* the member variables know their spot, don't worry! */
    info_data->filename = strrchr(file, '/') ? strrchr(file, '/') + 1 : file;
    info_data->line = line;
    info_data->time = (float)clock() / CLOCKS_PER_SEC;
    info_data->size = size;
    info_data->freed = 0;

    alloc_head ? (alloc_head->next = info_data) : (alloc_tail = info_data);
    alloc_head = info_data;
    alloc_head->next = NULL;

    return info_data + 1; /* give the user their data, not info and data! */
}

void* _leak_check_calloc(size_t size, char* file, int line)
{
    MallocInfo* data = _leak_check_malloc(size, file, line);
    if (data) memset(data, 0, size);
    return data;
}

void _leak_check_free(void** data)
{
    MallocInfo* info_data;

    if (*data == NULL) return; /* sure, we'll accept nulls! */

    /* only user code is deleted, the info segment is saved for final processing */
    info_data = (MallocInfo*)(*data) - 1;
    info_data = realloc(info_data, sizeof(MallocInfo));

    /* nullify the freed address */
    *data = NULL;

    info_data->freed = 1;
}

void* _leak_check_realloc(void* data, size_t size, char* file, int line)
{
    void* data_with_more_space;
    MallocInfo* info_data;

    if (data == NULL) return _leak_check_malloc(size, file, line); /* given empty memory, just malloc it! */
    if (size == 0) return NULL; /* user is being silly! this part is normally implementation defined. */

    info_data = (MallocInfo*)data - 1; /* recover our part to the memory */
    if (size <= info_data->size) return data; /* stupid user, you have enough memory already! */

    /* oh, i guess they really do need more! */
    data_with_more_space = _leak_check_malloc(size, file, line);
    if (data_with_more_space)
    {
        memcpy(data_with_more_space, data, info_data->size);
        _leak_check_free(data); /* free our old space! */
    }

    return data_with_more_space;
}

/* __attribute__((destructor))  */
void leak_check_run_and_print(void)
{
    size_t total_unfreed_allocs = 0, total_allocs = 0, total_leaked = 0;
    MallocInfo *info_data = alloc_tail, *next;

    /* go through each malloc info! */
    while (info_data)
    {
        next = info_data->next;
        total_allocs++;

        if (info_data->freed)
        {
            /* free up the rest of our MallocInfo only for freed blocks.  Since this isn't a garbage collector!*/
            free(info_data);
        }
        else
        {
            total_leaked += info_data->size;
            total_unfreed_allocs++;
            printf("WARNING: leaked '%u' bytes at address '%p' in file '%s' on line '%i' after %.2fs\n",
                   (unsigned)info_data->size, (void*)(info_data + 1), info_data->filename,
                   info_data->line, info_data->time);
        }

        info_data = next;
    }

    printf("MEMORY REPORT: '%u' alloc(s) made.  '%u' not freed (total of '%u' bytes leaked).\n",
           (unsigned)total_allocs, (unsigned)total_unfreed_allocs, (unsigned)total_leaked);
}
