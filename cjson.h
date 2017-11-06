#ifndef _CJSON_H_
#define _CJSON_H_

typedef enum {
	CJSON_NUL, CJSON_FALSE, CJSON_TRUE, CJSON_NUM, CJSON_STR, CJSON_ARR, CJSON_OBJ
} cjson_Type;

typedef struct cjson_Node {
	char *name;
	cjson_Type type;
	struct cjson_Node *next;
	union {
		float num;
		char *str;
		struct cjson_Node *child;
	};
} cjson_Node;

cjson_Node* cjson_parse(char *json);
void cjson_print(cjson_Node *node);
void cjson_delete(cjson_Node *node);
void cjson_addNodeToArr(cjson_Node* arr, cjson_Node* node);
void cjson_addNodeToObj(cjson_Node* obj, char *name, cjson_Node* node);

#endif