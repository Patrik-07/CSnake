#ifndef MENU_H_INCLUDED
#define MENU_H_INCLUDED

#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "snake.h"

#define B_FONT "LiberationSerif-Regular.ttf"
#define T_FONT "Title.ttf"

#define M_WIDTH   360
#define M_HEIGHT  640

/* A menurendszer struktúrális felépítése */
typedef enum Menu_name {Top, Start, Lb, Guide} Menu_name;
typedef enum Gamemode {Single, Multi, None} Gamemode;
/* Felírat*/
typedef struct Title {
    TTF_Font *font;
    SDL_Rect rect;
    SDL_Color c;
    char name[50+1];
} Title;

/* Gomb */
typedef struct Button {
    SDL_Rect rect;
    Title title;
    SDL_Color c;
    bool active;
} Button;

/* Menu */
typedef struct Menu {
    Title title;
    Button *button;
    SDL_Color bg;
    int n;
    bool run;
} Menu;

/* Menut vezérlõ függvények */
Menu menu_init(const char *tname, SDL_Color tc, SDL_Rect brect, const char **bname, SDL_Color *bc, int n, SDL_Color bg);
Title title_init(SDL_Rect trect, SDL_Color tc, const char *tname, const char *name);
void title_draw(SDL_Renderer *renderer, Title title);
void menu_draw(SDL_Renderer *renderer, Menu *menu);
int menu_event(SDL_Renderer *renderer, Menu *menu);
bool menu_process(SDL_Renderer *renderer, Menu *menu, Menu *top_menu, int select);
void free_menu(Menu *menu);

/* Szövegeket rajzol a képernyõre */
void text_draw(SDL_Renderer *renderer, SDL_Rect rect, SDL_Color tc, const char **text, int n);

/* Dicsõségtábla rajzolja ki. */
void draw_leaderboard(SDL_Renderer *renderer, Position position, Best *best);

#endif // MENU_H_INCLUDED
