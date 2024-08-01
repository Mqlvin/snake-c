#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <stdlib.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#define GRID_SIZE 32
#define GRID_X 24
#define GRID_Y 16

#define MAX(a,b) (((a)>(b))?(a):(b))

void SDLErrorAndExit(const char *cause);
void IMGErrorAndExit(const char *cause);
void SDLMakeSurface(const char *name, SDL_Surface **surface);

void endGame(SDL_Window *window);
void renderCube(SDL_Surface *windowSurface, int x, int y, int r, int g, int b, int a);
int lerp(int a, int b, double t);

void titleScreen(SDL_Window *window, SDL_Surface *windowSurface, SDL_Surface *logo, SDL_Surface *controls);
void deathAnim(SDL_Window *window, SDL_Surface *windowSurface);
void game(SDL_Window *window, SDL_Surface *windowSurface);


typedef struct Pos {
    int x;
    int y;
} Pos;

SDL_KeyCode key = SDLK_CLEAR; // "unpressable" key represents no key pressed

Pos powerup;
Pos snakeBody[GRID_X * GRID_Y];
int snakeLength = 0;
Pos nextSnakePart = (Pos) {.x = -1};

SDL_Rect logoRect;
SDL_Rect controlsRect;
bool gameStarted = false; // determines whether start screen showed
int introAnimation = 0; // ticks of anim
bool isDead = false;
int deadAnimation = 0; // ticks of anim


int main(int argc, char **argv) {
    srand(time(NULL)); 

    // init splash screen positions
    logoRect.x = ((GRID_X * GRID_SIZE) / 2) - (279 / 2);
    logoRect.y = (157 / 2);
    controlsRect.x = ((GRID_X * GRID_SIZE) / 2) - (404 / 2);
    controlsRect.y = (GRID_Y * GRID_SIZE) - 160;

    // init starting point of powerup/snake
    powerup = (Pos) { .x = rand() % GRID_X, .y = rand() % GRID_Y };
    snakeBody[0] = (Pos) { .x = GRID_X / 2, .y = GRID_Y / 2 };
    snakeLength++;


    // init window and window surface
    SDL_Window *window = NULL;
    SDL_Surface *wSurface = NULL;
    
    if(SDL_Init(SDL_INIT_VIDEO) < 0) SDLErrorAndExit("SDL could not initialise");

    window = SDL_CreateWindow("Snake", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, GRID_SIZE * GRID_X, GRID_SIZE * GRID_Y, SDL_WINDOW_SHOWN);
    if(window == NULL) SDLErrorAndExit("Failed to create SDL window");
 
    wSurface = SDL_GetWindowSurface(window);
    if(wSurface == NULL) SDLErrorAndExit("Failed to get window surface");


    // init image loader / load assets
    int sdlImgFlags = IMG_INIT_PNG;
    if(!(IMG_Init(sdlImgFlags) & sdlImgFlags)) {
        IMGErrorAndExit("SDL Image could not intialise");
    }
    
    SDL_Surface *logo = NULL;
    SDLMakeSurface("res/logo.png", &logo);
    SDL_Surface *controls = NULL;
    SDLMakeSurface("res/controls.png", &controls);
    
    if(logo == NULL || controls == NULL) {
        IMGErrorAndExit("Failed to load image surfaces");
    }


    // main game loop
    SDL_Event e;
    while(true) {
        while(SDL_PollEvent(&e)) {
            if(e.type == SDL_QUIT) { 
                endGame(window);
            } else if(e.type == SDL_KEYDOWN) {
                switch(e.key.keysym.sym) {
                    case SDLK_DOWN:
                        if(key != SDLK_UP) key = SDLK_DOWN;
                        break;
                    case SDLK_UP:
                        if(key != SDLK_DOWN) key = SDLK_UP;
                        break;
                    case SDLK_RIGHT:
                        if(key != SDLK_LEFT) key = SDLK_RIGHT;
                        break;
                    case SDLK_LEFT:
                        if(key != SDLK_RIGHT) key = SDLK_LEFT;
                        break;
                    default:
                        break;
                }
            }
        }

        // all do update+render, funcs can be found below
        if(!gameStarted) {
            titleScreen(window, wSurface, logo, controls);
        } else if(isDead && gameStarted) {
            deathAnim(window, wSurface);
        } else {
            game(window, wSurface);
        }
    }
    
    // should theoretically never get here
    endGame(window);
}



// main game loop funcs
void titleScreen(SDL_Window *window, SDL_Surface *windowSurface, SDL_Surface *logo, SDL_Surface *controls) {
    SDL_FillRect(windowSurface, NULL, SDL_MapRGB(windowSurface->format, 0xF2, 0xE8, 0xCF));

    if(key != SDLK_CLEAR) { // therefore arrow key pressed
        introAnimation++;
                
        int newAlpha = MAX(255 - introAnimation * 5, 0);
                
        SDL_SetSurfaceAlphaMod(logo, newAlpha);
        SDL_SetSurfaceAlphaMod(controls, newAlpha);

        if(introAnimation > 70) {
            gameStarted = true;
        }
    } else {
        SDL_SetSurfaceAlphaMod(controls, (sin(SDL_GetTicks() * 0.005) + 1) * 50 + 155);
    }

    SDL_BlitSurface(logo, NULL, windowSurface, &logoRect);
    SDL_BlitSurface(controls, NULL, windowSurface, &controlsRect);

    SDL_UpdateWindowSurface(window);
    SDL_Delay(16);
}

void deathAnim(SDL_Window *window, SDL_Surface *windowSurface) {
    deadAnimation++;

    SDL_FillRect(windowSurface, NULL, SDL_MapRGB(windowSurface->format, 0xF2, 0xE8, 0xCF));

              
    renderCube(windowSurface, powerup.x, powerup.y, 0xBC, 0x47, 0x49, 0xFF);
    for(int i = 0; i < snakeLength; ++i) {
        double lerpT = (double) i / (double) snakeLength;
        bool isRed = (deadAnimation / 10) % 2 == 0;
        renderCube(windowSurface, snakeBody[i].x, snakeBody[i].y, isRed ? lerp(0xBC, 0xE8, lerpT) : lerp(0x38, 0xA7, lerpT), isRed ? lerp(0x47, 0x68, lerpT) : lerp(0x66, 0xC9, lerpT), isRed ? lerp(0x41, 0x57, lerpT) : lerp(0x49, 0x6A, lerpT), 0xFF);
    }

    SDL_UpdateWindowSurface(window);
    SDL_Delay(16);

    if(deadAnimation > 60) {
        endGame(window);
    }
}

void game(SDL_Window *window, SDL_Surface *windowSurface) {
    // If snake/powerup colliding
    if(snakeBody[0].x == powerup.x && snakeBody[0].y == powerup.y) {
        powerup.x = rand() % GRID_X;
        powerup.y = rand() % GRID_Y;
        nextSnakePart = snakeBody[snakeLength - 1]; // registers part to be added
    }

    // Update all except front snake part to be ahead
    // Also since we're looping through, check if head position colliding
    for(int i = snakeLength - 1; i >= 1; --i) {
        snakeBody[i] = snakeBody[i - 1];
    }

    // If new snake part needed, add it to bottom
    if(nextSnakePart.x > -1) {
        snakeBody[snakeLength] = nextSnakePart;
        snakeLength++;

        nextSnakePart = (Pos) {.x = -1};
    }

    // Move snake front part forward, based on key (could do this before adding new part)
    switch(key) {
        case SDLK_DOWN:
            snakeBody[0].y++;
            break;
        case SDLK_UP:
            snakeBody[0].y--;
            break;
        case SDLK_RIGHT:
            snakeBody[0].x++;
            break;
        case SDLK_LEFT:
            snakeBody[0].x--;
            break;
        default:
            break;
    }

    for(int i = 1; i < snakeLength; i++) {
        if(snakeBody[0].x == snakeBody[i].x && snakeBody[0].y == snakeBody[i].y) {
            isDead = true;
        }   
    }

    if(snakeBody[0].x < 0 || snakeBody[0].x >= GRID_X || snakeBody[0].y < 0 || snakeBody[0].y >= GRID_Y) {
        isDead = true;
    }


    SDL_FillRect(windowSurface, NULL, SDL_MapRGB(windowSurface->format, 0xF2, 0xE8, 0xCF));

    // Render powerup, then snake parts
    renderCube(windowSurface, powerup.x, powerup.y, 0xBC, 0x47, 0x49, 0xFF);
    for(int i = 0; i < snakeLength; ++i) {
        double lerpT = (double) i / (double) snakeLength;
        renderCube(windowSurface, snakeBody[i].x, snakeBody[i].y, lerp(0x38, 0xA7, lerpT), lerp(0x66, 0xC9, lerpT), lerp(0x41, 0x57, lerpT), 0xFF);
    }
    
    SDL_UpdateWindowSurface(window);
    SDL_Delay(150);
}



// other util funcs
void IMGErrorAndExit(const char *cause) {
    printf("%s! IMG_Error: %s\n", cause, IMG_GetError());
}

void SDLErrorAndExit(const char *cause) {
    printf("%s! SDL_Error: %s\n", cause, SDL_GetError());
    exit(1);
}

void SDLMakeSurface(const char *name, SDL_Surface **surface) {
    SDL_Surface *loadedSurface = IMG_Load(name);
    if(loadedSurface == NULL) {
        IMGErrorAndExit("Could not load image file from path");
    }
    *surface = SDL_ConvertSurfaceFormat(loadedSurface, SDL_PIXELFORMAT_RGBA32, 0);

    SDL_FreeSurface(loadedSurface);
}

void endGame(SDL_Window *window) {
    SDL_DestroyWindow(window);
    SDL_Quit();
    exit(0);
}

void renderCube(SDL_Surface *windowSurface, int x, int y, int r, int g, int b, int a) {
    SDL_Rect rect;
    rect.x = x * GRID_SIZE;
    rect.y = y * GRID_SIZE;
    rect.w = GRID_SIZE;
    rect.h = GRID_SIZE;

    SDL_FillRect(windowSurface, &rect, SDL_MapRGBA(windowSurface->format, r, g, b, a));
}

int lerp(int a, int b, double t) {
    return a * (1 - t) + b * t;
}
