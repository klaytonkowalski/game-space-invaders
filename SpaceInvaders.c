#include "raylib.h"
#include <stdio.h>

#define MAX_ALIEN_COUNT 64
#define MAX_BULLET_COUNT 64

typedef enum GameState
{
    startState,
    readyState,
    playState,
    winState,
    loseState
}
GameState;

typedef struct Player
{
    Vector2 position;
    int livesRemaining;
    bool alive;
}
Player;

typedef struct Bullet
{
    Vector2 position;
    bool belongsToPlayer;
    bool active;
}
Bullet;

typedef struct Alien
{
    Vector2 position;
    bool alive;
}
Alien;

static const int screenWidth = 960;
static const int screenHalfWidth = 480;
static const int screenHeight = 540;
static const int screenHalfHeight = 270;
static const int targetFPS = 60;
static const float cameraZoom = 4;
static const int playerWidth = 8;
static const int playerHalfWidth = 4;
static const int playerHeight = 8;
static const int playerHalfHeight = 4;
static const int alienWidth = 8;
static const int alienHalfWidth = 4;
static const int alienHeight = 8;
static const int alienHalfHeight = 4;
static const int playerBulletRadius = 4;
static const int playerBulletDiameter = 8;
static const int alienBulletRadius = 2;
static const int alienBulletDiameter = 4;
static const Color playerBulletColor = GOLD;
static const Color alienBulletColor = PURPLE;
static const int playerSpeed = 3;
static const float alienSpeed = 0.5;
static const int playerBulletSpeed = 2;
static const int alienBulletSpeed = 1;
static const int textSize = 20;
static const float delayThreshold = 3;
static const float animationThreshold = 0.5;
static const int animationFrameCount = 2;

static Texture2D playerTexture;
static Texture2D alienTexture;

static GameState gameState;
static Player player;
static Camera2D camera;
static Rectangle cameraBounds;
static Alien aliens[MAX_ALIEN_COUNT];
static int nextAvailableAlien;
static Bullet bullets[MAX_BULLET_COUNT];
static int nextAvailableBullet;
static int wave;
static float frameTime;
static float readyElapsed;
static float winElapsed;
static float loseElapsed;
static int alienFrameIndex;
static float alienFrameElapsed;
static bool alienDirection;

void Initialize();
void Update();
void Draw();
void Terminate();

void FromStartToReadyState();
void FromReadyToPlayState();
void FromPlayToWinState();
void FromPlayToLoseState();
void FromWinToReadyState();
void FromLoseToReadyState();
void FromLoseToStartState();

void DrawStartState();
void DrawReadyState();
void DrawPlayState();
void DrawWinState();
void DrawLoseState();

void DrawBottomShelf();
void DrawPlayer();
void DrawAliens();
void DrawBullets();

void UpdateStartState();
void UpdateReadyState();
void UpdatePlayState();
void UpdateWinState();
void UpdateLoseState();

void UpdateAlienAnimations();

int main(int argc, char *argv[])
{
    Initialize();
    while (!WindowShouldClose())
    {
        Update();
        Draw();
    }
    Terminate();
    return 0;
}

void Initialize()
{
    InitWindow(screenWidth, screenHeight, "Space Invaders");
    SetTargetFPS(targetFPS);
    playerTexture = LoadTexture("Player.png");
    alienTexture = LoadTexture("Alien.png");
    gameState = startState;
    player.position = (Vector2) { screenHalfWidth - playerHalfWidth, screenHalfHeight - playerHalfHeight };
    player.livesRemaining = 3;
    player.alive = true;
    camera.offset = (Vector2) { screenHalfWidth, screenHalfHeight };
    camera.target = (Vector2) { player.position.x + playerHalfWidth, player.position.y + playerHalfHeight - 50 };
    camera.rotation = 0;
    camera.zoom = cameraZoom;
    cameraBounds = (Rectangle) { 360, 150, screenWidth * 0.25, screenHeight * 0.25 };
    for (int i = 0; i < MAX_ALIEN_COUNT; ++i)
    {
        aliens[i].alive = false;
    }
    nextAvailableAlien = 0;
    for (int i = 0; i < MAX_BULLET_COUNT; ++i)
    {
        bullets[i].active = false;
    }
    nextAvailableBullet = 0;
    wave = 1;
    frameTime = 0;
    readyElapsed = 0;
    winElapsed = 0;
    loseElapsed = 0;
    alienFrameIndex = 0;
    alienFrameElapsed = 0;
    alienDirection = 0;
}

void Update()
{
    frameTime = GetFrameTime();
    if (gameState == startState)
        UpdateStartState();
    else if (gameState == readyState)
        UpdateReadyState();
    else if (gameState == playState)
        UpdatePlayState();
    else if (gameState == winState)
        UpdateWinState();
    else if (gameState == loseState)
        UpdateLoseState();
}

void Draw()
{
    BeginDrawing();
    ClearBackground(BLACK);
    if (gameState == startState)
        DrawStartState();
    else if (gameState == readyState)
        DrawReadyState();
    else if (gameState == playState)
        DrawPlayState();
    else if (gameState == winState)
        DrawWinState();
    else if (gameState == loseState)
        DrawLoseState();
    EndDrawing();
}

void Terminate()
{
    UnloadTexture(playerTexture);
    UnloadTexture(alienTexture);
    CloseWindow();
}

void FromStartToReadyState()
{
    gameState = readyState;
}

void FromReadyToPlayState()
{
    gameState = playState;
    const int rows = Clamp(wave / 3 + 1, 1, 5);
    for (int row = 0; row < rows; ++row)
    {
        for (int column = -5; column <= 5; ++column)
        {
            aliens[nextAvailableAlien].position = (Vector2) { camera.target.x - column * (alienWidth + alienHalfWidth), cameraBounds.y + 20 };
            aliens[nextAvailableAlien].alive = true;
            ++nextAvailableAlien;
            nextAvailableAlien %= MAX_ALIEN_COUNT;
        }
    }
}

void FromPlayToWinState()
{
    gameState = winState;
}

void FromPlayToLoseState()
{
    gameState = loseState;
}

void FromWinToReadyState()
{
    gameState = readyState;
}

void FromLoseToReadyState()
{
    gameState = readyState;
}

void FromLoseToStartState()
{
    gameState = startState;
}

void DrawStartState()
{
    const int startHalfWidth = MeasureText("Press SHOOT To Play!", textSize) * 0.5;
    DrawText("Press SHOOT To Play!", screenHalfWidth - startHalfWidth, 40, textSize, WHITE);
    BeginMode2D(camera);
    DrawPlayer();
    EndMode2D();
}

void DrawReadyState()
{
    char readyBuffer[14];
    sprintf(readyBuffer, "Ready Wave %d", wave);
    const int readyHalfWidth = MeasureText(readyBuffer, textSize) * 0.5;
    DrawText(readyBuffer, screenHalfWidth - readyHalfWidth, 40, textSize, WHITE);
    DrawBottomShelf();
    BeginMode2D(camera);
    DrawPlayer();
    EndMode2D();
}

void DrawPlayState()
{
    DrawBottomShelf();
    BeginMode2D(camera);
    DrawPlayer();
    DrawAliens();
    DrawBullets();
    EndMode2D();
}

void DrawWinState()
{
    char winBuffer[18];
    sprintf(winBuffer, "Wave %d Complete!", wave);
    const int winHalfWidth = MeasureText(winBuffer, textSize) * 0.5;
    DrawText(winBuffer, screenHalfWidth - winHalfWidth, 40, textSize, WHITE);
    DrawBottomShelf();
    BeginMode2D(camera);
    DrawPlayer();
    DrawBullets();
    EndMode2D();
}

void DrawLoseState()
{
    const int loseHalfWidth = MeasureText("You died!", textSize) * 0.5;
    DrawText("You died!", screenHalfWidth - loseHalfWidth, 40, textSize, WHITE);
    DrawBottomShelf();
    BeginMode2D(camera);
    DrawAliens();
    DrawBullets();
    EndMode2D();
}

void DrawBottomShelf()
{
    char livesBuffer[9];
    sprintf(livesBuffer, "Lives: %d", player.livesRemaining);
    DrawText(livesBuffer, 20, screenHeight - textSize - 20, textSize, WHITE);
    char waveBuffer[9];
    sprintf(waveBuffer, "Wave: %d", wave);
    const int waveBufferWidth = MeasureText(waveBuffer, textSize);
    DrawText(waveBuffer, screenWidth - waveBufferWidth - 20, screenHeight - textSize - 20, textSize, WHITE);
}

void DrawPlayer()
{
    DrawTextureV(playerTexture, player.position, WHITE);
}

void DrawAliens()
{
    for (int i = 0; i < MAX_ALIEN_COUNT; ++i)
    {
        if (aliens[i].alive)
        {
            DrawTextureRec(alienTexture, (Rectangle) { alienFrameIndex * alienWidth, 0, alienWidth, alienHeight }, aliens[i].position, WHITE);
        }
    }
}

void DrawBullets()
{
    for (int i = 0; i < MAX_BULLET_COUNT; ++i)
    {
        if (bullets[i].active)
        {
            if (bullets[i].belongsToPlayer)
            {
                DrawCircleV(bullets[i].position, playerBulletRadius, playerBulletColor);
            }
            else
            {
                DrawCircleV(bullets[i].position, alienBulletRadius, alienBulletColor);
            }
        }
    }
}

void UpdateStartState()
{
    if (IsKeyDown(KEY_SPACE))
    {
        FromStartToReadyState();
    }
}

void UpdateReadyState()
{
    readyElapsed += frameTime;
    if (readyElapsed > delayThreshold)
    {
        FromReadyToPlayState();
    }
}

void UpdatePlayState()
{
    if (IsKeyDown(KEY_A))
    {
        player.position.x -= playerSpeed;
        if (player.position.x < cameraBounds.x)
        {
            player.position.x = cameraBounds.x;
        }
        
    }
    if (IsKeyDown(KEY_D))
    {
        player.position.x += playerSpeed;
        if (player.position.x > cameraBounds.x + cameraBounds.width - playerWidth)
        {
            player.position.x = cameraBounds.x + cameraBounds.width - playerWidth;
        }
    }
    if (IsKeyPressed(KEY_SPACE))
    {
        bullets[nextAvailableBullet].position = (Vector2) { player.position.x + playerHalfWidth, player.position.y - playerBulletRadius };
        bullets[nextAvailableBullet].belongsToPlayer = true;
        bullets[nextAvailableBullet].active = true;
        ++nextAvailableBullet;
        nextAvailableBullet %= MAX_BULLET_COUNT;
    }
    UpdateAlienAnimations();
    int newAlienDirection = alienDirection;
    for (int i = 0; i < MAX_ALIEN_COUNT; ++i)
    {
        if (aliens[i].alive)
        {
            if (alienDirection == 0)
            {
                aliens[i].position.x += alienSpeed;
                if (aliens[i].position.x > cameraBounds.x + cameraBounds.width - alienWidth)
                {
                    newAlienDirection = 1;
                }
            }
            else if (alienDirection == 1)
            {
                aliens[i].position.x -= alienSpeed;
                if (aliens[i].position.x < cameraBounds.x)
                {
                    newAlienDirection = 0;
                }
            }
            // Todo: Shoot alien bullets.
        }
    }
    alienDirection = newAlienDirection;
    for (int i = 0; i < MAX_BULLET_COUNT; ++i)
    {
        if (bullets[i].active)
        {
            if (bullets[i].belongsToPlayer)
            {
                bullets[i].position.y -= playerBulletSpeed;
            }
            else
            {
                bullets[i].position.y += alienBulletSpeed;
            }
            if (bullets[i].position.y < cameraBounds.y)
            {
                bullets[i].active = false;
            }
        }
    }
}

void UpdateWinState()
{
    winElapsed += frameTime;
    if (winElapsed > delayThreshold)
    {
        FromWinToReadyState();
    }
}

void UpdateLoseState()
{
    UpdateAlienAnimations();
    loseElapsed += frameTime;
    if (loseElapsed > delayThreshold)
    {
        if (player.livesRemaining > 0)
        {
            FromLoseToReadyState();
        }
        else
        {
            FromLoseToStartState();
        }
    }
}

void UpdateAlienAnimations()
{
    alienFrameElapsed += frameTime;
    if (alienFrameElapsed > animationThreshold)
    {
        alienFrameElapsed = 0;
        ++alienFrameIndex;
        alienFrameIndex %= animationFrameCount;
    }
}