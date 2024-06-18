#ifndef SMALLOC_H
#define SMALLOC_H

#include <stddef.h>
#include <stdint.h>

/**
 * Memory header structure for managing memory blocks.
 */
struct _smheader {
    size_t size;          // Size of the memory block
    uint8_t used;         // 1 if used, 0 if free
    struct _smheader *next;  // Pointer to the next memory block header
};

typedef struct _smheader  smheader;
typedef struct _smheader* smheader_ptr;

/**
 * Memory allocation modes.
 */
typedef enum smmode {
    bestfit,
    worstfit,
    firstfit
} smmode;


// Function declarations
void *smalloc(size_t size);
void *smalloc_mode(size_t size, smmode mode);
void sfree(void *ptr);
void *srealloc(void *ptr, size_t size);
void smcoalesce(void);  
void smdump(void);


#endif // SMALLOC_H

