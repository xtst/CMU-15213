#include "csapp.h"
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

typedef struct _obj_t {
	char flag;
	char uri[100];
	char respHeader[1024];
	char respBody[MAX_OBJECT_SIZE];
	int respHeaderLen;
	int respBodyLen;
	struct _obj_t *prev;
	struct _obj_t *next;
} obj_t;

typedef struct _cache_t {
	obj_t *head;
	obj_t *tail;
	int nitems;
} cache_t;

// write to cache
// read cache
// search cache

void initializeCache(cache_t *);