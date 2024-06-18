#include <psp2/kernel/processmgr.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

enum {
  SCREEN_WIDTH  = 960,
  SCREEN_HEIGHT = 544
};

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
SDL_GameController* controller = NULL;
SDL_Texture* sprite;

SDL_Rect spriteBounds = {SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 64, 64};

const int SPEED = 600; 
const int FRAME_RATE = 60;
 
SDL_Texture* loadSprite(const char* file, SDL_Renderer* renderer) {

    SDL_Texture* texture = IMG_LoadTexture(renderer, file);
    return texture;
}

void renderSprite(SDL_Texture* sprite, SDL_Renderer* renderer, SDL_Rect spriteBounds) {

    SDL_QueryTexture(sprite, NULL, NULL, &spriteBounds.w, &spriteBounds.h);
    SDL_RenderCopy(renderer, sprite, NULL, &spriteBounds);
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

    if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_UP) && spriteBounds.y > 0) {
        spriteBounds.y -= SPEED * deltaTime;
    }

    else if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_DOWN) && spriteBounds.y < SCREEN_HEIGHT - spriteBounds.h) {
        spriteBounds.y += SPEED * deltaTime;
    }

    else if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_LEFT) && spriteBounds.x > 0) {
        spriteBounds.x -= SPEED * deltaTime;
    }

    else if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_RIGHT) && spriteBounds.x < SCREEN_WIDTH - spriteBounds.w) {
        spriteBounds.x += SPEED * deltaTime;
    }
}

void render() {

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

    renderSprite(sprite, renderer, spriteBounds);

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

    if ((window = SDL_CreateWindow("sdl-image", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN)) == NULL) {
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

    sprite = loadSprite("sprites/sprite.png", renderer);

    Uint32 previousFrameTime = SDL_GetTicks();
    Uint32 currentFrameTime;
    float deltaTime;

    while (true) {

        currentFrameTime = SDL_GetTicks();
        deltaTime = (currentFrameTime - previousFrameTime) / 1000.0f;
        previousFrameTime = currentFrameTime;

        handleEvents();
        update(deltaTime);
        render();

        capFrameRate(currentFrameTime);
    }

    quitGame();
}