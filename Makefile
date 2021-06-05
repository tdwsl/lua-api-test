LUA_DIR = ~/lua-5.4.3/src

ppge: main.c
	gcc main.c -o ppge -I $(LUA_DIR) -L $(LUA_DIR) -llua -lm -lSDL2 -ldl

clean:
	rm ppge
