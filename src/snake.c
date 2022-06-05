#include "init.h"
#include "menu.h"
#include "snake.h"

/* Az rgb színeket állítja elõ */
static void snake_rgb(Snake *snake) {
    SDL_Color color;
    Part *body = snake->tail->prev;
    if(body->c.r > 0 && body->c.b == 0){
        color.r = body->c.r-15;
        color.g = body->c.g+15;
        color.b = 0;
    }
    if(body->c.g > 0 && body->c.r == 0){
        color.g = body->c.g-15;
        color.b = body->c.b+15;
        color.r = 0;
    }
    if(body->c.b > 0 && body->c.g == 0){
        color.r = body->c.r+15;
        color.b = body->c.b-15;
        color.g = 0;
    }
    color.a = 255;
    snake->tail->c = color;
}

/* Megnöveli a kígyó méretét egyel, hozzáfûz egy új részt. */
static void snake_grow(Snake *snake) {
    Part *new_body = (Part*) malloc(sizeof(Part));
    if (new_body == NULL) {
        SDL_Quit();
        exit(1);
    }
    if (snake->head != NULL) {
        snake->tail->next = new_body;
        new_body->prev = snake->tail;
        snake->tail = new_body;
        snake_rgb(snake);
    }
    else {
        snake->head = new_body;
        snake->tail = new_body;
        new_body->prev = NULL;
    }
    new_body->next = NULL;
    snake->score++;
}

/* Visszatér egy Snake típusú pointerrel, ami az új kígyó fejére mutat. */
Snake *snake_init(Position position, SDL_Keycode *control, Direction direction, SDL_Color chead) {
    Snake *new_snake = (Snake*) malloc(sizeof(Snake));
    new_snake->live = true;
    new_snake->score = 0;
    /* Fej */
    new_snake->head = NULL;
    snake_grow(new_snake);
    new_snake->head->c = chead;
    /* Törzs és farok */
    snake_grow(new_snake);
    snake_grow(new_snake);
    /* Kezdõ pozíció beállítása */
    Position d;
    switch (direction) {
        case Up:    d = (Position) {0, 1};  break;
        case Down:  d = (Position) {0, -1};   break;
        case Left:  d = (Position) {1, 0};   break;
        case Right: d = (Position) {-1, 0};   break;
        default: exit(1); break;
    }
    int i = 0;
    for (Part *iter = new_snake->head; iter != NULL; iter = iter->next) {
        iter->pos = (Position) {position.x + i*d.x, position.y + i*d.y};
        i++;
    }
    for (int i = 0; i < 4; i++)
        new_snake->control[i] = control[i];
    new_snake->direction = direction;
    new_snake->score = new_snake->score-3;
    return new_snake;
}

/* Beállítja az új irányt amerre a kígyó mozog. */
bool snake_direction(Snake *snake, SDL_Keycode keycode) {
    bool correct = false;
    Direction new_d;
    Direction d = snake->direction;
    for (int i = 0; i < 4; i++) {
        if (keycode == snake->control[i]) {
            new_d = i;
            correct = true;
        }
    }
    if (correct)
        if ((d == Up && new_d != Down) || (d == Down && new_d != Up) || (d == Left && new_d != Right) || (d == Right && new_d != Left)) {
            snake->direction = new_d;
            return false;
        }
    return true;
}

/* Beállítja a kígyó pozícióját a képernyõre való kirajzoláshoz. */
static void snake_move(Snake *snake) {
    for (Part *iter = snake->tail; iter != snake->head; iter = iter->prev)
        iter->pos = iter->prev->pos;
    Position d;
    switch (snake->direction) {
        case Up: d = (Position) {0, -1}; break;
        case Down: d = (Position) {0, 1}; break;
        case Right: d = (Position) {1, 0}; break;
        case Left: d = (Position) {-1, 0}; break;
        default: return;
    }
    snake->head->pos = (Position) {snake->head->pos.x+d.x, snake->head->pos.y+d.y};
}

static void set_game_bg(SDL_Renderer *renderer, SDL_Color wall, SDL_Color field) {
    SDL_Rect r = {0, 0, G_WIDTH, G_HEIGHT};
    set_background_color(renderer, r, wall);
    r = (SDL_Rect) {PX, PX, G_WIDTH-PX, G_HEIGHT-PX};
    set_background_color(renderer, r, field);
}

/* Kirajzolja a kígyót a képernyõre */
static void snake_draw(SDL_Renderer *renderer, Snake *snake, SDL_Color wall, SDL_Color field) {
    Part *head = snake->head;
    boxRGBA(renderer, (head->pos.x+1)*PX, (head->pos.y+1)*PX, (head->pos.x+2)*PX, (head->pos.y+2)*PX,
            head->c.r, head->c.g, head->c.b, head->c.a);
    for (Part *iter = snake->head->next; iter != NULL; iter = iter->next) {
        boxRGBA(renderer, (iter->pos.x+1.9)*PX, (iter->pos.y+1.9)*PX, (iter->pos.x+1.1)*PX, (iter->pos.y+1.1)*PX,
                iter->c.r, iter->c.g, iter->c.b, iter->c.a);
    }
    char str[3+1];
    SDL_Color black = {0, 0, 0, 255};
    itoa(snake->score, str, 10);
    SDL_Rect tr = {(snake->head->pos.x+2)*PX, (snake->head->pos.y)*PX, 20, 20};
    Title score = title_init(tr, black, str, B_FONT);
    title_draw(renderer, score);
}

/* Megvizsgálja, hogy melyik kígyó halt meg. */
static void snake_alive(Snake *snake1, Snake *snake2) {
    for (Part *iter1 = snake1->head->next; iter1 != NULL; iter1 = iter1->next) {
        if (snake1->head->pos.x == iter1->pos.x && snake1->head->pos.y == iter1->pos.y)
            snake1->live = false;
        if (snake2 != NULL)
            if (snake2->head->pos.x == iter1->pos.x && snake2->head->pos.y == iter1->pos.y)
                snake2->live = false;
    }
    if (snake2 != NULL) {
        for (Part *iter2 = snake2->head->next; iter2 != NULL; iter2 = iter2->next) {
            if (snake2->head->pos.x == iter2->pos.x && snake2->head->pos.y == iter2->pos.y)
                snake2->live = false;
            if (snake1->head->pos.x == iter2->pos.x && snake1->head->pos.y == iter2->pos.y)
                    snake1->live = false;
        }
        if ((0 > snake2->head->pos.x || snake2->head->pos.x > G_WIDTH/PX-3) || (0 > snake2->head->pos.y || snake2->head->pos.y > G_HEIGHT/PX-3))
            snake2->live = false;
        if ((snake1->head->pos.x == snake2->head->pos.x) && (snake1->head->pos.y == snake2->head->pos.y)) {
            snake1->live = false;
            snake2->live = false;
        }
    }
    if ((0 > snake1->head->pos.x || snake1->head->pos.x > G_WIDTH/PX-3) || (0 > snake1->head->pos.y || snake1->head->pos.y > G_HEIGHT/PX-3))
        snake1->live = false;
}

/* Megmondja, hogy a kígyó megette-e az ételt. Igazzal tér vissza, ha igen. Hammissal, ha nem.*/
static bool snake_eat(Snake *snake, Position food_pos) {
    if (snake->head->pos.x == food_pos.x && snake->head->pos.y == food_pos.y) {
        return true;
    }
    return false;
}

/* Kirajzolja az ételt a képernyõre. */
static void draw_food(SDL_Renderer *renderer, Position food_pos) {
    boxRGBA(renderer, (food_pos.x+1)*PX, (food_pos.y+1)*PX, (food_pos.x+2)*PX, (food_pos.y+2)*PX, 244, 164, 96, 255);
}

/* Kiírja a képernyõre, hogy melyik gombokkal lehet mozgatni a kígyót. */
static void snake_info(SDL_Renderer *renderer, Snake* snake, const char *name) {
    SDL_Rect tpos = {(snake->head->pos.x+1)*PX, (snake->head->pos.y+2)*PX, 32, 32};
    Title snake_t = title_init(tpos, snake->head->c, name, B_FONT);
    title_draw(renderer, snake_t);
}

/* Egy gomb lenyomására vár. */
static void wait() {
    bool start = false;
    while (!start) {
        SDL_Event wait;
        SDL_WaitEvent(&wait);
        switch (wait.type) {
            case SDL_KEYDOWN: start = true; break;
            case SDL_QUIT: exit(1);
            default: break;
        }
    }
}

void game_start(SDL_Renderer *renderer, Snake *snake1, Snake *snake2, const char *s1, const char *s2, SDL_Color wall, SDL_Color field) {
    set_game_bg(renderer, wall, field);
    snake_draw(renderer, snake1, wall, field);
    if (snake2 != NULL)
        snake_draw(renderer, snake2, wall, field);
    SDL_Rect r = {0, 0, G_WIDTH, G_HEIGHT};
    set_background_color(renderer, r, (SDL_Color){0, 0, 0, 100});
    const char *title[1] = {"Nyomj meg egy gombot a kezdéshez!"};
    SDL_Color tc = {0, 60, 0, 255};
    text_draw(renderer, (SDL_Rect){0, 0, G_WIDTH, G_HEIGHT}, tc, title, 1);
    snake_info(renderer, snake1, s1);
    if (snake2 != NULL) {
        snake_info(renderer, snake2, s2);
    }
    wait();
    set_game_bg(renderer, wall, field);
}

/* Kiírja a végeredményeket. */
static void snake_win(SDL_Renderer *renderer, Snake *snake1, Snake *snake2) {
    char str1[3+1];
    char str2[3+1];
    itoa(snake1->score, str1, 10);
    itoa(snake2->score, str2, 10);
    SDL_Rect r1 = { G_WIDTH/2, G_HEIGHT/3, 5, 5 };
    SDL_Rect r2 = { G_WIDTH/2, G_HEIGHT/3+50, 5, 5 };
    Title t1 = title_init(r1, snake1->head->c, str1, B_FONT);
    Title t2 = title_init(r2, snake2->head->c, str2, B_FONT);
    title_draw(renderer, t1);
    title_draw(renderer, t2);
    SDL_Color white = {255, 255, 255, 255};
    const char *redw[1] = {"Piros nyert!"};
    const char *bluew[1] = {"Kék nyert!"};
    const char *draw[1] = {"Döntetlen!"};
    if (snake1->live && !snake2->live)
        text_draw(renderer, (SDL_Rect){0, 0, G_WIDTH, G_HEIGHT}, white, redw, 1);
    if (!snake1->live && snake2->live)
        text_draw(renderer, (SDL_Rect){0, 0, G_WIDTH, G_HEIGHT}, white, bluew, 1);
    if (!snake1->live && !snake2->live) {
        if (snake1->score == snake2->score)
            text_draw(renderer, (SDL_Rect){0, 0, G_WIDTH, G_HEIGHT}, white, draw, 1);
        if (snake1->score > snake2->score)
            text_draw(renderer, (SDL_Rect){0, 0, G_WIDTH, G_HEIGHT}, white, redw, 1);
        if (snake1->score < snake2->score)
            text_draw(renderer, (SDL_Rect){0, 0, G_WIDTH, G_HEIGHT}, white, bluew, 1);
    }
}

void multi_end(SDL_Renderer *renderer, Snake *snake1, Snake *snake2, SDL_Color wall, SDL_Color field) {
    SDL_Rect r = {0, 0, G_WIDTH, G_HEIGHT};
    snake_draw(renderer, snake1, wall, field);
    snake_draw(renderer, snake2, wall, field);
    set_background_color(renderer, r, (SDL_Color){0, 0, 0, 100});
    snake_win(renderer, snake1, snake2);
    const char *title_m[1] = {"Nyomj meg egy gombot, hogy visszalépj a menübe!"};
    SDL_Color tc = {0, 60, 0, 255};
    text_draw(renderer, (SDL_Rect){0, G_HEIGHT*3/4, G_WIDTH, G_HEIGHT}, tc, title_m, 1);
    wait();
}

/* Törli a kígyót a memóriából. */
void snake_delete(Snake *snake) {
    if (snake != NULL) {
        Part *iter = snake->head;
        while (iter != NULL) {
            Part *next = iter->next;
            free(iter);
            iter = next;
        }
    }
    free(snake);
}

/* Visszatér az új étel pozíciójával. */
static Position new_food(Snake *snake1, Snake *snake2) {
    srand(time(NULL));
    int x = (G_WIDTH/PX-2);
    int y = (G_HEIGHT/PX-2);
    int amount = x*y;
    bool free[x][y];
    if (snake2 != NULL) {
        for (int i = 0; i < x; i++)
            for (int j = 0; j < y; j++) {
                bool ok = true;
                for (Part *iter1 = snake1->head; iter1 != NULL; iter1 = iter1->next)
                    if ((i == iter1->pos.x) && (j == iter1->pos.y)) {
                        ok = false;
                        break;
                    }
                for (Part *iter2 = snake2->head; iter2 != NULL; iter2 = iter2->next)
                    if ((i == iter2->pos.x) && (j == iter2->pos.y)) {
                        ok = false;
                        break;
                    }
                if (!ok) {
                    free[i][j] = false;
                    amount--;
                }
                free[i][j] = true;
            }
    }
    else {
        for (int i = 0; i < x; i++)
            for (int j = 0; j < y; j++)
                for (Part *iter1 = snake1->head; iter1 != NULL; iter1 = iter1->next) {
                    if ((i == iter1->pos.x) && (j == iter1->pos.y)) {
                        free[i][j] = false;
                        amount--;
                        break;
                    }
                    free[i][j] = true;
                }
    }
    Position food[amount];
    int k = 0;
    for (int i = 0; i < x; i++)
        for (int j = 0; j < y; j++)
            if (free[i][j] == true)
                food[k++] = (Position) { i, j };
    srand(time(0));
    int random_number = rand() % (k-1);
    return food[random_number];
}

/* A következõ pozícióba rajzolja ki a kígyót a képernyõre. */
bool snake_event(SDL_Renderer *renderer, Snake *snake1, Snake *snake2, SDL_Color wall, SDL_Color field) {
    static Position food = {(G_WIDTH/PX/2)-1, (G_HEIGHT/PX/2)-1};
    set_game_bg(renderer, wall, field);
    draw_food(renderer, food);
    if (snake2 != NULL) {
        snake_move(snake1);
        snake_move(snake2);
        snake_alive(snake1, snake2);
        if (snake1->live && snake2->live) {
            snake_draw(renderer, snake1, wall, field);
            snake_draw(renderer, snake2, wall, field);
        }
        else
            return false;
        if (snake_eat(snake1, food)) {
            snake_grow(snake1);
            food = new_food(snake1, snake2);
        }
        if (snake_eat(snake2, food)) {
            snake_grow(snake2);
            food = new_food(snake1, snake2);
        }
    }
    else {
        snake_move(snake1);
        snake_alive(snake1, snake2);
        if (snake1->live)
            snake_draw(renderer, snake1, wall, field);
        else
            return false;
        if (snake_eat(snake1, food)) {
            snake_grow(snake1);
            food = new_food(snake1, snake2);
        }
    }
    SDL_RenderPresent(renderer);
    return true;
}

/* Megnézi, hogy fent felkerülhet-e a listára egy játékos */
bool score_process(Snake *snake, Best *best) {
    for (int i = 0; i < 10; i++)
        if (snake->score > best->player[i].score)
            return true;
    return false;
}

/* Beolvassa a legjobb játékosoakt */
void read_scores(const char *filename, Best *best) {
    FILE *fptr = fopen(filename, "rt");
    if (fptr == NULL)
        exit(1);
    for (int i = 0; i < 10; i++)
        best->player[i].score = 0;
    int i = 0;
    while (fscanf(fptr, "%s\t%d", best->player[i].name, &(best->player[i].score)) == 2)
        i++;
    best->n = i;
    fclose(fptr);
}

/* Az új játkos felkerül a listára a pontszáma alapján */
void new_bestplayers(Player new_best, Best *best) {
    int i;
    if (best->n == 10) {
        strcpy(best->player[9].name, new_best.name);
        best->player[9].score = new_best.score;
        i = best->n-2;
    }
    else {
        int iter = best->n;
        strcpy(best->player[iter].name, new_best.name);
        best->player[iter].score = new_best.score;
        i = best->n;
        if (best->n == 9)
            i = best->n-1;
        best->n++;
    }
    while (i >= 0 && (new_best.score >= best->player[i].score)) {
        char name[20];
        strcpy(name, best->player[i].name);
        int score = best->player[i].score;
        strcpy(best->player[i].name, new_best.name);
        best->player[i].score = new_best.score;
        strcpy(best->player[i+1].name, name);
        best->player[i+1].score = score;
        i--;
    }
}

/* Fáljba írja a legjobb játékosokat */
void write_score(char *filename, Best *best) {
    FILE *fptr = fopen(filename, "wt");
    if (fptr == NULL)
        exit(1);
    int i = 0;
    while (i != best->n) {
        fprintf(fptr, "%s\t%d\n", best->player[i].name, best->player[i].score);
        i++;
    }
}

/* Idõzítõ a kígyó mozgatásához. */
Uint32 timer(Uint32 ms, void *param) {
    SDL_Event ev;
    ev.type = SDL_USEREVENT;
    SDL_PushEvent(&ev);
    return ms;
}
