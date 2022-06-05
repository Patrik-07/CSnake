#ifndef SNAKE_H_INCLUDED
#define SNAKE_H_INCLUDED

#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_ttf.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#define PX 20
#define G_WIDTH  800
#define G_HEIGHT 500

/* A kígyó strutúrálsi felépítése */
typedef enum Direction {Up, Down, Left, Right} Direction;
typedef struct Position {
    int x, y;
} Position;
/* A kígyó egy tagja */
typedef struct Part {
    Position pos;
    SDL_Color c;
    struct Part *next, *prev;
} Part;
/* A kígyó */
typedef struct Snake {
    Part *head;
    Part *tail;
    bool live;
    int score;
    Direction direction;
    SDL_Keycode control[4];
} Snake;

typedef struct Player{
    char name[10+1];
    int score;
} Player;

typedef struct Best {
    Player player[10];
    int n;
} Best;

Snake *snake_init(Position position, SDL_Keycode *control, Direction direction, SDL_Color chead);
bool snake_direction(Snake *snake, SDL_Keycode keycode);
bool snake_event(SDL_Renderer *renderer, Snake *snake1, Snake *snake2, SDL_Color wall, SDL_Color field);
void snake_delete(Snake *snake);
void game_start(SDL_Renderer *renderer, Snake *snake1, Snake *snake2, const char *s1, const char *s2, SDL_Color wall, SDL_Color field);
void multi_end(SDL_Renderer *renderer, Snake *snake1, Snake *snake2, SDL_Color wall, SDL_Color field);
void read_scores(const char *file, Best *best);
bool score_process(Snake *snake, Best *best);
void new_bestplayers(Player new_best, Best *best);
void write_score(char *filename, Best *best);
Uint32 timer(Uint32 ms, void *param);

#endif // SNAKE_H_INCLUDED
