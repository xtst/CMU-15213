# Part A
给你s,E,b，模拟缓存的过程，利用lru策略，替换最久没用过的缓存。
```
#include "cachelab.h"
#include <ctype.h>
#include <getopt.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define mbit 64
#define debug(x)                                                                                                                           \
	if (debug_mod) { printf("%s\t=%d\n", (#x), x); }
#define sdebug(x)                                                                                                                          \
	if (debug_mod) { printf("%s=%s\n", (#x), x); }
int debug_mod;
int miss, hit, eviction;
int ***cache;

void change_to_bits(char *s, char *address) {
	int len = strlen(s), tot = 0;
	for (int i = len; i < 16; i++)
		for (int j = 0; j < 4; j++) address[tot++] = '0';
	for (int i = 0; i < len; i++) {
		int value = (isalpha(s[i])) ? (s[i] - 'a' + 10) : s[i] - '0';
		for (int j = 3; j >= 0; j--) { address[tot++] = ((value >> j) & 1) + '0'; }
	}
	address[tot] = 0;
}

void malloc_space(int s, int E, int b) {
	int S = pow(2, s);
	cache = (int ***)malloc(S * sizeof(int **));
	for (int i = 0; i < S; i++) {
		cache[i] = (int **)malloc(E * sizeof(int *));
		for (int j = 0; j < E; j++) {
			cache[i][j] = (int *)malloc((mbit + 5) * sizeof(int));
			// printf("====== %d %d %d\n", i, j, (mbit));
			cache[i][j][0] = 0;
		}
	}
}

void collect_garbage(int s, int E, int b) {
	int S = pow(2, s);
	for (int i = 0; i < S; i++) {
		for (int j = 0; j < E; j++) { free(cache[i][j]); }
		free(cache[i]);
	}
	free(cache);
}

void miss_cache() {
	miss++;
	if (debug_mod) printf(" miss");
}
void hit_cache() {
	hit++;
	if (debug_mod) printf(" hit");
}
void eviction_cache() {
	eviction++;
	if (debug_mod) printf(" eviction");
}

int get_s(int s, int E, int b, char *address) {
	int res = 0;
	for (int i = mbit - b - s; i < mbit - b; i++) {
		res <<= 1;
		res |= (address[i] - '0') & 1;
	}
	return res;
}
void replace_cache(int s, int E, int b, int id, char *address, int s_now) {
	int t = mbit - s - b, min_id = 1e9;
	int E_now = 0;
	for (int i = 0; i < E; i++) {
		if (cache[s_now][i][0] < min_id) {
			min_id = cache[s_now][i][0];
			E_now = i;
		}
	}
	if (cache[s_now][E_now][0] != 0) { eviction_cache(); }
	cache[s_now][E_now][0] = id;
	for (int i = 1; i <= t; i++) { cache[s_now][E_now][i] = address[i - 1] - '0'; }
}

void load_cache(int s, int E, int b, int id, char *address, int go_replace) {
	int t = mbit - s - b;
	int s_now = get_s(s, E, b, address);
	debug(s_now);
	for (int i = 0; i < E; i++) {
		// debug(s_now);
		// debug(i);
		if (cache[s_now][i][0]) {
			int flag = 1;
			for (int j = 1; j <= t; j++) {
				if (address[j - 1] - '0' != cache[s_now][i][j]) {
					flag = 0;
					break;
				}
			}
			if (flag) {
				hit_cache();
				return;
			}
		}
	}
	miss_cache();
	if (go_replace) replace_cache(s, E, b, id, address, s_now);
	return;
}

void process(int s, int E, int b, int id, char *option, char *address) {
	if (option[0] == 'I') return;
	if (option[0] == 'L') { load_cache(s, E, b, id, address, 1); }
	if (option[0] == 'M') {
		load_cache(s, E, b, id, address, 1);
		load_cache(s, E, b, id, address, 0);
	}
	if (option[0] == 'S') {
		load_cache(s, E, b, id, address, 1);
		// load_cache(s, E, b, id, address, 0);
	}
}

int main(int argc, char **argv) {
	int s, E, b;
	char command;
	char file_name[20];
	while ((command = getopt(argc, argv, "vs:E:b:t:")) != -1) {
		if (command == 'v') {
			debug_mod = 1;
			debug(debug_mod);
		} else if (command == 't') {
			strcpy(file_name, optarg);
			sdebug(file_name);
		} else {
			int value = atoi(optarg);
			if (command == 's') s = value;
			if (command == 'E') E = value;
			if (command == 'b') b = value;
			debug(s);
			debug(E);
			debug(b);
		}
	}
	FILE *fp = fopen(file_name, "r");
	if (fp == NULL) {
		printf("file_name Wrong\n");
		return 0;
	}

	malloc_space(s, E, b);

	char option[10];
	int id = 0;
	while (fscanf(fp, "%s", option) != -1) {
		id++;
		char temp[15], address[80], size[15], string[30];

		fscanf(fp, "%s", string);
		for (int i = 0; i < strlen(string); i++)
			if (string[i] == ',') string[i] = ' ';

		sscanf(string, "%s%s", temp, size);

		change_to_bits(temp, address);

		sdebug(address);
		process(s, E, b, id, option, address);
		if (debug_mod) putchar('\n');
	}
	printSummary(hit, miss, eviction);
	return 0;
}

```
# Part B
让你给矩阵转置设计最少的miss的策略，`s=5,E=1,b=5`，主要思想是根据`s,E,b`分块
```
void trans(int M, int N, int A[N][M], int B[M][N]) {
	int i, j, ii, jj, a1, a2, a3, a4, a5, a6, a7, a0;
	if (M == 32) {

		for (i = 0; i < N; i += 8) {
			for (j = 0; j < M; j += 8) {
				for (ii = i; ii < i + 8; ii++) {
					jj = j;
					a0 = A[ii][jj];
					a1 = A[ii][jj + 1];
					a2 = A[ii][jj + 2];
					a3 = A[ii][jj + 3];
					a4 = A[ii][jj + 4];
					a5 = A[ii][jj + 5];
					a6 = A[ii][jj + 6];
					a7 = A[ii][jj + 7];
					B[jj][ii] = a0;
					B[jj + 1][ii] = a1;
					B[jj + 2][ii] = a2;
					B[jj + 3][ii] = a3;
					B[jj + 4][ii] = a4;
					B[jj + 5][ii] = a5;
					B[jj + 6][ii] = a6;
					B[jj + 7][ii] = a7;
				}
			}
		}
	} else if (N == 64 && M == 64) {
		int t0, t1, t2, t3, t4, t5, t6, t7;
		for (int i = 0; i < N; i += 8) {
			for (int j = 0; j < M; j += 8) {
				for (int k = i; k < i + 4; k++) {
					t0 = A[k][j];
					t1 = A[k][j + 1];
					t2 = A[k][j + 2];
					t3 = A[k][j + 3];
					t4 = A[k][j + 4];
					t5 = A[k][j + 5];
					t6 = A[k][j + 6];
					t7 = A[k][j + 7];
					B[j][k] = t0;
					B[j + 1][k] = t1;
					B[j + 2][k] = t2;
					B[j + 3][k] = t3;
					B[j + 0][k + 4] = t7;
					B[j + 1][k + 4] = t6;
					B[j + 2][k + 4] = t5;
					B[j + 3][k + 4] = t4;
				}
				for (int h = 0; h < 4; h++) {
					t0 = A[i + 4][j + 3 - h];
					t1 = A[i + 5][j + 3 - h];
					t2 = A[i + 6][j + 3 - h];
					t3 = A[i + 7][j + 3 - h];
					t4 = A[i + 4][j + 4 + h];
					t5 = A[i + 5][j + 4 + h];
					t6 = A[i + 6][j + 4 + h];
					t7 = A[i + 7][j + 4 + h];
					B[j + 4 + h][i + 0] = B[j + 3 - h][i + 4];
					B[j + 4 + h][i + 1] = B[j + 3 - h][i + 5];
					B[j + 4 + h][i + 2] = B[j + 3 - h][i + 6];
					B[j + 4 + h][i + 3] = B[j + 3 - h][i + 7];
					B[j + 3 - h][i + 4] = t0;
					B[j + 3 - h][i + 5] = t1;
					B[j + 3 - h][i + 6] = t2;
					B[j + 3 - h][i + 7] = t3;
					B[j + 4 + h][i + 4] = t4;
					B[j + 4 + h][i + 5] = t5;
					B[j + 4 + h][i + 6] = t6;
					B[j + 4 + h][i + 7] = t7;
				}
			}
		}
	} else {
		for (i = 0; i < N; i += 16) {
			for (j = 0; j < M; j += 16) {
				for (ii = i; ii < i + 16 && ii < N; ii++) {
					for (jj = j; jj < j + 16 && jj < M; jj++) {
						a0 = A[ii][jj];
						B[jj][ii] = a0;
					}
				}
			}
		}
	}
}
```

# 最终结果
```
Cache Lab summary:
                        Points   Max pts      Misses
Csim correctness          21.0        27
Trans perf 32x32           0.0         8     invalid
Trans perf 64x64           0.0         8     invalid
Trans perf 61x67           0.0        10     invalid
          Total points    21.0        53
```