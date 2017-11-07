#include "cjson.h"
#include <stdio.h>
#include <malloc.h>

#define MAXSIZE 1000

int main(int argc, char *argv[]) {
	FILE *fp;
	if(argc != 2) { printf("error13!\n"); exit(-1); }
	if(!(fp = fopen(argv[1], "r"))) { printf("error14!\n"); exit(-1); }
	char *p = (char*)malloc(MAXSIZE * sizeof(char));
	{ int i = fread(p, sizeof(char), MAXSIZE, fp); p[i] = '\0'; }
	fclose(fp);
	
	printf(cjson_print(cjson_parse(p)));
}