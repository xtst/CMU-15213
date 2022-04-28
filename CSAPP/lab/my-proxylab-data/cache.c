#include "cache.h"
#include "csapp.h"

void initializeCache(cache_t *cache) {
	cache->head = Malloc(sizeof(*(cache->head)));
	cache->head->flag = '@';
	cache->head->prev = NULL;
	cache->head->next = NULL;

	cache->tail = Malloc(sizeof(*(cache->tail)));
	cache->tail->flag = '@';
	cache->tail->prev = NULL;
	cache->tail->next = NULL;

	/* construct the doubly linked list */
	cache->head->next = cache->tail;
	cache->tail->prev = cache->head;

	cache->nitems = 0;
}