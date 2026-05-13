#include <raylib.h>
#include <vector>
#include <algorithm>

// ----------------------------------------------------
// Константы
// ----------------------------------------------------
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const float GRAVITY = 1500.0f;
const float JUMP_FORCE = -550.0f;
const float PLAYER_SPEED = 300.0f;

// ----------------------------------------------------
// Состояния игры
// ----------------------------------------------------
enum GameState { STATE_PLAYING, STATE_GAME_OVER, STATE_WIN };

// ----------------------------------------------------
// Класс Player (Марио)
// ----------------------------------------------------
class Player {
public:
    Rectangle rect;
    Vector2 velocity;
    bool onGround;
    bool facingRight;
    int lives;

    Player(float x, float y) {
        rect = { x, y, 32, 32 };
        velocity = { 0, 0 };
        onGround = false;
        facingRight = true;
        lives = 3;
    }

    void Update(float deltaTime, const std::vector<Rectangle>& platforms) {
        // Горизонтальное управление
        float moveInput = 0.0f;
        if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) moveInput = -1.0f;
        if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) moveInput = 1.0f;
        velocity.x = moveInput * PLAYER_SPEED;
        if (moveInput != 0) facingRight = (moveInput > 0);

        // Прыжок
        if ((IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) && onGround) {
            velocity.y = JUMP_FORCE;
            onGround = false;
        }

        // Гравитация
        velocity.y += GRAVITY * deltaTime;

        // Временное перемещение по X с коллизиями
        rect.x += velocity.x * deltaTime;
        for (const auto& platform : platforms) {
            if (CheckCollisionRecs(rect, platform)) {
                if (velocity.x > 0) rect.x = platform.x - rect.width;
                else if (velocity.x < 0) rect.x = platform.x + platform.width;
            }
        }

        // Временное перемещение по Y с коллизиями
        rect.y += velocity.y * deltaTime;
        onGround = false;
        for (const auto& platform : platforms) {
            if (CheckCollisionRecs(rect, platform)) {
                if (velocity.y > 0) {
                    rect.y = platform.y - rect.height;
                    velocity.y = 0;
                    onGround = true;
                } else if (velocity.y < 0) {
                    rect.y = platform.y + platform.height;
                    velocity.y = 0;
                }
            }
        }

        // Границы экрана
        if (rect.x < 0) rect.x = 0;
        if (rect.x + rect.width > SCREEN_WIDTH) rect.x = SCREEN_WIDTH - rect.width;
        if (rect.y + rect.height > SCREEN_HEIGHT) {
            lives--;
            rect.x = 100; rect.y = SCREEN_HEIGHT - 128;
            velocity = { 0, 0 };
            onGround = true;
        }
        if (rect.y < 0) rect.y = 0;
    }

    void Draw() const {
        Color playerColor = facingRight ? GREEN : DARKGREEN;
        DrawRectangleRec(rect, playerColor);
        // Рисуем глаза для выразительности
        DrawCircle(rect.x + rect.width - 8, rect.y + 10, 3, WHITE);
        DrawCircle(rect.x + 8, rect.y + 10, 3, WHITE);
    }
};

// ----------------------------------------------------
// Класс Enemy (Гумба)
// ----------------------------------------------------
class Enemy {
public:
    Rectangle rect;
    float speed;
    bool active;

    Enemy(float x, float y, float s = 80.0f) {
        rect = { x, y, 28, 28 };
        speed = s;
        active = true;
    }

    void Update(float deltaTime) {
        if (!active) return;
        rect.x += speed * deltaTime;
    }

    void Draw() const {
        if (active) {
            DrawRectangleRec(rect, RED);
            DrawRectangleLinesEx(rect, 1, RED);
        }
    }
};

// ----------------------------------------------------
// Класс Coin (монета)
// ----------------------------------------------------
class Coin {
public:
    Rectangle rect;
    bool collected;

    Coin(float x, float y) {
        rect = { x, y, 20, 20 };
        collected = false;
    }

    void Draw() const {
        if (!collected) {
            DrawRectangleRec(rect, GOLD);
            DrawCircle(rect.x + rect.width/2, rect.y + rect.height/2, 6, YELLOW);
        }
    }
};

// ----------------------------------------------------
// Класс Goal (финишный флаг)
// ----------------------------------------------------
class Goal {
public:
    Rectangle rect;

    Goal(float x, float y) {
        rect = { x, y, 20, 48 };
    }

    void Draw() const {
        DrawRectangleRec(rect, BLUE);
        DrawRectangle(rect.x + 5, rect.y - 10, 4, 20, RED); // древко флага
    }
};

// ----------------------------------------------------
// Основная функция
// ----------------------------------------------------
int main() {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Super Mario Style - Raylib");
    SetTargetFPS(60);
    InitAudioDevice();

    // Платформы
    std::vector<Rectangle> platforms = {
        { 0, SCREEN_HEIGHT - 40, SCREEN_WIDTH, 40 },          // земля
        { 200, SCREEN_HEIGHT - 120, 100, 20 },
        { 400, SCREEN_HEIGHT - 180, 100, 20 },
        { 600, SCREEN_HEIGHT - 140, 100, 20 },
        { 100, SCREEN_HEIGHT - 280, 100, 20 },
        { 500, SCREEN_HEIGHT - 350, 100, 20 },
        { 700, SCREEN_HEIGHT - 450, 100, 20 }
    };

    // Игровые объекты
    Player player(100, SCREEN_HEIGHT - 128);
    std::vector<Enemy> enemies = {
        Enemy(300, SCREEN_HEIGHT - 68, -60),
        Enemy(500, SCREEN_HEIGHT - 128, -70),
        Enemy(650, SCREEN_HEIGHT - 68, -60)
    };
    std::vector<Coin> coins = {
        Coin(250, SCREEN_HEIGHT - 148),
        Coin(450, SCREEN_HEIGHT - 208),
        Coin(650, SCREEN_HEIGHT - 168),
        Coin(150, SCREEN_HEIGHT - 308),
        Coin(550, SCREEN_HEIGHT - 378),
        Coin(750, SCREEN_HEIGHT - 478)
    };
    Goal goal(760, SCREEN_HEIGHT - 88);

    GameState gameState = STATE_PLAYING;
    int score = 0;
    int totalCoins = (int)coins.size();

    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();
        if (deltaTime > 0.1f) deltaTime = 0.1f;

        if (gameState == STATE_PLAYING) {
            player.Update(deltaTime, platforms);

            // Обновление врагов и столкновения
            for (auto& enemy : enemies) {
                enemy.Update(deltaTime);
                if (enemy.active && CheckCollisionRecs(player.rect, enemy.rect)) {
                    if (player.velocity.y > 0 && player.rect.y + player.rect.height - enemy.rect.y <= 15) {
                        enemy.active = false;
                        player.velocity.y = JUMP_FORCE * 0.6f;
                        score += 20;
                    } else {
                        player.lives--;
                        if (player.lives <= 0) {
                            gameState = STATE_GAME_OVER;
                        } else {
                            player.rect.x = 100;
                            player.rect.y = SCREEN_HEIGHT - 128;
                            player.velocity = { 0, 0 };
                            player.onGround = true;
                        }
                    }
                }
            }

            // Сбор монет
            for (auto& coin : coins) {
                if (!coin.collected && CheckCollisionRecs(player.rect, coin.rect)) {
                    coin.collected = true;
                    score += 10;
                }
            }

            // Проверка победы
            if (CheckCollisionRecs(player.rect, goal.rect) || score >= totalCoins * 10) {
                gameState = STATE_WIN;
            }
        }

        // Отрисовка
        BeginDrawing();
        ClearBackground(SKYBLUE);

        // Платформы
        for (const auto& platform : platforms) {
            DrawRectangleRec(platform, DARKGRAY);
            DrawRectangleLinesEx(platform, 1, BLACK);
        }

        // Объекты
        goal.Draw();
        for (const auto& coin : coins) coin.Draw();
        for (const auto& enemy : enemies) enemy.Draw();
        player.Draw();

        // UI
        DrawText(TextFormat("SCORE: %d", score), 10, 10, 20, BLACK);
        DrawText(TextFormat("COINS: %d/%d", score / 10, totalCoins), 10, 40, 20, BLACK);
        for (int i = 0; i < player.lives; i++) {
            DrawRectangle(SCREEN_WIDTH - 40 * (i + 1), 15, 30, 20, RED);
        }

        // Конец игры
        if (gameState == STATE_WIN) {
            DrawText("YOU WIN!", SCREEN_WIDTH/2 - 60, SCREEN_HEIGHT/2, 40, GREEN);
            DrawText("Press R to restart", SCREEN_WIDTH/2 - 90, SCREEN_HEIGHT/2 + 40, 20, DARKGREEN);
            if (IsKeyPressed(KEY_R)) {
                gameState = STATE_PLAYING;
                player.lives = 3;
                score = 0;
                player.rect = { 100, SCREEN_HEIGHT - 128, 32, 32 };
                player.velocity = { 0, 0 };
                player.onGround = true;
                for (auto& coin : coins) coin.collected = false;
                for (auto& enemy : enemies) enemy.active = true;
            }
        } else if (gameState == STATE_GAME_OVER) {
            DrawText("GAME OVER", SCREEN_WIDTH/2 - 70, SCREEN_HEIGHT/2, 40, RED);
            DrawText("Press R to restart", SCREEN_WIDTH/2 - 90, SCREEN_HEIGHT/2 + 40, 20, RED);
            if (IsKeyPressed(KEY_R)) {
                gameState = STATE_PLAYING;
                player.lives = 3;
                score = 0;
                player.rect = { 100, SCREEN_HEIGHT - 128, 32, 32 };
                player.velocity = { 0, 0 };
                player.onGround = true;
                for (auto& coin : coins) coin.collected = false;
                for (auto& enemy : enemies) enemy.active = true;
            }
        }

        EndDrawing();
    }

    CloseAudioDevice();
    CloseWindow();
    return 0;
}