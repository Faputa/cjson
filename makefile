all:
	@gcc cjson.c test.c -o cjson -Wall
	@cjson test.json