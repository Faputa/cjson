#include "cjson.h"
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>

#define MAXSIZE 1000
#define INDENT 4

static int tki;
static char *tks, *p;

static void next() {
	char *keyword[] = {
		"null", "false", "true"
	};
	char *trans[] = {
		"\\n", "\n",
		"\\\\", "\\",
		"\\t", "\t",
		"\\\"", "\"",
		"\\0", "\0"
	};
	tks = ""; tki = -1;
	while(*p) {
		if((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z') || *p == '_') {
			int len = 0; char *_p = p;
			while((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z') || (*p >= '0' && *p <= '9') || *p == '_') {
				len++; p++;
			}
			for(int i = 0; i < sizeof(keyword) / sizeof(*keyword); i++) {
				if(strlen(keyword[i]) == len && !strncmp(keyword[i], _p, len)) {
					tki = i;
					return;
				}
			}
			printf("error1!\n"); exit(-1);
		} else if(*p >= '0' && *p <= '9') {
			int len = 0; char *_p = p;
			while(*p >= '0' && *p <= '9') {
				len++; p++;
				if(*p == '.') {
					len++; p++;
					while(*p >= '0' && *p <= '9') {
						len++; p++;
					}
					break;
				}
			}
			tki = CJSON_NUM;
			tks = (char*)malloc(sizeof(char) * (len+1));
			strncpy(tks, _p, len);
			tks[len] = '\0';
			return;
		} else if(*p == '"') {
			tki = CJSON_STR;
			int len = 0; char *_p = ++p;
			while(*p && *p != '"') {
				if(*p++ == '\\') p++;
				len++;
			}
			if(*p) p++;
			tks = (char*)malloc(sizeof(char) * (len+1));
			int i = 0;
			while(*_p != '"') {
				if(*_p == '\\') {
					for(int j = 0; j < sizeof(trans) / sizeof(*trans); j+=2) {
						if(!strncmp(trans[j], _p, strlen(trans[j]))) {
							tks[i] = *trans[j+1];
							_p += strlen(trans[j]);
							break;
						}
					}
				}
				else tks[i] = *_p++;
				i++;
			}
			tks[i] = '\0';
			return;
		}
		else if(*p == '{') { tks = "{"; p++; return; }
		else if(*p == '}') { tks = "}"; p++; return; }
		else if(*p == '[') { tks = "["; p++; return; }
		else if(*p == ']') { tks = "]"; p++; return; }
		else if(*p == ':') { tks = ":"; p++; return; }
		else if(*p == ',') { tks = ","; p++; return; }
		else if(*p == ' ' || *p == '\t' || *p == '\n') p++;
		else { printf("error2!\n"); exit(-1); }
	}
}

static cjson_Node* newNode() {
	cjson_Node *node = (cjson_Node*)malloc(sizeof(cjson_Node));
	node->name = "";
	node->next = NULL;
	node->child = NULL;
	return node;
}

static cjson_Node* parseArr();
static cjson_Node* parseObj() {
	assert(!strcmp(tks, "{"));
	next();
	cjson_Node *node = newNode();
	node->type = CJSON_OBJ;
	if(strcmp(tks, "}")) {
		while(1) {
			cjson_Node *child;
			char *name;
			if(tki == CJSON_STR) { name = tks; next(); } else { printf("error3!\n"); exit(-1); }
			if(!strcmp(tks, ":")) next(); else { printf("error4!\n"); exit(-1); }
			if(tki == CJSON_NUL || tki == CJSON_FALSE || tki == CJSON_TRUE) { child = newNode(); child->type = tki; }
			else if(tki == CJSON_NUM) { child = newNode(); child->type = tki; child->num = atof(tks); }
			else if(tki == CJSON_STR) { child = newNode(); child->type = tki; child->str = tks; }
			else if(!strcmp(tks, "{")) child = parseObj();
			else if(!strcmp(tks, "[")) child = parseArr();
			else { printf("error2!\n"); exit(-1); }
			cjson_addNodeToObj(node, name, child);
			next();
			if(!strcmp(tks, "}")) break;
			else if(!strcmp(tks, ",")) next();
			else { printf("error5!\n"); exit(-1); }
		}
	}
	return node;
}

static cjson_Node* parseArr() {
	assert(!strcmp(tks, "["));
	next();
	cjson_Node *node = newNode();
	node->type = CJSON_ARR;
	if(strcmp(tks, "]")) {
		cjson_Node *i = NULL;
		while(1) {
			cjson_Node *child;
			if(tki == CJSON_NUL || tki == CJSON_FALSE || tki == CJSON_TRUE) { child = newNode(); child->type = tki; }
			else if(tki == CJSON_NUM) { child = newNode(); child->type = tki; child->num = atof(tks); }
			else if(tki == CJSON_STR) { child = newNode(); child->type = tki; child->str = tks; }
			else if(!strcmp(tks, "{")) child = parseObj();
			else if(!strcmp(tks, "[")) child = parseArr();
			else { printf("error2!\n"); exit(-1); }
			// cjson_addNodeToArr(node, child);
			if(i == NULL) node->child = child;
			else i->next = child;
			next();
			if(!strcmp(tks, "]")) break;
			else if(!strcmp(tks, ",")) { i = child; next(); }
			else { printf("error6!\n"); exit(-1); }
		}
	}
	return node;
}

void cjson_addNodeToObj(cjson_Node* obj, char *name, cjson_Node* node) {
	assert(obj != NULL);
	assert(obj->type == CJSON_OBJ);
	assert(node != NULL);
	node->name = name;
	if(obj->child == NULL) { obj->child = node; return; }
	cjson_Node *i = obj;
	cjson_Node *j = obj->child;
	while(j != NULL) {
		if(!strcmp(j->name, node->name)) {
			i->next = node;
			node->next = j->next;
			cjson_delete(j);
			return;
		}
		i = j;
		j = j->next;
	}
	i->next = node;
}

void cjson_addNodeToArr(cjson_Node* arr, cjson_Node* node) {
	assert(arr != NULL);
	assert(arr->type == CJSON_ARR);
	assert(node != NULL);
	if(arr->child == NULL) { arr->child = node; return; }
	cjson_Node *i = arr;
	cjson_Node *j = arr->child;
	while(j != NULL) {
		i = j;
		j = j->next;
	}
	i->next = node;
}

cjson_Node* cjson_parse(char *json) {
	p = json;
	cjson_Node *node;
	next();
	if(!strcmp(tks, "{")) node = parseObj();
	else if(!strcmp(tks, "[")) node = parseArr();
	else { printf("error7!\n"); exit(-1); }
	return node;
}

void cjson_delete(cjson_Node *node) {
	if(!node) return;
	switch(node->type) {
	case CJSON_NUL: case CJSON_FALSE: case CJSON_TRUE: case CJSON_STR: break;
	case CJSON_ARR: case CJSON_OBJ: for(cjson_Node *c = node->child; c != NULL; c = c->next) { cjson_delete(c); } break;
	default: assert(0);
	}
	free(node);
}

static void catstr(char *dest, char *fmt, ...) {
	char *s = dest + strlen(dest);
	va_list args;
	va_start(args, fmt);
	vsprintf(s, fmt, args);
	va_end(args);
}

static void printFloat(char *s, float f) {
	char buf[40], *i, *j;
	sprintf(buf, "%f", f);
	i = buf;
	while(*i != '.') i++;
	j = i + 1;
	while(*j) {
		if(*j != '0') i = j + 1;
		j++;
	}
	*i = '\0';
	catstr(s, "%s", buf);
}

static void printIndent(char *s, int indent) {
	for(int i = 0; i < indent * INDENT; i++) catstr(s, " ");
}

static void printEndl(char *s, cjson_Node *node) {
	catstr(s, node->next != NULL ? ",\n" : "\n");
}

static void print(char *s, cjson_Node *node, int indent) {
	printIndent(s, indent);
	if(strcmp(node->name, "")) catstr(s, "\"%s\" : ", node->name);
	switch(node->type) {
	case CJSON_NUL: catstr(s, "null"); break;
	case CJSON_FALSE: catstr(s, "false"); break;
	case CJSON_TRUE: catstr(s, "true"); break;
	case CJSON_NUM: printFloat(s, node->num); break;
	case CJSON_STR: catstr(s, "\"%s\"", node->str); break;
	case CJSON_OBJ: catstr(s, "{\n");
		for(cjson_Node *i = node->child; i != NULL; i = i->next) print(s, i, indent + 1);
		printIndent(s, indent); catstr(s, "}"); break;
	case CJSON_ARR: catstr(s, "[\n");
		for(cjson_Node *i = node->child; i != NULL; i = i->next) print(s, i, indent + 1);
		printIndent(s, indent); catstr(s, "]"); break;
	default: assert(0);
	}
	printEndl(s, node);
}

char* cjson_print(cjson_Node *node) {
	char *s = malloc(MAXSIZE * sizeof(char));
	*s = '\0';
	print(s, node, 0);
	return s;
}
