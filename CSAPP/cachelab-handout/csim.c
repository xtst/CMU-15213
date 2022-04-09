#include "cachelab.h"
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define debug(x)                                                                                                                           \
	if (debugMod) { printf("%s\t=%d\n", (#x), x); }
#define sdebug(x)                                                                                                                          \
	if (debugMod) { printf("%s=%s\n", (#x), x); }

int debugMod;
int s, E, b;
int miss, hit, eviction;

int main(int argc, char **argv) {
	char command;
	char fileName[20];
	while ((command = getopt(argc, argv, "vs:E:b:t:")) != -1) {
		if (command == 'v') {
			debugMod = 1;
			debug(debugMod);
		} else if (command == 't') {
			strcpy(fileName, optarg);
			sdebug(fileName);
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
	FILE *fp = fopen(fileName, "r");
	if (fp == NULL) {
		printf("FileName Wrong\n");
		return 0;
	}

	char option[10];
	while (fscanf(fp, "%s", option) != -1) {
		// sdebug(s);
		char address[15], size[15], string[30];
		// unsigned int addr;

		fscanf(fp, "%s", string);
		for (int i = 0; i < strlen(string); i++)
			if (string[i] == ',') string[i] = ' ';

		sscanf(string, "%s%s", address, size);
		// debug(addr);
		sdebug(address);
		sdebug(size);
	}
	printSummary(hit, miss, eviction);
	return 0;
}
