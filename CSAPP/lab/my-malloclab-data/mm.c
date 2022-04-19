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
	return 0;
}

/*
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
int bigger = 5, first_in, flag = 0;
void *mm_malloc(size_t size) {
	// if (!flag && size == 5580) {}
	int times = 0;
	size_t new_size = high_size(ALIGN(size + SIZE_T_SIZE), &times);
	// if ((1 << (times)) < new_size) exit(1);
	// if (times >= 32) exit(-1);

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
