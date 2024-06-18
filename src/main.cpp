#include <psp2/kernel/processmgr.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <iostream>
#include <vector>

enum {
  SCREEN_WIDTH  = 960,
  SCREEN_HEIGHT = 544
};
const int FRAME_RATE = 60;

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
SDL_GameController* controller = NULL;
 
typedef struct
{
    SDL_Rect bounds;
    SDL_Texture *sprite;
    int lives;
    int speed;
} Player;

Player player;

typedef struct
{
    SDL_Rect bounds;
    SDL_Texture *sprite;
    int points;
    int velocityX;
    bool shouldMove;
    bool isDestroyed;
} MysteryShip;

MysteryShip mysteryShip;

typedef struct
{
    SDL_Rect bounds;
    SDL_Texture *sprite;
    int lives;
    bool isDestroyed;
} Structure;

std::vector<Structure> structures;

SDL_Texture *loadSprite(const char *file)
{
    SDL_Texture *texture = IMG_LoadTexture(renderer, file);
    return texture;
}

typedef struct
{
    SDL_Rect bounds;
    SDL_Texture *sprite;
    int points;
    int velocity;
    bool isDestroyed;
} Alien;

std::vector<Alien> aliens;

std::vector<Alien> createAliens()
{
    SDL_Texture *alienSprite1 = loadSprite("sprites/alien_1.png");
    SDL_Texture *alienSprite2 = loadSprite("sprites/alien_2.png");
    SDL_Texture *alienSprite3 = loadSprite("sprites/alien_3.png");

    std::vector<Alien> aliens;

    int positionX;
    int positionY = 80;
    int alienPoints = 8;

    SDL_Texture *actualSprite;

    for (int row = 0; row < 5; row++)
    {
        positionX = 150;

        switch (row)
        {

        case 0:
            actualSprite = alienSprite3;
            break;

        case 1:
        case 2:
            actualSprite = alienSprite2;
            break;

        default:
            actualSprite = alienSprite1;
        }

        for (int columns = 0; columns < 11; columns++)
        {
            SDL_Rect alienBounds = {positionX, positionY, 38, 34};

            Alien actualAlien = {alienBounds, actualSprite, alienPoints, 50, false};

            aliens.push_back(actualAlien);
            positionX += 60;
        }

        alienPoints--;
        positionY += 50;
    }

    return aliens;
}

void quitGame() {

    SDL_GameControllerClose(controller);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void handleEvents() {

    SDL_Event event;

    while (SDL_PollEvent(&event)) {

        if (event.type == SDL_QUIT) {
            
            quitGame();
            exit(0);
        }
    }
}
 
void update(float deltaTime) {

    SDL_GameControllerUpdate();

    if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_LEFT) && player.bounds.x > 0) {
        player.bounds.x -= player.speed * deltaTime;
    }

    else if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_RIGHT) && player.bounds.x < SCREEN_WIDTH - player.bounds.w) {
        player.bounds.x += player.speed * deltaTime;
    }
}

void renderSprite(SDL_Texture *sprite, SDL_Rect spriteBounds)
{
    SDL_QueryTexture(sprite, NULL, NULL, &spriteBounds.w, &spriteBounds.h);
    SDL_RenderCopy(renderer, sprite, NULL, &spriteBounds);
}

void render()
{
    SDL_SetRenderDrawColor(renderer, 29, 29, 27, 255);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

    renderSprite(mysteryShip.sprite, mysteryShip.bounds);

    for (Alien alien : aliens)
    {
        renderSprite(alien.sprite, alien.bounds);
    }

    for (Structure structure : structures)
    {
        renderSprite(structure.sprite, structure.bounds);
    }

    renderSprite(player.sprite, player.bounds);

    SDL_RenderPresent(renderer);
}

void capFrameRate(Uint32 frameStartTime) {

    Uint32 frameTime = SDL_GetTicks() - frameStartTime;
    
    if (frameTime < 1000 / FRAME_RATE) {
        SDL_Delay(1000 / FRAME_RATE - frameTime);
    }
}

int main() {

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) < 0) {
        return -1;
    }

    if ((window = SDL_CreateWindow("space", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN)) == NULL) {
        return -1;
    }

    if ((renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC)) == NULL) {
        return -1;
    }

    if (SDL_NumJoysticks() < 1) {
        printf("No game controllers connected!\n");
        return -1;
    } 
    else {

        controller = SDL_GameControllerOpen(0);
        if (controller == NULL) {

            printf("Unable to open game controller! SDL Error: %s\n", SDL_GetError());
            return -1;
        }
    }

    if (!IMG_Init(IMG_INIT_PNG)){
        return -1;
    }

    SDL_Texture *shipSprite = loadSprite("sprites/mystery.png");

    SDL_Rect shipBounds = {SCREEN_WIDTH / 2, 30, 58, 25};

    mysteryShip = {shipBounds, shipSprite, 50, -100, false, false};

    aliens = createAliens();

    SDL_Texture *playerSprite = loadSprite("sprites/spaceship.png");

    SDL_Rect playerBounds = {SCREEN_WIDTH / 2, SCREEN_HEIGHT - 50, 38, 34};

    player = {playerBounds, playerSprite, 2, 600};

    SDL_Rect structureBounds = {120, SCREEN_HEIGHT - 100, 56, 33};
    SDL_Rect structureBounds2 = {350, SCREEN_HEIGHT - 100, 56, 33};
    SDL_Rect structureBounds3 = {200*3, SCREEN_HEIGHT - 100, 56, 33};
    SDL_Rect structureBounds4 = {200*4, SCREEN_HEIGHT - 100, 56, 33};

    SDL_Texture *structureSprite = loadSprite("sprites/structure.png");

    structures.push_back({structureBounds, structureSprite, 5, false});
    structures.push_back({structureBounds2, structureSprite, 5, false});
    structures.push_back({structureBounds3, structureSprite, 5, false});
    structures.push_back({structureBounds4, structureSprite, 5, false});

    Uint32 previousFrameTime = SDL_GetTicks();
    Uint32 currentFrameTime = previousFrameTime;
    float deltaTime = 0.0f;

    while (true)
    {
        currentFrameTime = SDL_GetTicks();

        deltaTime = (currentFrameTime - previousFrameTime) / 1000.0f;

        previousFrameTime = currentFrameTime;

        handleEvents();
        update(deltaTime);
        render();
    }

    quitGame();
}