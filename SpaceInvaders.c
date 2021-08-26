#include "raylib.h"

#include <stdio.h>

#define MAX_BULLET_COUNT 64

typedef enum GameState
{
    gameState_start,
    gameState_play,
    gameState_win,
    gameState_lose
}
GameState;

typedef struct Player
{
    Vector2 position;
    Texture2D texture;
    int lives;
    bool alive;
}
Player;

typedef struct Bullet
{
    Vector2 position;
    bool player;
    bool alive;
}
Bullet;

static const int screenWidth = 960;
static const int screenHeight = 540;

static const int targetFPS = 60;

static const int textSize = 20;

static const float startTheshold = 0.5;
static const float winThreshold = 3;
static const float loseThreshold = 3;

static const int playerSpeed = 3;

static const int bulletRadiusPlayer = 2;
static const int bulletRadiusAlien = 1;
static const int bulletSpeedPlayer = 2;
static const int bulletSpeedAlien = 1;

static GameState gameState;

static Player player;

static Camera2D camera;

static float frameTime;

static bool showStartText;
static float startElapsed;

static float winElapsed;

static float loseElapsed;

static int wave;

static Bullet bullets[MAX_BULLET_COUNT];
static int bulletIndex;

void Init();
void Update();
void Draw();
void Terminate();

int main(int argc, char *argv[])
{
    Init();
    while (!WindowShouldClose())
    {
        Update();
        Draw();
    }
    Terminate();
}

void Init()
{
    InitWindow(screenWidth, screenHeight, "Space Invaders");
    SetTargetFPS(targetFPS);
    player.texture = LoadTexture("Player.png");
    player.position = (Vector2) { screenWidth * 0.5 - player.texture.width * 0.5, screenHeight - player.texture.height - 20 };
    player.lives = 3;
    player.alive = true;
    camera.rotation = 0;
    camera.zoom = 4;
    camera.offset = (Vector2) { screenWidth * 0.5, screenHeight * 0.5 };
    camera.target = (Vector2) { player.position.x + player.texture.width * 0.5, player.position.y + player.texture.height * 0.5 - screenHeight * 0.35 / camera.zoom };
    frameTime = 0;
    showStartText = true;
    startElapsed = 0;
    winElapsed = 0;
    wave = 1;
    for (int i = 0; i < MAX_BULLET_COUNT; ++i)
    {
        bullets[i].position = (Vector2) { 0, 0 };
        bullets[i].player = false;
        bullets[i].alive = false;
    }
    bulletIndex = 0;
}

void Update()
{
    frameTime = GetFrameTime();
    if (gameState == gameState_start)
    {
        if (IsKeyDown(KEY_SPACE))
        {
            gameState = gameState_win;
            startElapsed = 0;
            showStartText = true;
        }
        else
        {
            startElapsed += frameTime;
            if (startElapsed > startTheshold)
            {
                startElapsed = 0;
                showStartText = !showStartText;
            }
        }
    }
    else if (gameState == gameState_play)
    {
        if (IsKeyDown(KEY_A))
        {
            player.position.x -= playerSpeed;
            const int leftBounds = screenWidth * 0.5 - screenWidth / camera.zoom * 0.5;
            if (player.position.x < leftBounds)
                player.position.x = leftBounds;
        }
        if (IsKeyDown(KEY_D))
        {
            player.position.x += playerSpeed;
            const int rightBounds = screenWidth * 0.5 + screenWidth / camera.zoom * 0.5 - player.texture.width;
            if (player.position.x > rightBounds)
                player.position.x = rightBounds;
        }
        if (IsKeyPressed(KEY_ENTER))
        {
            bullets[bulletIndex].position = (Vector2) { player.position.x + player.texture.width * 0.5, player.position.y - bulletRadiusPlayer };
            bullets[bulletIndex].player = true;
            bullets[bulletIndex].alive = true;
            ++bulletIndex;
            bulletIndex %= MAX_BULLET_COUNT;
        }
        for (int i = 0; i < MAX_BULLET_COUNT; ++i)
        {
            if (bullets[i].alive)
            {
                if (bullets[i].player)
                {
                    bullets[i].position.y -= bulletSpeedPlayer;
                    if (bullets[i].position.y < 190)
                        bullets[i].alive = false;
                }
                else
                {
                    bullets[i].position.y += bulletSpeedAlien;
                    if (bullets[i].position.y > 350)
                        bullets[i].alive = false;
                }
            }
        }
    }
    else if (gameState == gameState_win)
    {
        winElapsed += frameTime;
        if (winElapsed > winThreshold)
        {
            winElapsed = 0;
            gameState = gameState_play;
        }
    }
    else if (gameState == gameState_lose)
    {
        loseElapsed += frameTime;
        if (loseElapsed > loseThreshold)
        {
            loseElapsed = 0;
            player.position = (Vector2) { screenWidth * 0.5 - player.texture.width * 0.5, screenHeight - player.texture.height - 20 };
            if (player.lives > 0)
            {
                gameState = gameState_win;
                // Todo
            }
            else
            {
                gameState = gameState_start;
                // Todo
            }
        }
    }
}

void Draw()
{
    BeginDrawing();
    ClearBackground(BLACK);
    if (gameState == gameState_start)
    {
        BeginMode2D(camera);
        DrawTextureV(player.texture, player.position, WHITE);
        EndMode2D();
        if (showStartText)
        {
            const int textHalfWidth = MeasureText("Press SPACE To Start!", textSize) * 0.5;
            DrawText("Press SPACE To Start!", screenWidth * 0.5 - textHalfWidth, 40, textSize, WHITE);
        }
    }
    else if (gameState == gameState_play)
    {
        BeginMode2D(camera);
        DrawTextureV(player.texture, player.position, WHITE);
        for (int i = 0; i < MAX_BULLET_COUNT; ++i)
        {
            if (bullets[i].alive)
            {
                DrawCircleV(bullets[i].position, bullets[i].player ? bulletRadiusPlayer : bulletRadiusAlien, WHITE);
            }
        }
        EndMode2D();
        char livesBuffer[16];
        sprintf(livesBuffer, "Lives: %d", player.lives);
        DrawText(livesBuffer, 20, screenHeight - 40, textSize, WHITE);
        char waveBuffer[16];
        sprintf(waveBuffer, "Wave: %d", wave);
        const int waveWidth = MeasureText(waveBuffer, textSize);
        DrawText(waveBuffer, screenWidth - waveWidth - 20, screenHeight - 40, textSize, WHITE);
    }
    else if (gameState == gameState_win)
    {
        BeginMode2D(camera);
        DrawTextureV(player.texture, player.position, WHITE);
        EndMode2D();
        char readyBuffer[32];
        sprintf(readyBuffer, "Ready Wave %d!", wave);
        const int readyHalfWidth = MeasureText(readyBuffer, textSize) * 0.5;
        DrawText(readyBuffer, screenWidth * 0.5 - readyHalfWidth, 40, textSize, WHITE);
        char livesBuffer[16];
        sprintf(livesBuffer, "Lives: %d", player.lives);
        DrawText(livesBuffer, 20, screenHeight - 40, textSize, WHITE);
        char waveBuffer[16];
        sprintf(waveBuffer, "Wave: %d", wave);
        const int waveWidth = MeasureText(waveBuffer, textSize);
        DrawText(waveBuffer, screenWidth - waveWidth - 20, screenHeight - 40, textSize, WHITE);
    }
    else if (gameState == gameState_lose)
    {
        BeginMode2D(camera);
        for (int i = 0; i < MAX_BULLET_COUNT; ++i)
        {
            if (bullets[i].alive)
            {
                DrawCircleV(bullets[i].position, bullets[i].player ? bulletRadiusPlayer : bulletRadiusAlien, WHITE);
            }
        }
        EndMode2D();
        char livesBuffer[16];
        sprintf(livesBuffer, "Lives: %d", player.lives);
        DrawText(livesBuffer, 20, screenHeight - 40, textSize, WHITE);
        char waveBuffer[16];
        sprintf(waveBuffer, "Wave: %d", wave);
        const int waveWidth = MeasureText(waveBuffer, textSize);
        DrawText(waveBuffer, screenWidth - waveWidth - 20, screenHeight - 40, textSize, WHITE);
    }
    EndDrawing();
}

void Terminate()
{
    UnloadTexture(player.texture);
    CloseWindow();
}