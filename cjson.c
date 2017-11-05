#include "cjson.h"
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <assert.h>

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
			if(*p) p++;//sprintf(s, "%d\n",len);
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

static void parseArr(cjson_Node *node);
static void parseObj(cjson_Node *node) {
	assert(!strcmp(tks, "{"));
	next();
	node->type = CJSON_OBJ;
	if(strcmp(tks, "}")) {
		while(1) {
			cjson_Node *child = newNode();
			if(tki == CJSON_STR) { child->name = tks; next(); } else { printf("error3!\n"); exit(-1); }
			if(!strcmp(tks, ":")) next(); else { printf("error4!\n"); exit(-1); }
			switch(tki) {
				case CJSON_NUL: case CJSON_FALSE: case CJSON_TRUE: child->type = tki; break;
				case CJSON_NUM: child->type = tki; child->num = atof(tks); break;
				case CJSON_STR: child->type = tki; child->str = tks; break;
				case CJSON_ARR: parseArr(child);
				case CJSON_OBJ: parseObj(child);
				default: assert(0);
			}
			cjson_addNodeToObj(node, child->name, child);
			next();
			if(!strcmp(tks, "}")) break;
			else if(!strcmp(tks, ",")) next();
			else { printf("error5!\n"); exit(-1); }
		}
	}
}

static void parseArr(cjson_Node *node) {
	assert(!strcmp(tks, "["));
	next();
	node->type = CJSON_ARR;
	if(strcmp(tks, "]")) {
		node->child = newNode();
		node = node->child;
		while(1) {
			switch(tki) {
				case CJSON_NUL: case CJSON_FALSE: case CJSON_TRUE: node->type = tki; break;
				case CJSON_NUM: node->type = tki; node->num = atof(tks); break;
				case CJSON_STR: node->type = tki; node->str = tks; break;
				case CJSON_ARR: parseArr(node);
				case CJSON_OBJ: parseObj(node);
				default: assert(0);
			}
			next();
			if(!strcmp(tks, "]")) break;
			else if(!strcmp(tks, ",")) { node->next = newNode(); node = node->next; next(); }
			else { printf("error6!\n"); exit(-1); }
		}
	}
}

void cjson_addNodeToArr(cjson_Node* arr, cjson_Node* node) {
	arr->child->next = node;
}

void cjson_addNodeToObj(cjson_Node* obj, char *name, cjson_Node* node) {
	assert(obj != NULL);
	assert(obj->type == CJSON_OBJ);
	assert(node != NULL);
	if(obj->child == NULL) { obj->child = node; return; }
	cjson_Node *i = obj;
	cjson_Node *j = obj->child;
	while(j != NULL) {
		if(!strcmp(j->name, node->name)) {
			cjson_delete(j);
			j = node;
			return;
		}
		i = j;
		j = j->next;
	}
	i->next = node;
}

cjson_Node* cjson_parse(char *json) {
	p = json;
	cjson_Node *node = newNode();
	if(!strcmp(tks, "{")) parseObj(node);
	else if(!strcmp(tks, "[")) parseArr(node);
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

static void printFloat(float f, char *s) {
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
	sprintf(s, "%s", buf);
}

static void printIndent(int indent, char *s) {
	for(int i = 0; i < indent * INDENT; i++) sprintf(s, " ");
}

static void printEndl(cjson_Node *node, char *s) {
	sprintf(s, node->next != NULL ? ",\n" : "\n");
}

static void print(cjson_Node *node, char *s, int indent) {
	printIndent(indent, s);
	if(strcmp(node->name, "")) sprintf(s, "\"%s\" : ", node->name);
	switch(node->type) {
	case CJSON_NUL: sprintf(s, "null"); break;
	case CJSON_FALSE: sprintf(s, "false"); break;
	case CJSON_TRUE: sprintf(s, "true"); break;
	case CJSON_NUM: printFloat(node->num, s); break;
	case CJSON_STR: sprintf(s, "\"%s\"", node->str); break;
	case CJSON_OBJ: sprintf(s, "{\n");
		for(cjson_Node *i = node->child; i != NULL; i = i->next) print(i, s, indent + 1);
		printIndent(indent, s); sprintf(s, "}"); break;
	case CJSON_ARR: sprintf(s, "[\n");
		for(cjson_Node *i = node->child; i != NULL; i = i->next) print(i, s, indent + 1);
		printIndent(indent, s); sprintf(s, "]"); break;
	default: printf("error12!\n"); exit(-1);
	}
	printEndl(node, s);
}

char* cjson_print(cjson_Node *node) {
	char *strbuf = malloc(MAXSIZE * sizeof(char));
	print(node, strbuf, 0);
	return strbuf;
}
