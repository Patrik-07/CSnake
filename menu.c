#include "init.h"
#include "menu.h"

/** Menüt működtető függvények **/
/* Visszatér annak a gombnak az indexével, amin az egér van. */
static int button_select(Menu *menu, Sint16 x, Sint16 y) {
    for (int i = 0; i < menu->n; i++)
        if ((menu->button[i].rect.x < x && x < menu->button[i].rect.x+menu->button[i].rect.w) && (menu->button[i].rect.y < y && y < menu->button[i].rect.y+menu->button[i].rect.h)) {
                menu->button[i].active = true;
                return i;
        }
        else
            menu->button[i].active = false;
    return -1;
}

/* Visszatér egy Title típusú változóval, ami megadott paraméterekkel van inicializálva. */
Title title_init(SDL_Rect trect, SDL_Color tc, const char *tname, const char *name) {
    Title new_title;
    TTF_Font *tfont;
    int l;
    if (strcmp(T_FONT, name) == 0)
        l = trect.w/strlen(tname)*4.2;
    else
        l = 32;
    font_open(&tfont, name, l);
    new_title.font = tfont;
    new_title.rect = trect;
    strcpy(new_title.name, tname);
    new_title.c = tc;

    return new_title;
}

/* Inicializálja a paraméterként megadott gombokat, tetszõleges mérettel és pozícióval. */
static Button button_init(SDL_Rect brect, SDL_Color *bc, const char *bname) {
    Button new_button;
    new_button.rect = brect;
    new_button.c = bc[0];
    new_button.title = title_init(brect, bc[1], bname, B_FONT);
    new_button.active = false;

    return new_button;
}

/* Visszatér egy Menu típusú változóval, a megadott paraméterekkel inicializálva. */
Menu menu_init(const char *tname, SDL_Color tc, SDL_Rect brect, const char **bname, SDL_Color *bc, int n, SDL_Color bg) {
    Menu new_menu;
    /* Felírat inicializálása */
    new_menu.title = title_init((SDL_Rect){0, 0, M_WIDTH, M_HEIGHT/4}, tc, tname, T_FONT);
    /* Új gombok foglalálsa */
    new_menu.button = (Button*) malloc(n*sizeof(Button));
    if (new_menu.button == NULL)
        exit(1);
    /* Gombok inicializálása */
    new_menu.n = n;
    for (int i = 0; i < new_menu.n; i++) {
        SDL_Rect rect = {brect.x, brect.y+((brect.h)/(new_menu.n))*i, brect.w, (2*brect.h)/(3*new_menu.n)};
        new_menu.button[i] = button_init(rect, bc, bname[i]);
    }
    /* A menu fut */
    new_menu.run = true;
    /* Háttérszín beállítása */
    new_menu.bg = bg;

    return new_menu;
}

/* Szöveget rajzol a képernyõre. */
void title_draw(SDL_Renderer *renderer, Title title) {
    SDL_Surface *text = TTF_RenderUTF8_Blended(title.font, title.name, title.c);
    SDL_Texture *text_t = SDL_CreateTextureFromSurface(renderer, text);

    SDL_Rect dest = {title.rect.x+(title.rect.w-text->w)/2, title.rect.y+(title.rect.h-text->h)/2, text->w, text->h};
    SDL_RenderCopy(renderer, text_t, NULL, &dest);
    SDL_RenderPresent(renderer);

    SDL_FreeSurface(text);
    SDL_DestroyTexture(text_t);
}

/* Gombot rajzol a képernyõre. */
static void button_draw(SDL_Renderer *renderer, Button button) {
    if (button.active) {
        SDL_Color temp = button.c;
        button.c = button.title.c;
        button.title.c = temp;
    }
    boxRGBA(renderer, button.rect.x, button.rect.y, button.rect.x+button.rect.w, button.rect.y+button.rect.h, button.c.r, button.c.g, button.c.b, button.c.a);
    title_draw(renderer, button.title);
    SDL_RenderPresent(renderer);
}

/* Kirajzolja a menüt */
void menu_draw(SDL_Renderer *renderer, Menu *menu) {
    SDL_Rect m = {0, 0, M_WIDTH, M_HEIGHT};
    set_background_color(renderer, m, menu->bg);
    title_draw(renderer, menu->title);
    for (int i = 0; i < menu->n; i++)
        button_draw(renderer, menu->button[i]);
}

/* Szövegeket rajzol a képernyőre. */
void text_draw(SDL_Renderer *renderer, SDL_Rect rect, SDL_Color tc, const char **text, int n) {
    SDL_Rect trect;
    for (int i = 0; i < n; i++) {
        trect = (SDL_Rect) {rect.x, rect.y+(rect.h/4)*i, rect.w, rect.h/4};
        Title txt = title_init(trect, tc, text[i], B_FONT);
        title_draw(renderer, txt);
        TTF_CloseFont(txt.font);
    }
}

/* Ez az event vezérli a menüt, visszatér egy egész számmal ami a kiválasztott esemény. */
int menu_event(SDL_Renderer *renderer, Menu *menu) {
    SDL_Event event;
    SDL_WaitEvent(&event);
    switch (event.type) {
        case SDL_MOUSEMOTION:
            button_select(menu, event.motion.x, event.motion.y);
            for (int i = 0; i < menu->n; i++)
                button_draw(renderer, menu->button[i]);
            return -1;
        case SDL_MOUSEBUTTONUP:
            return button_select(menu, event.button.x, event.button.y);
        case SDL_QUIT:
            return SDL_QUIT;
        default:
            return -1;
    }
}

/* Egy menü működését valósítja meg, az előző függvény visszatérési értékeként átvett paraméter szerint. */
bool menu_process(SDL_Renderer *renderer, Menu *menu, Menu *top_menu, int select) {
    if (select == menu->n-1) {
        menu->run = false;
        if (top_menu == NULL)
            return false;
        menu_draw(renderer, top_menu);
        return true;
    }
    switch (select) {
        case SDL_QUIT:
            SDL_Quit();
            menu->run = false;
            if (top_menu != NULL)
                top_menu->run = false;
            return false;
        case -1: return true;
        default: return true;
    }
}

/* Kirajzolja a dicsőség táblát. */
void draw_leaderboard(SDL_Renderer *renderer, Position position, Best *best) {
    SDL_Surface *text;
    SDL_Texture *text_t;
    SDL_Rect destination;
    SDL_Color white = {255, 255, 255};
    char str[100];
    char score[3+1];
    TTF_Font *font;
    font_open(&font, B_FONT, 32);
    for (int i = 0; i < best->n; i++) {
        itoa(i+1, str, 10);
        strcat(str, ". ");
        strcat(str, best->player[i].name);
        text = TTF_RenderUTF8_Blended(font, str, white);
        while (position.x + text->w < M_WIDTH*4/5) {
            strcat(str, ".");
            text = TTF_RenderUTF8_Blended(font, str, white);
        }
        itoa(best->player[i].score, score, 10);
        strcat(str, score);
        text = TTF_RenderUTF8_Blended(font, str, white);
        text_t = SDL_CreateTextureFromSurface(renderer, text);
        destination = (SDL_Rect) {position.x, position.y+(i*35), text->w, text->h};
        SDL_RenderCopy(renderer, text_t, NULL, &destination);
    }
    TTF_CloseFont(font);
    SDL_RenderPresent(renderer);
}

/* Felszabadítja a menüt. */
void free_menu(Menu *menu) {
    TTF_CloseFont(menu->title.font);
    for (int i = 0; i < menu->n; i++)
        TTF_CloseFont(menu->button[i].title.font);
    free(menu->button);
}
