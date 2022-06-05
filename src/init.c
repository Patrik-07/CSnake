#include "init.h"

/* Inicializálja az ablakot. */
void window_init(SDL_Window **pwindow, const int w, const int h, const char *title) {
    SDL_Window *window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, w, h, 0);
    if (window == NULL) {
        SDL_Log("%s", SDL_GetError());
        exit(1);
    }
    *pwindow = window;
}

/* Inicializálja a megjelenítõt. */
void renderer_init(SDL_Renderer **prenderer, SDL_Window *window) {
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    if (renderer == NULL) {
        SDL_Log("%s", SDL_GetError());
        exit(1);
    }
    *prenderer = renderer;
}

/* Az elõzõ két függvénnyel inicializál. */
void sdl_init(SDL_Window **pwindow, SDL_Renderer **prenderer, const int w, const int h, const char *title) {
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        SDL_Log("%s", SDL_GetError());
        exit(1);
    }
    window_init(pwindow, w, h, title);
    renderer_init(prenderer, *pwindow);
}

/* Fontot nyit meg */
void font_open(TTF_Font **pfont, const char *style, int l) {
    TTF_Font *font = TTF_OpenFont(style, l);
    if (font == NULL) {
        SDL_Log("%s\n", TTF_GetError());
        exit(1);
    }
    *pfont = font;
}

/* Beállítja a háttérszínt. */
void set_background_color(SDL_Renderer *renderer, SDL_Rect r, SDL_Color c) {
    boxRGBA(renderer, r.x, r.y, r.w, r.h, c.r, c.g, c.b, c.a);
}


/* Felszabadítja a renderert és az ablakot */
void destroy(SDL_Renderer *renderer, SDL_Window *window) {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
}

/* InfoC */
bool input_text(char *dest, size_t hossz, SDL_Rect teglalap, SDL_Color hatter, SDL_Color szoveg, SDL_Renderer *renderer) {
    TTF_Font *font;
    font_open(&font, B_FONT, 32);
    /* Ez tartalmazza az aktualis szerkesztest */
    char composition[SDL_TEXTEDITINGEVENT_TEXT_SIZE];
    composition[0] = '\0';
    /* Ezt a kirajzolas kozben hasznaljuk */
    char textandcomposition[hossz + SDL_TEXTEDITINGEVENT_TEXT_SIZE + 1];
    /* Max hasznalhato szelesseg */
    int maxw = teglalap.w - 2;
    int maxh = teglalap.h - 2;

    dest[0] = '\0';

    bool enter = false;
    bool kilep = false;

    SDL_StartTextInput();
    while (!kilep && !enter) {
        /* doboz kirajzolasa */
        boxRGBA(renderer, teglalap.x, teglalap.y, teglalap.x + teglalap.w - 1, teglalap.y + teglalap.h - 1, hatter.r, hatter.g, hatter.b, 255);
        rectangleRGBA(renderer, teglalap.x, teglalap.y, teglalap.x + teglalap.w - 1, teglalap.y + teglalap.h - 1, szoveg.r, szoveg.g, szoveg.b, 255);
        /* szoveg kirajzolasa */
        int w;
        strcpy(textandcomposition, dest);
        strcat(textandcomposition, composition);
        if (textandcomposition[0] != '\0') {
            SDL_Surface *felirat = TTF_RenderUTF8_Blended(font, textandcomposition, szoveg);
            SDL_Texture *felirat_t = SDL_CreateTextureFromSurface(renderer, felirat);
            SDL_Rect cel = { teglalap.x, teglalap.y, felirat->w < maxw ? felirat->w : maxw, felirat->h < maxh ? felirat->h : maxh };
            SDL_RenderCopy(renderer, felirat_t, NULL, &cel);
            SDL_FreeSurface(felirat);
            SDL_DestroyTexture(felirat_t);
            w = cel.w;
        } else {
            w = 0;
        }
        /* kurzor kirajzolasa */
        if (w < maxw) {
            vlineRGBA(renderer, teglalap.x + w + 2, teglalap.y + 2, teglalap.y + teglalap.h - 3, szoveg.r, szoveg.g, szoveg.b, 192);
        }
        /* megjeleniti a képernyon az eddig rajzoltakat */
        SDL_RenderPresent(renderer);

        SDL_Event event;
        SDL_WaitEvent(&event);
        switch (event.type) {
            /* Kulonleges karakter */
            case SDL_KEYDOWN:
                if (event.key.keysym.sym == SDLK_BACKSPACE) {
                    int textlen = strlen(dest);
                    do {
                        if (textlen == 0) {
                            break;
                        }
                        if ((dest[textlen-1] & 0x80) == 0x00)   {
                            /* Egy bajt */
                            dest[textlen-1] = 0x00;
                            break;
                        }
                        if ((dest[textlen-1] & 0xC0) == 0x80) {
                            /* Bajt, egy tobb-bajtos szekvenciabol */
                            dest[textlen-1] = 0x00;
                            textlen--;
                        }
                        if ((dest[textlen-1] & 0xC0) == 0xC0) {
                            /* Egy tobb-bajtos szekvencia elso bajtja */
                            dest[textlen-1] = 0x00;
                            break;
                        }
                    } while(true);
                }
                if (event.key.keysym.sym == SDLK_RETURN) {
                    enter = true;
                }
                break;

            /* A feldolgozott szoveg bemenete */
            case SDL_TEXTINPUT:
                if (strlen(dest) + strlen(event.text.text) < hossz) {
                    strcat(dest, event.text.text);
                }

                /* Az eddigi szerkesztes torolheto */
                composition[0] = '\0';
                break;

            /* Szoveg szerkesztese */
            case SDL_TEXTEDITING:
                strcpy(composition, event.edit.text);
                break;

            case SDL_QUIT:
                /* visszatesszuk a sorba ezt az eventet, mert
                 * sok mindent nem tudunk vele kezdeni */
                SDL_PushEvent(&event);
                kilep = true;
                break;
        }
    }

    /* igaz jelzi a helyes beolvasast; = ha enter miatt allt meg a ciklus */
    SDL_StopTextInput();
    return enter;
}
