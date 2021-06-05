#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdbool.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#define WIDTH 80
#define HEIGHT 60

SDL_Renderer *g_renderer = NULL;
SDL_Window *g_window = NULL;
int g_scale = 8, g_pixels[WIDTH*HEIGHT][3], g_rgb[3];
char g_name[25];
const Uint8 *g_keyboard_state;

void ensure(bool cond, const char *desc)
{
	if(cond) return;
	printf("failed to ____ %s\n", desc);
	exit(1);
}

void init_sdl()
{
	ensure((SDL_Init(SDL_INIT_EVERYTHING) >= 0), "sdl");
	ensure((g_window = SDL_CreateWindow(g_name, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, g_scale*WIDTH, g_scale*HEIGHT, SDL_WINDOW_SHOWN)), "window");
	ensure((g_renderer = SDL_CreateRenderer(g_window, -1, SDL_RENDERER_SOFTWARE)), "renderer");
	SDL_SetRenderDrawColor(g_renderer, 0x00, 0x00, 0x00, 0xff);
	g_keyboard_state = SDL_GetKeyboardState(NULL);
}

void end_sdl()
{
	SDL_DestroyRenderer(g_renderer);
	g_renderer = NULL;
	SDL_DestroyWindow(g_window);
	g_window = NULL;
	SDL_Quit();
}

int l_putpixel(lua_State *l)
{
	int x = lua_tointeger(l, 1);
	int y = lua_tointeger(l, 2);
	lua_pop(l, 2);
	if(x >= WIDTH || y >= HEIGHT || x < 0 || y < 0)
		return 0;
	for(int i = 0; i < 3; i++)
		g_pixels[y*WIDTH+x][i] = g_rgb[i];
	return 0;
}

int l_clear(lua_State *l)
{
	for(int i = 0; i < WIDTH*HEIGHT; i++)
		for(int j = 0; j < 3; j++)
			g_pixels[i][j] = g_rgb[j];
	return 0;
}

int l_draw(lua_State *l)
{
	SDL_SetRenderDrawColor(g_renderer, 0x00, 0x00, 0x00, 0xff);
	SDL_RenderClear(g_renderer);
	for(int x = 0; x < WIDTH; x++)
		for(int y = 0; y < HEIGHT; y++)
		{
			int i = y*WIDTH+x;
			if(g_pixels[i][0] == 0 && g_pixels[i][1] == 0 && g_pixels[i][2] == 0)
				continue;
			SDL_SetRenderDrawColor(g_renderer, g_pixels[i][0], g_pixels[i][1], g_pixels[i][2], 0xff);
			SDL_Rect r;
			r.x = x * g_scale;
			r.y = y * g_scale;
			r.w = g_scale;
			r.h = g_scale;
			SDL_RenderFillRect(g_renderer, &r);
		}
	SDL_RenderPresent(g_renderer);
	return 0;
}

int l_setcolour(lua_State *l)
{
	for(int i = 0; i < 3; i++)
	{
		g_rgb[i] = lua_tointeger(l, i+1);
	}
	lua_pop(l, 3);
	return 0;
}

int l_wait(lua_State *l)
{
	int t = lua_tointeger(l, 1);
	lua_pop(l, 1);
	SDL_Delay(t);
	return 0;
}

void update_buttons(lua_State *l)
{
	int index[6] =
	{
		SDL_SCANCODE_UP, SDL_SCANCODE_DOWN,
		SDL_SCANCODE_LEFT, SDL_SCANCODE_RIGHT,
		SDL_SCANCODE_SPACE, SDL_SCANCODE_RETURN,
	};
	char name[6][10] =
	{
		"up", "down", "left", "right", "fire", "menu",
	};
	for(int i = 0; i < 6; i++)
	{
		lua_pushboolean(l, g_keyboard_state[index[i]]);
		char dest[30];
		sprintf(dest, "buttons.%s", name[i]);
		lua_setglobal(l, dest);
	}
}

int main(int argc, char **args)
{
	char fn[25] = "game.lua";
	if(argc > 1)
		sprintf(fn, args[1]);
	sprintf(g_name, fn);
	int i;
	for(i = 0; g_name[i] != '.'; i++);
	g_name[i] = 0;

	init_sdl();

	lua_State *lua = luaL_newstate();
	luaL_openlibs(lua);
	lua_pushcfunction(lua, l_clear);
	lua_setglobal(lua, "clear");
	lua_pushcfunction(lua, l_putpixel);
	lua_setglobal(lua, "pixel");
	lua_pushcfunction(lua, l_setcolour);
	lua_setglobal(lua, "set_colour");
	lua_pushcfunction(lua, l_draw);
	lua_setglobal(lua, "draw");
	lua_pushcfunction(lua, l_wait);
	lua_setglobal(lua, "wait");
	lua_createtable(lua, 0, 5);
	lua_setglobal(lua, "buttons");

	update_buttons(lua);
	luaL_dostring(lua, "buttons.up = false buttons.down = false buttons.left = false buttons.right = false");
	luaL_dostring(lua, "set_colour(0, 0, 0) clear()");
	luaL_dofile(lua, fn);

	lua_close(lua);
	end_sdl();
	return 0;
}
