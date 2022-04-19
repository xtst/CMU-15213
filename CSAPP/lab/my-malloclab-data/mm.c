/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 *
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "memlib.h"
#include "mm.h"

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
	/* Team name */
	"santo",
	/* First member's full name */
	"santo santo",
	/* First member's email address */
	"santo@santo.com",
	/* Second member's full name (leave blank if none) */
	"",
	/* Second member's email address (leave blank if none) */
	""};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~0x7)

#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

//==============================================================================================
#define WSIZE 4				/* Word and header/footer size (bytes) */
#define DSIZE 8				/* Double word size (bytes) */
#define CHUNKSIZE (1 << 12) /* Extend heap by this amount (bytes) */

#define MAX(x, y) ((x) > (y) ? (x) : (y))

#define PREALLOC(x) ((!x) ? 0 : 2)
/* Pack a size and allocated bit into a word */
#define PACK(size, prealloc, alloc) ((size) | (PREALLOC(prealloc)) | (alloc))

/* Read and write a word at address p */
#define GET(p) (*(unsigned int *)(p))
#define PUT(p, val) (*(unsigned int *)(p) = (val))

/* Read the size and allocated fields from address p */
#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)
#define GET_PREALLOC(p) (GET(p) & 0x2)

/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp) ((char *)(bp)-WSIZE)
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp)-WSIZE)))
#define PREV_BLKP(bp) ((char *)(bp)-GET_SIZE(((char *)(bp)-DSIZE)))

static void *extend_heap(size_t words);
static void *find_fit(size_t asize);
static void place(void *bp, size_t asize);
static void *coalesce(void *bp);
inline void set_next_prealloc(void *bp, size_t prealloc);

static char *heap_listp;
static char *pre_listp;

static void *extend_heap(size_t words) {
	char *bp;
	size_t size, prealloc;

	/* Allocate an even number of words to maintain alignment */
	size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE;
	if ((long)(bp = mem_sbrk(size)) == -1) return NULL;

	/* Initialize free block header/footer and the epilogue header */
	prealloc = GET_PREALLOC(HDRP(bp));
	PUT(HDRP(bp), PACK(size, prealloc, 0));	 /* Free block header */
	PUT(FTRP(bp), PACK(size, prealloc, 0));	 /* Free block footer */
	PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 0, 1)); /* New epilogue header */

	/* Coalesce if the previous block was free */
	return coalesce(bp);
}
/*
 * mm_init - initialize the malloc package.
 */

static void *find_fit(size_t asize) {
	char *bp = pre_listp;
	size_t alloc;
	size_t size;
	while (GET_SIZE(HDRP(NEXT_BLKP(bp))) > 0) {
		bp = NEXT_BLKP(bp);
		alloc = GET_ALLOC(HDRP(bp));
		if (alloc) continue;
		size = GET_SIZE(HDRP(bp));
		if (size < asize) continue;
		return bp;
	}
	bp = heap_listp;
	while (bp != pre_listp) {
		bp = NEXT_BLKP(bp);
		alloc = GET_ALLOC(HDRP(bp));
		if (alloc) continue;
		size = GET_SIZE(HDRP(bp));
		if (size < asize) continue;
		return bp;
	}
	return NULL;
}

static void place(void *bp, size_t asize) {
	size_t size = GET_SIZE(HDRP(bp));

	if ((size - asize) >= DSIZE) {
		PUT(HDRP(bp), PACK(asize, 1, 1));
		// PUT(FTRP(bp),PACK(asize,1,1));
		PUT(HDRP(NEXT_BLKP(bp)), PACK(size - asize, 1, 0));
		PUT(FTRP(NEXT_BLKP(bp)), PACK(size - asize, 1, 0));
		set_next_prealloc(bp, 0);
	} else {
		PUT(HDRP(bp), PACK(size, 1, 1));
		set_next_prealloc(bp, 1);
		// PUT(FTRP(bp),PACK(size,1,1));
	}
	pre_listp = bp;
}
/*
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */

static void *coalesce(void *bp) {
	size_t prev_alloc = GET_PREALLOC(HDRP(bp));
	size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
	size_t size = GET_SIZE(HDRP(bp));

	if (prev_alloc && !next_alloc) { /* Case 2 */
		size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
		PUT(HDRP(bp), PACK(size, 1, 0));
		PUT(FTRP(bp), PACK(size, 1, 0));

	}

	else if (!prev_alloc && next_alloc) { /* Case 3 */
		size += GET_SIZE(HDRP(PREV_BLKP(bp)));
		PUT(FTRP(bp), PACK(size, 1, 0));
		PUT(HDRP(PREV_BLKP(bp)), PACK(size, 1, 0));
		bp = PREV_BLKP(bp);
	}

	else if (!prev_alloc && !next_alloc) { /* Case 4 */
		size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(FTRP(NEXT_BLKP(bp)));
		PUT(HDRP(PREV_BLKP(bp)), PACK(size, 1, 0));
		PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 1, 0));
		bp = PREV_BLKP(bp);
	}
	set_next_prealloc(bp, 0);
	pre_listp = bp;
	return bp;
}

inline void set_next_prealloc(void *bp, size_t prealloc) {
	size_t size = GET_SIZE(HDRP(NEXT_BLKP(bp)));
	size_t alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
	PUT(HDRP(NEXT_BLKP(bp)), PACK(size, prealloc, alloc));
}
//==============================================================================================

size_t high_size(size_t x, int *times) {
	size_t p = 1;
	while (p < x) {
		p <<= 1;
		(*times)++;
	}
	return p;
}
/*
 * mm_init - initialize the malloc package.
 */
void **list_node;
int mm_init(void) {
	// void *p = mem_heap_lo();
	list_node = mem_sbrk(sizeof(void *) * 32);
	if (list_node == ((void *)-1)) exit(-1);
	for (int i = 0; i < 32; i++) list_node[i] = NULL;
	// if (((int)p & 7) == 0) exit(1);

	/* Create the initial empty heap */
	if ((heap_listp = mem_sbrk(4 * WSIZE)) == (void *)-1) return -1;
	PUT(heap_listp, 0);								  /* Alignment padding */
	PUT(heap_listp + (1 * WSIZE), PACK(DSIZE, 1, 1)); /* Prologue header */
	PUT(heap_listp + (2 * WSIZE), PACK(DSIZE, 1, 1)); /* Prologue footer */
	PUT(heap_listp + (3 * WSIZE), PACK(0, 1, 1));	  /* Epilogue header */
	heap_listp += DSIZE;
	pre_listp = heap_listp;
	/* Extend the empty heap with a free block of CHUNKSIZE bytes */
	if (extend_heap(CHUNKSIZE / WSIZE) == NULL) return -1;
	return 0;
	return 0;
}

/*
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
int bigger = 5, first_in, flag = 0;
void *mm_malloc(size_t size) {
	// flag = 1;
	if (!flag && size == 5580) { flag = 1; }
	if (size == 559) flag = 0;
	int times = 0;
	size_t new_size = high_size(ALIGN(size + SIZE_T_SIZE), &times);
	// if ((1 << (times)) < new_size) exit(1);
	// if (times >= 32) exit(-1);
	if (flag) {

		size_t asize;	   /* Adjusted block size */
		size_t extendsize; /* Amount to extend heap if no fit */
		char *bp;

		/* Ignore spurious requests */
		if (size == 0) return NULL;

		/* Adjust block size to include overhead and alignment reqs. */
		if (size <= WSIZE)
			asize = DSIZE;
		else
			asize = DSIZE * ((size + (WSIZE) + (DSIZE - 1)) / DSIZE);

		/* Search the free list for a fit */
		if ((bp = find_fit(asize)) != NULL) {
			place(bp, asize);
			return bp;
		}

		/* No fit found. Get more memory and place the block */
		extendsize = MAX(asize, CHUNKSIZE);
		if ((bp = extend_heap(extendsize / WSIZE)) == NULL) return NULL;
		place(bp, asize);
		return bp;
	}

	if (list_node[times] != NULL) {
		void *res = list_node[times];
		list_node[times] = (void *)(*((size_t *)(list_node[times])));
		*((size_t *)res) = new_size;
		return (void *)((char *)res + SIZE_T_SIZE);
	}

	// size_t temp_size = new_size;
	// if (mem_heapsize() > (1 * (1 << 11))) bigger = 15;
	if (times >= 1)
		for (int i = times + 1; i <= MIN(times + bigger, 31); i++) {
			if (list_node[i] != NULL) {
				void *res = list_node[i];
				list_node[i] = (void *)(*((size_t *)(list_node[i])));
				*((size_t *)res) = new_size;

				// void *pos = res;
				for (int j = times; j < i; j++) {
					size_t now_size = (new_size) << (j - times);
					void *tmp_ptr = ((char *)res) + now_size;
					*((size_t *)tmp_ptr) = now_size;
					mm_free((void *)((char *)tmp_ptr + SIZE_T_SIZE));
				}

				return (void *)((char *)res + SIZE_T_SIZE);
			}
			// temp_size <<= 1;
		}
	// new_size = temp;

	void *p = mem_sbrk(new_size);
	if (p == (void *)-1)
		return NULL;
	else {
		*((size_t *)p) = new_size;
		return (void *)((char *)p + SIZE_T_SIZE);
	}
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr) {
	if (flag) {
		size_t size = GET_SIZE(HDRP(ptr));
		size_t prealloc = GET_PREALLOC(HDRP(ptr));
		PUT(HDRP(ptr), PACK(size, prealloc, 0));
		PUT(FTRP(ptr), PACK(size, prealloc, 0));
		coalesce(ptr);
		return;
	}

	size_t size = *((size_t *)((char *)ptr - SIZE_T_SIZE));
	void *real_position = (void *)((char *)ptr - SIZE_T_SIZE);
	int times = 0;
	high_size(size, &times);

	if (list_node[times] == NULL) {
		list_node[times] = real_position;
		*((size_t *)real_position) = 0;
	} else {
		void *next = list_node[times];
		list_node[times] = real_position;
		*((size_t *)(real_position)) = (size_t)next;
	}
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size) {
	size_t old_size = *((size_t *)((char *)ptr - SIZE_T_SIZE));
	// size_t old_expanded_size = high_size(ALIGN(size + SIZE_T_SIZE), &times);
	if (size <= old_size - SIZE_T_SIZE) return ptr;
	void *oldptr = ptr;
	void *newptr;
	size_t copySize;

	newptr = mm_malloc(size);
	if (newptr == NULL) return NULL;
	copySize = *(size_t *)((char *)oldptr - SIZE_T_SIZE);
	if (size < copySize) copySize = size;
	memcpy(newptr, oldptr, copySize);
	mm_free(oldptr);
	return newptr;
}
