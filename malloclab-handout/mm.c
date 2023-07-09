/*
 * mm-naive.c - Using explicit free list, split and coalesce.
 *
 * block structure:
 * allocated block: | header(size_t) | payload (8*N) bytes | footer(size_t) |
 * free block:      | header(size_t) | prev_ptr | next_ptr | payload (8*N) bytes | footer(size_t) |
 * 
 * In this approach, malloc package uses explicit double-linked list to record the current free blocks,
 * which adopts the LIFO finding and placement rule.
 *
 * Malloc: find a block in the list with bigger size (if not able, require more space by mem_sbrk())
 * mark this block as allocated, delete from list and split the remaining to be a free block 
 * (if the remaining size is large)
 *
 * Free: unmark the allocated block, coalesce (merge with the adjacent free blocks 
 * by removing them and adding result block to list)
 *
 */
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "ateam",
    /* First member's full name */
    "Yuchen Ouyang",
    /* First member's email address */
    "something",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)

#define MAX(a, b) (((a) > (b))? (a) : (b))

/*
 * the block structure:
 * alloc = 0: | header: size_t 4 | prev: ptr 4 | next: ptr 4 | payload 8*N | footer size_t 4|
 * alloc = 1: | header: size_t 4 | payload 8*N | footer size_t 4|
 */
#define HD_SIZE (sizeof(size_t))
#define PTR_SIZE (sizeof(void *))
#define MIN_BLK_SIZE (2 * HD_SIZE + 2 * PTR_SIZE)
/* operate on the header or footer */
#define PACK(size, alloc) ((size) | (alloc))
#define GET_SIZE(p) (*(size_t *)(p) & ~0x7)
#define IS_ALLOC(p) (*(size_t *)(p) & 0x1)
/* get the address of previous or next pointer */
#define GET_PREV(bp) ((char *)(bp) - 2 * PTR_SIZE)
#define GET_NEXT(bp) ((char *)(bp) - PTR_SIZE)
/* put size_t or pointer-size bytes value to the memory location */
#define PUT_SIZE(p, val) (*(size_t *)(p) = (val))
#define PUT_PTR(p, addr) (*(void **)(p) = (addr))
/* get the block pointer in the explicit free list */
#define PREV_NODE(bp) (*(void **)GET_PREV(bp))
#define NEXT_NODE(bp) (*(void **)GET_NEXT(bp))
/* the head/tail block pointer */
static void *HEAD;
static void *TAIL;

/* return the address of header */
static void *HD(void *bp, int alloc) {
    int offset = (!alloc) * 2 * PTR_SIZE + HD_SIZE;
    return (char *)bp - offset;
}

/* return the address of footer */
static void *FT(void *bp, int alloc) {
    char *header = HD(bp, alloc);
    return header + GET_SIZE(header) - HD_SIZE;
}

/* get the previous block pointer in the heap */
static void *LEFT_BLKP(void *bp, int alloc) {
    char *header = HD(bp, alloc);
    int offset = (!IS_ALLOC(header - HD_SIZE)) * 2 * PTR_SIZE;
    int size_prev = GET_SIZE(header - HD_SIZE);
    return header - size_prev + offset + HD_SIZE;
}

/* get the next block pointer in the heap */
static void *RIGHT_BLKP(void *bp, int alloc) {
    char *footer = FT(bp, alloc);
    int offset = (!IS_ALLOC(footer + HD_SIZE)) * 2 * PTR_SIZE;
    return footer + 2 * HD_SIZE + offset;
}

/* 
 * add the free block to the front of free-list,
 * (*bp) should point to the address of **free** block
 */
static void add_to_list(void *bp) {
    void *nx_node = NEXT_NODE(HEAD);
    PUT_PTR(GET_NEXT(bp), nx_node);     /* bp.next = next_node */
    PUT_PTR(GET_PREV(bp), HEAD);        /* bp.prev = head */
    PUT_PTR(GET_PREV(nx_node), bp);     /* next_node.prev = bp */
    PUT_PTR(GET_NEXT(HEAD), bp);        /* head.next = bp */
}

/*
 * delete the free block from list, but leave it unmarked still
 */
static void delete_block(void *bp) {
    if (bp == NULL) return;
    void *prev_node = PREV_NODE(bp);
    void *next_node = NEXT_NODE(bp);
    PUT_PTR(GET_NEXT(prev_node), next_node);
    PUT_PTR(GET_PREV(next_node), prev_node);
}

/* find a free block that can hold the required size of data */
static void *find_free(size_t size) {
    /* traverse the free-list and return the first match block */
    void *ptr = NEXT_NODE(HEAD);
    while (ptr != TAIL && GET_SIZE(HD(ptr, 0)) < size) {
        ptr = NEXT_NODE(ptr);
    }
    return (ptr == TAIL)? NULL : ptr;
}

/*
 * merge the adjacent free-blocks to only one block
 * (*bp) is block pointer of an **free** block
 */
static void *coalesce(void *bp) {
    size_t size = GET_SIZE(HD(bp, 0));
    void *prev_block = LEFT_BLKP(bp, 0);
    void *next_block = RIGHT_BLKP(bp, 0);
    size_t prev_alloc = IS_ALLOC((char *)HD(bp, 0) - HD_SIZE);
    size_t next_alloc = IS_ALLOC((char *)HD(bp, 0) + size);

    if (prev_alloc && next_alloc) {
        // printf("case 1\n");
    }
    else if (prev_alloc && !next_alloc) {
        // printf("case 2\n");
        delete_block(next_block);       /* delete the next_block from free-list, because it no longer exists */
        size += GET_SIZE(HD(next_block, 0));
        PUT_SIZE(HD(bp, 0), PACK(size, 0));
        PUT_SIZE(FT(next_block, 0), PACK(size, 0));
    }
    else if (!prev_alloc && next_alloc) {
        // printf("case 3\n");
        delete_block(prev_block);       /* same for the previous block */
        size += GET_SIZE(HD(prev_block, 0));
        PUT_SIZE(HD(prev_block, 0), PACK(size, 0));
        PUT_SIZE(FT(bp, 0), PACK(size, 0));
        bp = prev_block;
    }
    else {
        // printf("case 4\n");
        delete_block(prev_block);
        delete_block(next_block);
        size += GET_SIZE(HD(prev_block, 0)) + GET_SIZE(HD(next_block, 0));
        PUT_SIZE(HD(prev_block, 0), PACK(size, 0));
        PUT_SIZE(FT(next_block, 0), PACK(size, 0));
        bp = prev_block;
    }

    add_to_list(bp);    /* add free block to list */
    return bp;
}

/* 
 * ask for at least words bytes from heap under the alignment requirement,
 * if run out of space and other error, return NULL
 */
static void *extend_heap(size_t words) {
    char *bp;
    size_t size = MAX(ALIGN(words), mem_pagesize());    /* align requirement */
    
    if ((bp = mem_sbrk(size)) == (void *)(-1))
        return NULL;
    
    PUT_SIZE(HD(bp, 1), PACK(size, 0));                   /* overwrite the old epilogue with header */
    PUT_SIZE(FT(bp, 1), PACK(size, 0));                   /* copy the header to footer */
    PUT_SIZE((char *)FT(bp, 1) + HD_SIZE, PACK(0, 1));    /* new epilogue */
    
    return coalesce((char *)bp + 2 * PTR_SIZE);
}

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    if (mem_sbrk(ALIGN(7 * HD_SIZE + 5 * PTR_SIZE)) == (void *)(-1)) {
        return -1;
    }

    /* initialize the head and tail nodes to be free and empty */
    HEAD = (char *)mem_heap_lo() + 3 * PTR_SIZE + HD_SIZE;
    TAIL = HEAD + MIN_BLK_SIZE;
    // printf("HEAD: %p, TAIL: %p\n", HEAD, TAIL);

    PUT_SIZE(HD(HEAD, 0), PACK(MIN_BLK_SIZE, 0));
    PUT_PTR(GET_PREV(HEAD), NULL);
    PUT_PTR(GET_NEXT(HEAD), TAIL);   /* head -> tail */
    PUT_SIZE(FT(HEAD, 0), PACK(MIN_BLK_SIZE, 0));

    PUT_SIZE(HD(TAIL, 0), PACK(MIN_BLK_SIZE, 0));
    PUT_PTR(GET_PREV(TAIL), HEAD);    /* head <- tail */
    PUT_PTR(GET_NEXT(TAIL), NULL);
    PUT_SIZE(FT(TAIL, 0), PACK(MIN_BLK_SIZE, 0));

    /* initialize the prologue and epilogue, mark as allocated */
    char *prologue = (char *)TAIL + PTR_SIZE;
    PUT_SIZE(prologue, PACK(2 * HD_SIZE, 1));
    PUT_SIZE(prologue + HD_SIZE, PACK(2 * HD_SIZE, 1));

    char *epilogue = (char *)mem_heap_hi() + 1 - HD_SIZE;
    PUT_SIZE(epilogue, PACK(0, 1));

    /* initialize the free block + add to the free-list */
    if (extend_heap(mem_pagesize()) == NULL)
        return -1;
    return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    if (size == 0) return NULL;

    size_t nsize = ALIGN(size) + 2 * HD_SIZE;     /* calculate the space that actually required */
    void *bp = find_free(nsize);                /* find the matched block with larger size */
    if (bp == NULL) {
        /* no matched free block, ask for more heap space */
        if ((bp = extend_heap(nsize)) == NULL) {
            fprintf(stderr, "Run out of heap memory."); 
            exit(1);
        }
    }
    delete_block(bp);       /* remove the block in the list for allocating */

    /* mark the entire free block as allocated */
    void *header = HD(bp, 0);
    void *footer = FT(bp, 0);
    size_t bsize = GET_SIZE(header);
    PUT_SIZE(footer, PACK(bsize, 1));
    PUT_SIZE(header, PACK(bsize, 1));

    if (bsize - nsize >= MIN_BLK_SIZE + ALIGNMENT) {
        /* split the free block */
        PUT_SIZE(header, PACK(nsize, 1));
        void *new_footer = FT(bp, 0);
        PUT_SIZE(new_footer, PACK(nsize, 1));    /* mark the new footer to be allocated */
        PUT_SIZE((char *)new_footer + HD_SIZE, PACK(bsize - nsize, 0));
        PUT_SIZE(footer, PACK(bsize - nsize, 0));       /* unmark the splitted block footer */
        /* add the splitted block to free-list */
        add_to_list((char *)new_footer + 2 * HD_SIZE + 2 * PTR_SIZE);
    }
    return (char *)bp - 2 * PTR_SIZE;
}

/*
 * mm_free - Freeing a block by merging adjacent free blocks and add it to the list
 */
void mm_free(void *ptr)
{
    size_t size = GET_SIZE(HD(ptr, 1));
    PUT_SIZE(HD(ptr, 1), PACK(size, 0));
    PUT_SIZE(FT(ptr, 1), PACK(size, 0));

    coalesce((char *)ptr + 2 * PTR_SIZE);    /* delete the adjacent free blocks from list and merge */
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    void *oldptr = ptr;
    void *newptr;
    size_t copySize;

    newptr = mm_malloc(size);
    if (newptr == NULL)
      return NULL;
    copySize = GET_SIZE((char *)oldptr - HD_SIZE);
    if (size < copySize)
      copySize = size;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}














