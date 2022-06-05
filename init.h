#ifndef INIT_H_INCLUDED
#define INIT_H_INCLUDED

#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define B_FONT "LiberationSerif-Regular.ttf"
#define T_FONT "Title.ttf"

void window_init(SDL_Window **pwindow, const int w, const int h, const char *title);
void renderer_init(SDL_Renderer **prenderer, SDL_Window *window);
void sdl_init(SDL_Window **pwindow, SDL_Renderer **prenderer, const int w, const int h, const char *title);
void font_open(TTF_Font **pfont, const char *style, int l);
void set_background_color(SDL_Renderer *renderer, SDL_Rect r, SDL_Color c);
void destroy(SDL_Renderer *renderer, SDL_Window *window);
Uint32 timer(Uint32 ms, void *param);
bool input_text(char *dest, size_t hossz, SDL_Rect teglalap, SDL_Color hatter, SDL_Color szoveg, SDL_Renderer *renderer);

#endif // INIT_H_INCLUDED
