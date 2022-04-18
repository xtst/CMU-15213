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
