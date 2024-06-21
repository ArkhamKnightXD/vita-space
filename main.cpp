#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <iostream>
#include <vector>

enum {
  SCREEN_WIDTH  = 960,
  SCREEN_HEIGHT = 544
};

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
SDL_GameController* controller = NULL;

Mix_Chunk *laserSound = nullptr;
Mix_Chunk *explosionSound = nullptr;
 
typedef struct
{
    SDL_Rect bounds;
    bool isDestroyed;
} Laser;

std::vector<Laser> playerLasers;
std::vector<Laser> alienLasers;

float lastTimePlayerShoot;
float lastTimeAliensShoot;

typedef struct
{
    SDL_Rect bounds;
    SDL_Texture *sprite;
    int lives;
    int speed;
    int score;
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

float lastTimeMysteryShipSpawn;

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
    float x;
    float y;
    SDL_Rect bounds;
    SDL_Texture *sprite;
    int points;
    int velocity;
    bool isDestroyed;
} Alien;

std::vector<Alien> aliens;

bool shouldChangeVelocity = false;
bool shouldAliensGoDown = false;

std::vector<Alien> createAliens()
{
    SDL_Texture *alienSprite1 = loadSprite("alien_1.png");
    SDL_Texture *alienSprite2 = loadSprite("alien_2.png");
    SDL_Texture *alienSprite3 = loadSprite("alien_3.png");

    std::vector<Alien> aliens;

    int positionX;
    int positionY = 50;
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

        for (int columns = 0; columns < 13; columns++)
        {
            SDL_Rect alienBounds = {positionX, positionY, 38, 34};

            Alien actualAlien = {(float)positionX, (float)positionY, alienBounds, actualSprite, alienPoints, 100, false};

            aliens.push_back(actualAlien);
            positionX += 60;
        }

        alienPoints--;
        positionY += 50;
    }

    return aliens;
}

void aliensMovement(float deltaTime)
{
    for (Alien &alien : aliens)
    {
        float alienPosition = alien.bounds.x + alien.bounds.w;

        if ((!shouldChangeVelocity && alienPosition > SCREEN_WIDTH) || alienPosition < alien.bounds.w)
        {
            shouldChangeVelocity = true;
            shouldAliensGoDown = true;

            break;
        }
    }

    if (shouldChangeVelocity)
    {
        for (Alien &alien : aliens)
        {
            alien.velocity *= -1;
        }

        shouldChangeVelocity = false;
    }

    if (shouldAliensGoDown)
    {
        for (Alien &alien : aliens)
        {
            alien.y += 500 * deltaTime;
            alien.bounds.y = alien.y;
        }

        shouldAliensGoDown = false;
    }

    for (Alien &alien : aliens)
    {
        alien.x += alien.velocity * deltaTime;
        alien.bounds.x = alien.x;
    }
}

Mix_Chunk *loadSound(const char *p_filePath)
{
    Mix_Chunk *sound = nullptr;

    sound = Mix_LoadWAV(p_filePath);
    if (sound == nullptr)
    {
        printf("Failed to load scratch sound effect! SDL_mixer Error: %s\n", Mix_GetError());
    }

    return sound;
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

bool hasCollision(SDL_Rect bounds, SDL_Rect bounds2)
{
    return bounds.x < bounds2.x + bounds2.w && bounds.x + bounds.w > bounds2.x &&
           bounds.y < bounds2.y + bounds2.h && bounds.y + bounds.h > bounds2.y;
}

void checkCollisionBetweenStructureAndLaser(Laser &laser)
{
    for (Structure &structure : structures)
    {
        if (!structure.isDestroyed && hasCollision(structure.bounds, laser.bounds))
        {
            laser.isDestroyed = true;

            structure.lives--;

            if (structure.lives == 0)
            {
                structure.isDestroyed = true;
            }

            Mix_PlayChannel(-1, explosionSound, 0);
        }
    }
}

void removingDestroyedElements() {

    for (auto iterator = aliens.begin(); iterator != aliens.end();)
    {
        if (iterator->isDestroyed)
        {
            aliens.erase(iterator);
        }
        else
        {
            iterator++;
        }
    }

    for (auto iterator = playerLasers.begin(); iterator != playerLasers.end();)
    {
        if (iterator->isDestroyed)
        {
            playerLasers.erase(iterator);
        }
        else
        {
            iterator++;
        }
    }

    for (auto iterator = alienLasers.begin(); iterator != alienLasers.end();)
    {
        if (iterator->isDestroyed)
        {
            alienLasers.erase(iterator);
        }
        else
        {
            iterator++;
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

    if (!mysteryShip.shouldMove)
    {
        lastTimeMysteryShipSpawn += deltaTime;

        if (lastTimeMysteryShipSpawn >= 10)
        {
            lastTimeMysteryShipSpawn = 0;

            mysteryShip.shouldMove = true;
        }
    }

    if (mysteryShip.shouldMove)
    {
        if (mysteryShip.bounds.x > SCREEN_WIDTH + mysteryShip.bounds.w || mysteryShip.bounds.x < -80)
        {
            mysteryShip.velocityX *= -1;
            mysteryShip.shouldMove = false;
        }

        mysteryShip.bounds.x += mysteryShip.velocityX * deltaTime;
    }

    if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_A))
    {
        lastTimePlayerShoot += deltaTime;

        if (lastTimePlayerShoot >= 0.35)
        {
            SDL_Rect laserBounds = {player.bounds.x + 20, player.bounds.y - player.bounds.h, 4, 16};

            playerLasers.push_back({laserBounds, false});

            lastTimePlayerShoot = 0;

            Mix_PlayChannel(-1, laserSound, 0);
        }
    }

    for (Laser &laser : playerLasers)
    {
        laser.bounds.y -= 400 * deltaTime;

        if (laser.bounds.y < 0)
            laser.isDestroyed = true;
    }

    for (Laser &laser : playerLasers)
    {
        if (!mysteryShip.isDestroyed && hasCollision(mysteryShip.bounds, laser.bounds))
        {
            laser.isDestroyed = true;

            player.score += mysteryShip.points;

            mysteryShip.isDestroyed = true;

            Mix_PlayChannel(-1, explosionSound, 0);
        }

        for (Alien &alien : aliens)
        {
            if (!alien.isDestroyed && hasCollision(alien.bounds, laser.bounds))
            {
                alien.isDestroyed = true;
                laser.isDestroyed = true;

                player.score += alien.points;

                Mix_PlayChannel(-1, explosionSound, 0);
            }
        }

        checkCollisionBetweenStructureAndLaser(laser);
    }

    lastTimeAliensShoot += deltaTime;

    if (aliens.size() > 0 && lastTimeAliensShoot >= 0.6)
    {
        int randomAlienIndex = rand() % aliens.size();

        Alien alienShooter = aliens[randomAlienIndex];

        SDL_Rect laserBounds = {alienShooter.bounds.x + 20, alienShooter.bounds.y + alienShooter.bounds.h, 4, 16};

        alienLasers.push_back({laserBounds, false});

        lastTimeAliensShoot = 0;

        Mix_PlayChannel(-1, laserSound, 0);
    }

    for (Laser &laser : alienLasers)
    {
        laser.bounds.y += 400 * deltaTime;

        if (laser.bounds.y > SCREEN_HEIGHT)
            laser.isDestroyed = true;
    }

    for (Laser &laser : alienLasers)
    {
        if (player.lives > 0 && hasCollision(player.bounds, laser.bounds))
        {
            laser.isDestroyed = true;

            player.lives--;

            Mix_PlayChannel(-1, explosionSound, 0);
        }

        checkCollisionBetweenStructureAndLaser(laser);
    }

    aliensMovement(deltaTime);

    removingDestroyedElements();
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

    if (!mysteryShip.isDestroyed)
    {
        renderSprite(mysteryShip.sprite, mysteryShip.bounds);
    }

    for (Alien alien : aliens)
    {
        if (!alien.isDestroyed)
        {
            renderSprite(alien.sprite, alien.bounds);
        }
    }

    SDL_SetRenderDrawColor(renderer, 243, 216, 63, 255);

    for (Laser laser : alienLasers)
    {
        if (!laser.isDestroyed)
        {
            SDL_RenderFillRect(renderer, &laser.bounds);
        }
    }

    for (Laser laser : playerLasers)
    {
        if (!laser.isDestroyed)
        {
            SDL_RenderFillRect(renderer, &laser.bounds);
        }
    }

    for (Structure structure : structures)
    {
        if (!structure.isDestroyed)
        {
            renderSprite(structure.sprite, structure.bounds);
        }
    }

    renderSprite(player.sprite, player.bounds);

    SDL_RenderPresent(renderer);
}

int main() {

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) < 0) {
        return -1;
    }

    if ((window = SDL_CreateWindow("space", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN)) == NULL) {
        return -1;
    }

    // Using SDL_RENDERER_PRESENTVSYNC effectively synchronizes your rendering with the display's refresh rate, which should cap the framerate to the refresh rate
    //  of the display (commonly 60 Hz, resulting in 60 FPS). This means that, in most cases, you don't need to manually cap the framerate since VSync already
    //  limits the maximum framerate to match the display's refresh rate.
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

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
    {
        printf("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
    }

    laserSound = loadSound("laser.ogg");
    explosionSound = loadSound("explosion.ogg");

    SDL_Texture *shipSprite = loadSprite("mystery.png");

    SDL_Rect shipBounds = {SCREEN_WIDTH, 40, 58, 25};

    mysteryShip = {shipBounds, shipSprite, 50, -150, false, false};

    aliens = createAliens();

    SDL_Texture *playerSprite = loadSprite("spaceship.png");

    SDL_Rect playerBounds = {SCREEN_WIDTH / 2, SCREEN_HEIGHT - 40, 38, 34};

    player = {playerBounds, playerSprite, 2, 600, 0};

    SDL_Rect structureBounds = {120, SCREEN_HEIGHT - 120, 56, 33};
    SDL_Rect structureBounds2 = {350, SCREEN_HEIGHT - 120, 56, 33};
    SDL_Rect structureBounds3 = {200 * 3, SCREEN_HEIGHT - 120, 56, 33};
    SDL_Rect structureBounds4 = {200 * 4, SCREEN_HEIGHT - 120, 56, 33};

    SDL_Texture *structureSprite = loadSprite("structure.png");

    structures.push_back({structureBounds, structureSprite, 5, false});
    structures.push_back({structureBounds2, structureSprite, 5, false});
    structures.push_back({structureBounds3, structureSprite, 5, false});
    structures.push_back({structureBounds4, structureSprite, 5, false});

    Uint32 previousFrameTime = SDL_GetTicks();
    Uint32 currentFrameTime = previousFrameTime;
    float deltaTime = 0.0f;

    srand(time(NULL));

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