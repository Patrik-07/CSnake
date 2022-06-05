/* My_headers */
#include "menu.h"
#include "init.h"
#include "snake.h"
/* SDL */
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_ttf.h>
/* Standard C */
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
/* InfoC */
#include "debugmalloc.h"

int main(int argc, char *argv[]) {
    /* Deklarációk */
    SDL_Window *window;
    SDL_Renderer *renderer;

    /* Inicializálások */
    TTF_Init();
    srand(time(NULL));

    /** A menü adatok **/
    /* Gomb aktív, inaktív színe és a Menü felírat színe, háttérszínek */
    SDL_Color bc[2] = { {81, 119, 231, 255}, {255, 255, 255, 255} };
    SDL_Color tc = {0, 60, 0, 255};
    SDL_Color bg = {126, 200, 80, 255};
    SDL_Color white = {255, 255, 255, 255};

    /** A játék adatok  **/
    /* Pálya színei */
    SDL_Color field = {255, 240, 228, 255};
    SDL_Color wall = {54, 37, 20, 255};
    /* Legjobb játékososk */
    Best best;
    read_scores("bestplayers.txt", &best);

    /*** Program ***/
    bool run = true;
    while (run) {
        Gamemode gamemode = None;
        /** Menurendszer **/
        sdl_init(&window, &renderer, M_WIDTH, M_HEIGHT, "Menü");
        /* Top menu */
        int select;
        const char *top_menu_b[4] = {"Játék", "Dicsőséglista", "Útmutató", "Kilépés"};
        Menu top_menu = menu_init("Kígyó Játék", tc, (SDL_Rect){50, 180, 260, 440}, top_menu_b, bc, 4, bg);
        menu_draw(renderer, &top_menu);
        /* Amíg fut a top_menu */
        while (top_menu.run) {
            select = menu_event(renderer, &top_menu);
            run = menu_process(renderer, &top_menu, NULL, select);
            switch (select) {
                /** Start menu **/
                case 0: {
                    const char *start_menu_b[3] = {"Egyjátékos", "Kétjátékos", "Vissza"};
                    Menu start_menu = menu_init("Válasz játékmódot!", tc, (SDL_Rect){70, 180, 220, 450}, start_menu_b, bc, 3, bg);
                    menu_draw(renderer, &start_menu);
                    /* Amíg fut a start_menu */
                    while (start_menu.run) {
                        select = menu_event(renderer, &start_menu);
                        run = menu_process(renderer, &start_menu, &top_menu, select);
                        switch (select) {
                            case Single:
                                gamemode = Single;
                                top_menu.run = start_menu.run = false;
                                break;
                            case Multi:
                                gamemode = Multi;
                                top_menu.run = start_menu.run = false;
                                break;
                        }
                    }
                    free_menu(&start_menu);
                    break;
                }
                /** LeaderBoard menu **/
                case 1: {
                    const char *lb_menu_b[1] = {"Vissza"};
                    Menu lb_menu = menu_init("Legjobb játékosok", tc, (SDL_Rect){100, 520, 160, 100}, lb_menu_b, bc, 1, bg);
                    menu_draw(renderer, &lb_menu);
                    /* Dicsőséglista kirajzolása */
                    draw_leaderboard(renderer, (Position) {20, M_WIDTH/3+25}, &best);
                    /* Amíg fut a lb_menu */
                    while (lb_menu.run) {
                        select = menu_event(renderer, &lb_menu);
                        run = menu_process(renderer, &lb_menu, &top_menu, select);
                    }
                    free_menu(&lb_menu);
                    break;
                }
                /** Útmutató menu **/
                case 2: {
                    const char *guide_menu_b[1] = {"Vissza"};
                    Menu guide_menu = menu_init("Útmutató a játékhoz", tc, (SDL_Rect){100, 520, 160, 100}, guide_menu_b, bc, 1, bg);
                    menu_draw(renderer, &guide_menu);
                    /* Szabályok megadása és kirajzolása a képernyõre. */
                    const char *rname[5] = {"Ne ütközz neki a falnak.", "Ne ütközz az ellenfélnek.", "Irányítás:", "Piros kígyó: Nyilak", "Kék kígyó: W,A,S,D"};
                    text_draw(renderer, (SDL_Rect) {20, 120, 340, 300}, white, rname, 5);
                    /* Amíg fut a guide_menu */
                    while (guide_menu.run) {
                        select = menu_event(renderer, &guide_menu);
                        run = menu_process(renderer, &guide_menu, &top_menu, select);
                    }
                    free_menu(&guide_menu);
                    break;
                }
                /** Kilépés **/
                case 3: SDL_Quit(); top_menu.run = false; run = false; break;
            }
        }
        free_menu(&top_menu);
        destroy(renderer, window);
        /** Menurendszer vége **/

        /** Játék **/
        if (gamemode != None) {
            /** Inicializálások **/
            Position start_pos1 = {(G_WIDTH/PX)/2, (G_HEIGHT/PX)*1/3};
            SDL_Color chead1 = {255, 0, 0, 255};
            SDL_Keycode control1[4] = {SDLK_UP, SDLK_DOWN, SDLK_LEFT, SDLK_RIGHT};
            Snake *snakeRED = snake_init(start_pos1, control1, Right, chead1);
            /* 2. Kígyó lérehozása. */
            Snake *snakeBLUE = NULL;
            if (gamemode == Multi) {
                Position start_pos2 = {(G_WIDTH/PX)/2, (G_HEIGHT/PX)*2/3};
                SDL_Color chead2 = {0, 0, 255, 255};
                SDL_Keycode control2[4] = {SDLK_w, SDLK_s, SDLK_a, SDLK_d};
                snakeBLUE = snake_init(start_pos2, control2, Left, chead2);
            }
            /* Ablak és idõzítõ. */
            sdl_init(&window, &renderer, G_WIDTH, G_HEIGHT, "Játék");
            SDL_TimerID id = SDL_AddTimer(125, timer, NULL);
            /* Kezdõ képernyõ */
            game_start(renderer, snakeRED, snakeBLUE, "Nyilak", "W,A,S,D", wall, field);

            /** A játék motorja **/
            /* Amíg élnek a kígyók */
            bool game = true;
            bool ok1 = true;
            bool ok2 = true;
            while (game) {
                SDL_Event event;
                SDL_WaitEvent(&event);
                switch (event.type) {
                    case SDL_KEYDOWN:
                        if (ok1)
                            ok1 = snake_direction(snakeRED, event.key.keysym.sym);
                        if (snakeBLUE != NULL)
                            if (ok2)
                                ok2 = snake_direction(snakeBLUE, event.key.keysym.sym);
                        break;
                    case SDL_USEREVENT:
                        game = snake_event(renderer, snakeRED, snakeBLUE, wall, field);
                        ok1 = ok2 = true;
                        break;
                    case SDL_QUIT:
                        game = false;
                        break;
                    default: break;
                }
            }
            if (gamemode == Multi)
                multi_end(renderer, snakeRED, snakeBLUE, wall, field);
            if (gamemode == Single) {
                if (score_process(snakeRED, &best)) {
                    destroy(renderer, window);
                    sdl_init(&window, &renderer, M_WIDTH, M_HEIGHT, "Snake menu");
                    const char *choose[2] = {"Igen", "Nem"};
                    Menu save_menu = menu_init(" Pont elmentése ", tc, (SDL_Rect){50, 300, 260, 150}, choose, bc, 2, bg);
                    menu_draw(renderer, &save_menu);
                    while (save_menu.run) {
                        select = menu_event(renderer, &save_menu);
                        menu_process(renderer, &save_menu, NULL, select);
                        switch (select) {
                            case 0: {
                                Player new_best;
                                new_best.score = snakeRED->score;
                                SDL_Rect r = {80, M_HEIGHT/4, M_WIDTH-160, 40};
                                SDL_Color white = {255, 255, 255, 255};
                                set_background_color(renderer, (SDL_Rect){0, 0, M_WIDTH, M_HEIGHT}, (SDL_Color){0, 0, 0, 100});
                                input_text(new_best.name, 11, r, white, tc, renderer);
                                new_bestplayers(new_best, &best);
                                save_menu.run = false;
                                break;
                            }
                        }
                    }
                    free_menu(&save_menu);
                }
            }
            SDL_RemoveTimer(id);
            snake_delete(snakeRED);
            snake_delete(snakeBLUE);
            destroy(renderer, window);
        }
        /** Játék vége **/
    }
    /*** Program vége ***/
    write_score("bestplayers.txt", &best);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    return 0;
}
