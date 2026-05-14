#include <raylib.h>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <unordered_map>

// ----------------------------------------------------
// Константы
// ----------------------------------------------------
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const float GRAVITY = 1500.0f;
const float JUMP_FORCE = -650.0f;
const float PLAYER_SPEED = 300.0f;
const float ENEMY_RESPAWN_TIME = 3.0f;

// ----------------------------------------------------
// Состояния игры
// ----------------------------------------------------
enum GameState { 
    STATE_PLAYING, 
    STATE_GAME_OVER, 
    STATE_WIN, 
    STATE_LEVEL_COMPLETE_MENU
};

// ----------------------------------------------------
// Структура облака
// ----------------------------------------------------
struct Cloud {
    Vector2 pos;
    float size;
    float speed;
    Color color;
    Cloud(float x, float y, float s, float sp) {
        pos = { x, y };
        size = s;
        speed = sp;
        color = { 255, 255, 255, 200 };
    }
    void Update(float deltaTime) {
        pos.x += speed * deltaTime;
        if (pos.x + size * 1.5f < 0) pos.x = SCREEN_WIDTH;
        if (pos.x - size * 1.5f > SCREEN_WIDTH) pos.x = -size;
    }
    void Draw() const {
        DrawCircleV(pos, size, color);
        DrawCircleV({ pos.x + size * 0.5f, pos.y - size * 0.3f }, size * 0.7f, color);
        DrawCircleV({ pos.x - size * 0.5f, pos.y - size * 0.2f }, size * 0.6f, color);
    }
};

// ----------------------------------------------------
// Статическая текстура блока травы (без мерцания)
// ----------------------------------------------------
void DrawGrassPlatform(Rectangle rect) {
    int grassHeight = 6;
    // Верхняя трава
    DrawRectangle(rect.x, rect.y, rect.width, grassHeight, GREEN);
    // Постоянные точки на траве (зависят от координат)
    for (int x = rect.x; x < rect.x + rect.width; x += 4) {
        if (((int)(x * 131) % 7) < 3) {
            DrawPixel(x, rect.y + (rand() % grassHeight), DARKGREEN);
        }
    }
    // Боковая земля
    DrawRectangle(rect.x, rect.y + grassHeight, rect.width, rect.height - grassHeight, BROWN);
    // Статические чёрные точки в земле (без мерцания)
    for (int x = rect.x; x < rect.x + rect.width; x += 3) {
        for (int y = rect.y + grassHeight; y < rect.y + rect.height; y += 3) {
            // Псевдослучайный детерминированный выбор точки
            int hash = (int)(x * 37 + y * 131) % 11;
            if (hash < 3) {
                DrawPixel(x, y, BLACK);
            }
        }
    }
    // Нижняя тень
    DrawLineEx({rect.x, rect.y + rect.height - 1}, {rect.x + rect.width, rect.y + rect.height - 1}, 2, DARKGRAY);
}

// ----------------------------------------------------
// Сердечко в стиле Minecraft (пиксельное)
// ----------------------------------------------------
void DrawMinecraftHeart(int x, int y) {
    int pixel = 4;
    const int heart[7][7] = {
        {0,1,0,0,0,1,0},
        {1,1,1,0,1,1,1},
        {1,1,1,1,1,1,1},
        {0,1,1,1,1,1,0},
        {0,0,1,1,1,0,0},
        {0,0,0,1,0,0,0},
        {0,0,0,0,0,0,0}
    };
    for (int i = 0; i < 7; i++) {
        for (int j = 0; j < 7; j++) {
            if (heart[i][j]) {
                DrawRectangle(x + j * pixel, y + i * pixel, pixel, pixel, RED);
                DrawRectangleLines(x + j * pixel, y + i * pixel, pixel, pixel, RED);
            }
        }
    }
}

// ----------------------------------------------------
// Монета
// ----------------------------------------------------
void DrawCoin(int x, int y, int size) {
    Rectangle rect = { (float)x, (float)y, (float)size, (float)size };
    DrawRectangleRec(rect, GOLD);
    DrawRectangleLinesEx(rect, 1, ORANGE);
    DrawCircle(x + size/2, y + size/2, size/3, YELLOW);
    DrawCircle(x + size/3, y + size/3, size/8, WHITE);
}

// ----------------------------------------------------
// Класс Player (добрый)
// ----------------------------------------------------
class Player {
public:
    Rectangle rect;
    Vector2 velocity;
    bool onGround;
    bool facingRight;
    int lives;
    float animationTimer;
    int frameIndex;
    
    Player(float x, float y) {
        rect = { x, y, 32, 32 };
        velocity = { 0, 0 };
        onGround = false;
        facingRight = true;
        lives = 3;
        animationTimer = 0;
        frameIndex = 0;
    }
    
    void Update(float deltaTime, const std::vector<Rectangle>& platforms) {
        // ... (оставляем существующую логику движения)
        float moveInput = 0.0f;
        if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) moveInput = -1.0f;
        if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) moveInput = 1.0f;
        
        // Анимация при движении
        if (moveInput != 0 && onGround) {
            animationTimer += deltaTime;
            if (animationTimer > 0.1f) {
                animationTimer = 0;
                frameIndex = (frameIndex + 1) % 4;
            }
        } else {
            animationTimer = 0;
            frameIndex = 0;
        }
        
        velocity.x = moveInput * PLAYER_SPEED;
        if (moveInput != 0) facingRight = (moveInput > 0);
        if (IsKeyPressed(KEY_SPACE) && onGround) {
            velocity.y = JUMP_FORCE;
            onGround = false;
        }
        velocity.y += GRAVITY * deltaTime;
        rect.x += velocity.x * deltaTime;
        for (const auto& platform : platforms) {
            if (CheckCollisionRecs(rect, platform)) {
                if (velocity.x > 0) rect.x = platform.x - rect.width;
                else if (velocity.x < 0) rect.x = platform.x + platform.width;
            }
        }
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
        // Капюшон/шапка
        DrawRectangle(rect.x, rect.y, rect.width, 10, DARKBLUE);
        
        // Лицо
        DrawRectangle(rect.x, rect.y + 10, rect.width, 12, {255, 224, 189, 255}); // Цвет кожи
        
        // Тело (синяя куртка)
        DrawRectangle(rect.x, rect.y + 22, rect.width, 10, BLUE);
        
        // Глаза
        DrawCircle(rect.x + 9, rect.y + 15, 3, WHITE);
        DrawCircle(rect.x + 23, rect.y + 15, 3, WHITE);
        DrawCircle(rect.x + 9, rect.y + 15, 1.5, BLACK);
        DrawCircle(rect.x + 23, rect.y + 15, 1.5, BLACK);
        
        // Зрачки (смотрят в сторону движения)
        if (facingRight) {
            DrawCircle(rect.x + 10, rect.y + 15, 0.8, WHITE);
            DrawCircle(rect.x + 24, rect.y + 15, 0.8, WHITE);
        } else {
            DrawCircle(rect.x + 8, rect.y + 15, 0.8, WHITE);
            DrawCircle(rect.x + 22, rect.y + 15, 0.8, WHITE);
        }
        
        // Улыбка
        DrawLine(rect.x + 13, rect.y + 20, rect.x + 19, rect.y + 20, BLACK);
        
        // Румянец
        DrawCircle(rect.x + 5, rect.y + 18, 2, {255, 182, 193, 255});
        DrawCircle(rect.x + 27, rect.y + 18, 2, {255, 182, 193, 255});
        
        // Ноги
        if (onGround && animationTimer > 0 && frameIndex % 2 == 0) {
            DrawRectangle(rect.x + 5, rect.y + rect.height - 8, 8, 8, DARKBLUE);
            DrawRectangle(rect.x + 19, rect.y + rect.height - 8, 8, 8, DARKBLUE);
        } else {
            DrawRectangle(rect.x + 5, rect.y + rect.height - 8, 8, 8, BLUE);
            DrawRectangle(rect.x + 19, rect.y + rect.height - 8, 8, 8, BLUE);
        }
    }
};

// ----------------------------------------------------
// Класс Enemy (злой)
// ----------------------------------------------------
class Enemy {
public:
    Rectangle rect;
    float speed;
    bool active;
    float respawnTimer;
    float startX, startY;
    float startSpeed;
    float bounceTimer;
    
    Enemy(float x, float y, float s = 80.0f) {
        startX = x; startY = y; startSpeed = s;
        rect = { x, y, 32, 32 };
        speed = s;
        active = true;
        respawnTimer = 0.0f;
        bounceTimer = 0.0f;
    }
    
    void Update(float deltaTime) {
        if (!active) {
            respawnTimer -= deltaTime;
            if (respawnTimer <= 0.0f) {
                rect.x = startX; rect.y = startY;
                speed = startSpeed;
                active = true;
            }
            return;
        }
        
        bounceTimer += deltaTime * 8;
        rect.x += speed * deltaTime;
        if (rect.x <= 0) { rect.x = 0; speed = -speed; }
        if (rect.x + rect.width >= SCREEN_WIDTH) { rect.x = SCREEN_WIDTH - rect.width; speed = -speed; }
        
        // Эффект подпрыгивания
        float bounce = sinf(bounceTimer) * 2;
        rect.y = startY + bounce;
    }
    
    void Draw() const {
        if (!active) return;
        
        // Тело слизня
        float bounce = sinf(bounceTimer) * 2;
        DrawRectangleRounded({rect.x, rect.y, rect.width, rect.height}, 0.5f, 10, GREEN);
        
        // Глаза
        DrawCircle(rect.x + 10, rect.y + 12, 4, WHITE);
        DrawCircle(rect.x + 22, rect.y + 12, 4, WHITE);
        DrawCircle(rect.x + 9, rect.y + 12, 2, BLACK);
        DrawCircle(rect.x + 21, rect.y + 12, 2, BLACK);
        
        // Злые брови
        DrawLine(rect.x + 6, rect.y + 8, rect.x + 14, rect.y + 10, BLACK);
        DrawLine(rect.x + 18, rect.y + 8, rect.x + 26, rect.y + 10, BLACK);
        
        // Рот
        DrawLine(rect.x + 12, rect.y + 22, rect.x + 20, rect.y + 22, BLACK);
        
        // Блики
        DrawCircle(rect.x + 8, rect.y + 10, 1, WHITE);
        DrawCircle(rect.x + 20, rect.y + 10, 1, WHITE);
    }
    
    void Kill() { 
        if (active) { 
            active = false; 
            respawnTimer = ENEMY_RESPAWN_TIME; 
        } 
    }
};

// ----------------------------------------------------
// Класс Coin
// ----------------------------------------------------
class Coin {
public:
    Rectangle rect;
    bool collected;
    Coin(float x, float y) { rect = { x, y, 20, 20 }; collected = false; }
    void Draw() const { if (!collected) DrawCoin(rect.x, rect.y, 20); }
};

// ----------------------------------------------------
// Класс Goal (флаг)
// ----------------------------------------------------
class Goal {
public:
    Rectangle pole;
    Rectangle flag;
    Goal(float x, float y) {
        pole = { x, y, 4, 48 };
        flag = { x + 4, y, 24, 20 };
    }
    void Draw() const {
        DrawRectangleRec(pole, DARKGRAY);
        DrawRectangleRec(flag, WHITE);
        DrawRectangleLinesEx(flag, 1, BLACK);
    }
};

// ----------------------------------------------------
// Структура уровня
// ----------------------------------------------------
struct Level {
    std::vector<Rectangle> platforms;
    std::vector<Enemy> enemies;
    std::vector<Coin> coins;
    Goal goal;
    float playerStartX, playerStartY;
};

// ----------------------------------------------------
// Создание 5 уровней
// ----------------------------------------------------
std::vector<Level> CreateLevels() {
    std::vector<Level> levels;
    // Уровень 1
    levels.push_back({
        { 
            {0, SCREEN_HEIGHT-40, SCREEN_WIDTH, 40}, 
            {200, SCREEN_HEIGHT-120,100,20}, 
            {400, SCREEN_HEIGHT-180,100,20}, 
            {600, SCREEN_HEIGHT-140,100,20}, 
            {200, SCREEN_HEIGHT-280,100,20}, 
            {500, SCREEN_HEIGHT-350,100,20} },
        { 
            Enemy(300, SCREEN_HEIGHT-68, -60), 
            Enemy(500, SCREEN_HEIGHT-128, -70), 
            Enemy(650, SCREEN_HEIGHT-68, -60) 
        },
        { 
            Coin(250, SCREEN_HEIGHT-148), 
            Coin(450, SCREEN_HEIGHT-208), 
            Coin(650, SCREEN_HEIGHT-168), 
            Coin(150, SCREEN_HEIGHT-308), 
            Coin(550, SCREEN_HEIGHT-378) },
        Goal(540, SCREEN_HEIGHT-390), 100, SCREEN_HEIGHT-128
    });
    // Уровень 2
    levels.push_back({
        { 
            {0, SCREEN_HEIGHT-40, SCREEN_WIDTH, 40}, 
            {150, SCREEN_HEIGHT-150,80,20}, 
            {300, SCREEN_HEIGHT-220,80,20}, 
            {450, SCREEN_HEIGHT-160,80,20}, 
            {600, SCREEN_HEIGHT-250,80,20}, 
            {720, SCREEN_HEIGHT-180,80,20}, 
            {400, SCREEN_HEIGHT-320,80,20}, 
            {550, SCREEN_HEIGHT-380,80,20} },
        { 
            Enemy(320, SCREEN_HEIGHT-188, -60), 
            Enemy(500, SCREEN_HEIGHT-128, -70),
            Enemy(680, SCREEN_HEIGHT-148, -60) },
        { 
            Coin(180, SCREEN_HEIGHT-178),
            Coin(330, SCREEN_HEIGHT-248), 
            Coin(480, SCREEN_HEIGHT-188), 
            Coin(630, SCREEN_HEIGHT-278), 
            Coin(740, SCREEN_HEIGHT-208), 
            Coin(430, SCREEN_HEIGHT-348), 
            Coin(580, SCREEN_HEIGHT-408) },
        Goal(585, SCREEN_HEIGHT-420), 100, SCREEN_HEIGHT-128
    });
    // Уровень 3
    levels.push_back({
        { 
            {0, SCREEN_HEIGHT-40, SCREEN_WIDTH, 40}, 
            {100, SCREEN_HEIGHT-120,60,20}, 
            {200, SCREEN_HEIGHT-200,60,20}, 
            {300, SCREEN_HEIGHT-160,60,20}, 
            {400, SCREEN_HEIGHT-240,60,20}, 
            {500, SCREEN_HEIGHT-180,60,20}, 
            {600, SCREEN_HEIGHT-260,60,20}, 
            {700, SCREEN_HEIGHT-200,60,20}, 
            {250, SCREEN_HEIGHT-300,60,20}, 
            {550, SCREEN_HEIGHT-350,60,20}, 
            {650, SCREEN_HEIGHT-420,60,20} 
        },
        { 
            Enemy(130, SCREEN_HEIGHT-88, -60), 
            Enemy(350, SCREEN_HEIGHT-128, -60), 
            Enemy(530, SCREEN_HEIGHT-148, -70), 
            Enemy(670, SCREEN_HEIGHT-228, -60) 
        },
        { 
            Coin(130, SCREEN_HEIGHT-148), 
            Coin(230, SCREEN_HEIGHT-228), 
            Coin(330, SCREEN_HEIGHT-188), 
            Coin(430, SCREEN_HEIGHT-268), 
            Coin(530, SCREEN_HEIGHT-208), 
            Coin(630, SCREEN_HEIGHT-288), 
            Coin(730, SCREEN_HEIGHT-228), 
            Coin(280, SCREEN_HEIGHT-328), 
            Coin(580, SCREEN_HEIGHT-378), 
            Coin(680, SCREEN_HEIGHT-448) 
        },
        Goal(680, SCREEN_HEIGHT-460), 80, SCREEN_HEIGHT-128
    });
    // Уровень 4
    levels.push_back({
        { 
            {0, SCREEN_HEIGHT-40, SCREEN_WIDTH, 40}, 
            {50, SCREEN_HEIGHT-180,200,20}, 
            {350, SCREEN_HEIGHT-120,100,20}, 
            {550, SCREEN_HEIGHT-220,100,20}, 
            {700, SCREEN_HEIGHT-140,80,20}, 
            {200, SCREEN_HEIGHT-280,80,20}, 
            {450, SCREEN_HEIGHT-340,80,20}, 
            {650, SCREEN_HEIGHT-300,80,20}, 
            {300, SCREEN_HEIGHT-400,80,20} 
        },
        { 
            Enemy(150, SCREEN_HEIGHT-148, -80), 
            Enemy(400, SCREEN_HEIGHT-88, -70), 
            Enemy(600, SCREEN_HEIGHT-188, -90), 
            Enemy(720, SCREEN_HEIGHT-108, -70) 
        },
        { 
            Coin(100, SCREEN_HEIGHT-208), 
            Coin(250, SCREEN_HEIGHT-208), 
            Coin(400, SCREEN_HEIGHT-148), 
            Coin(600, SCREEN_HEIGHT-248), 
            Coin(730, SCREEN_HEIGHT-168), 
            Coin(250, SCREEN_HEIGHT-308), 
            Coin(500, SCREEN_HEIGHT-368), 
            Coin(700, SCREEN_HEIGHT-328), 
            Coin(340, SCREEN_HEIGHT-428) 
        },
        Goal(335, SCREEN_HEIGHT-440), 70, SCREEN_HEIGHT-128
    });
    // Уровень 5
    levels.push_back({
        { 
            {0, SCREEN_HEIGHT-40, SCREEN_WIDTH, 40}, 
            {120, SCREEN_HEIGHT-150,70,20}, 
            {240, SCREEN_HEIGHT-240,70,20}, 
            {360, SCREEN_HEIGHT-180,70,20}, 
            {480, SCREEN_HEIGHT-300,70,20}, 
            {600, SCREEN_HEIGHT-220,70,20}, 
            {720, SCREEN_HEIGHT-350,70,20}, 
            {150, SCREEN_HEIGHT-380,70,20}, 
            {415, SCREEN_HEIGHT-423,70,20}, 
            {650, SCREEN_HEIGHT-500,70,20}, 
            {300, SCREEN_HEIGHT-520,70,20} 
        },
        { 
            Enemy(160, SCREEN_HEIGHT-118, -70), 
            Enemy(400, SCREEN_HEIGHT-148, -80), 
            Enemy(520, SCREEN_HEIGHT-268, -70), 
            Enemy(640, SCREEN_HEIGHT-188, -90), 
            Enemy(750, SCREEN_HEIGHT-318, -70), 
            Enemy(200, SCREEN_HEIGHT-348, -80) 
        },
        {
            Coin(150, SCREEN_HEIGHT-178), 
            Coin(270, SCREEN_HEIGHT-268), 
            Coin(390, SCREEN_HEIGHT-208), 
            Coin(510, SCREEN_HEIGHT-328), 
            Coin(630, SCREEN_HEIGHT-248), 
            Coin(750, SCREEN_HEIGHT-378), 
            Coin(180, SCREEN_HEIGHT-408), 
            Coin(480, SCREEN_HEIGHT-478), 
            Coin(680, SCREEN_HEIGHT-528), 
            Coin(330, SCREEN_HEIGHT-548), 
            Coin(600, SCREEN_HEIGHT-280) 
        },
        Goal(335, SCREEN_HEIGHT-560), 100, SCREEN_HEIGHT-128
    });
    return levels;
}

// ----------------------------------------------------
// Основная функция
// ----------------------------------------------------
int main() {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Super Mario - Minecraft Style");
    SetTargetFPS(60);
    InitAudioDevice();
    srand(time(nullptr));

    std::vector<Level> levels = CreateLevels();
    int currentLevel = 0;
    Player player(levels[currentLevel].playerStartX, levels[currentLevel].playerStartY);
    int levelScore = 0, totalScore = 0;
    GameState gameState = STATE_PLAYING;
    int menuSelection = 0; // 0 = Next Level, 1 = Exit Game
    float menuPulse = 0.0f;

    // Облака
    std::vector<Cloud> clouds;
    for (int i = 0; i < 8; i++) {
        float x = rand() % SCREEN_WIDTH;
        float y = rand() % (SCREEN_HEIGHT / 2);
        float size = 20 + rand() % 35;
        float speed = 10 + rand() % 30;
        clouds.emplace_back(x, y, size, speed);
    }

    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();
        if (deltaTime > 0.1f) deltaTime = 0.1f;

        Level& level = levels[currentLevel];

        if (gameState == STATE_PLAYING) {
            menuPulse += deltaTime;
            player.Update(deltaTime, level.platforms);
            // Враги
            for (auto& enemy : level.enemies) {
                enemy.Update(deltaTime);
                if (enemy.active && CheckCollisionRecs(player.rect, enemy.rect)) {
                    if (player.velocity.y > 0 && player.rect.y + player.rect.height - enemy.rect.y <= 15) {
                        enemy.Kill();
                        player.velocity.y = JUMP_FORCE * 0.6f;
                        levelScore += 20;
                    } else {
                        player.lives--;
                        if (player.lives <= 0) gameState = STATE_GAME_OVER;
                        else {
                            player.rect.x = level.playerStartX;
                            player.rect.y = level.playerStartY;
                            player.velocity = { 0, 0 };
                            player.onGround = true;
                        }
                    }
                }
            }
            // Монеты
            for (auto& coin : level.coins) {
                if (!coin.collected && CheckCollisionRecs(player.rect, coin.rect)) {
                    coin.collected = true;
                    levelScore += 10;
                }
            }
            // Финиш
            if (CheckCollisionRecs(player.rect, level.goal.pole) || CheckCollisionRecs(player.rect, level.goal.flag)) {
                totalScore += levelScore;
                levelScore = 0;
                gameState = STATE_LEVEL_COMPLETE_MENU;
                menuSelection = 0;
            }
        }

        // Обновление облаков
        for (auto& cloud : clouds) cloud.Update(deltaTime);

        // Меню выбора после уровня (стрелки ВВЕРХ/ВНИЗ)
        if (gameState == STATE_LEVEL_COMPLETE_MENU) {
            if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) menuSelection = 0;
            if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) menuSelection = 1;
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
                if (menuSelection == 0) { // Next Level
                    currentLevel++;
                    if (currentLevel >= (int)levels.size()) {
                        gameState = STATE_WIN;
                    } else {
                        player = Player(levels[currentLevel].playerStartX, levels[currentLevel].playerStartY);
                        gameState = STATE_PLAYING;
                    }
                } else { // Exit Game
                    CloseWindow();
                    return 0;
                }
            }
        }

        // ----------------------------------------------------
        // Отрисовка
        // ----------------------------------------------------
        BeginDrawing();
        ClearBackground(SKYBLUE);
        for (const auto& cloud : clouds) cloud.Draw();

        // Платформы с текстурой (без мерцания)
        for (const auto& platform : level.platforms) {
            DrawGrassPlatform(platform);
        }

        // Объекты
        level.goal.Draw();
        for (const auto& coin : level.coins) coin.Draw();
        for (const auto& enemy : level.enemies) enemy.Draw();
        player.Draw();

        // UI
        DrawText(TextFormat("SCORE: %d", totalScore + levelScore), 10, 10, 20, BLACK);
        DrawText(TextFormat("LEVEL: %d/%d", currentLevel + 1, (int)levels.size()), 10, 40, 20, BLACK);
        int coinsCollected = 0;
        for (const auto& coin : level.coins) if (coin.collected) coinsCollected++;
        DrawText(TextFormat("COINS: %d/%d", coinsCollected, (int)level.coins.size()), 10, 70, 20, BLACK);

        // Сердечки
        for (int i = 0; i < player.lives; i++) {
            DrawMinecraftHeart(SCREEN_WIDTH - 45 - i * 35, 15);
        }

        // Меню после уровня
        if (gameState == STATE_LEVEL_COMPLETE_MENU && currentLevel < (int)levels.size()) {
            DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(BLACK, 0.75f));
            float pulse = (sinf(menuPulse * 5) + 1) / 2;
            Color highlight = { 255, 255, 0, (unsigned char)(150 + pulse * 105) };
            DrawText("LEVEL COMPLETE!", SCREEN_WIDTH/2 - 130, SCREEN_HEIGHT/2 - 80, 40, YELLOW);
            const char* nextText = ">> NEXT LEVEL <<";
            const char* exitText = "EXIT GAME";
            if (menuSelection == 0) {
                DrawText(nextText, SCREEN_WIDTH/2 - MeasureText(nextText, 28)/2, SCREEN_HEIGHT/2 - 20, 28, highlight);
                DrawText(exitText, SCREEN_WIDTH/2 - MeasureText(exitText, 24)/2, SCREEN_HEIGHT/2 + 30, 24, WHITE);
            } else {
                DrawText(nextText, SCREEN_WIDTH/2 - MeasureText(nextText, 24)/2, SCREEN_HEIGHT/2 - 20, 24, WHITE);
                DrawText(exitText, SCREEN_WIDTH/2 - MeasureText(exitText, 28)/2, SCREEN_HEIGHT/2 + 30, 28, highlight);
            }
            DrawText("Use  UP / DOWN  to choose,  ENTER / SPACE  to select", SCREEN_WIDTH/2 - 220, SCREEN_HEIGHT - 50, 18, LIGHTGRAY);
        }

        // Победа
        if (gameState == STATE_WIN) {
            DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(BLACK, 0.85f));
            DrawText(TextFormat("YOU WIN! FINAL SCORE: %d", totalScore), SCREEN_WIDTH/2 - 180, SCREEN_HEIGHT/2 - 40, 28, GREEN);
            DrawText("Press R to restart   |   ESC to exit", SCREEN_WIDTH/2 - 180, SCREEN_HEIGHT/2 + 20, 20, WHITE);
            if (IsKeyPressed(KEY_R)) {
                currentLevel = 0; totalScore = 0; levelScore = 0;
                player = Player(levels[0].playerStartX, levels[0].playerStartY);
                player.lives = 3;
                for (auto& lvl : levels) {
                    for (auto& coin : lvl.coins) coin.collected = false;
                    for (auto& enemy : lvl.enemies) {
                        enemy.active = true;
                        enemy.respawnTimer = 0;
                        enemy.rect.x = enemy.startX;
                        enemy.rect.y = enemy.startY;
                        enemy.speed = enemy.startSpeed;
                    }
                }
                gameState = STATE_PLAYING;
            } else if (IsKeyPressed(KEY_ESCAPE)) CloseWindow();
        }

        // Поражение
        if (gameState == STATE_GAME_OVER) {
            DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(BLACK, 0.85f));
            DrawText("GAME OVER", SCREEN_WIDTH/2 - 80, SCREEN_HEIGHT/2 - 40, 40, RED);
            DrawText("Press R to restart   |   ESC to exit", SCREEN_WIDTH/2 - 180, SCREEN_HEIGHT/2 + 20, 20, WHITE);
            if (IsKeyPressed(KEY_R)) {
                currentLevel = 0; totalScore = 0; levelScore = 0;
                player = Player(levels[0].playerStartX, levels[0].playerStartY);
                player.lives = 3;
                for (auto& lvl : levels) {
                    for (auto& coin : lvl.coins) coin.collected = false;
                    for (auto& enemy : lvl.enemies) {
                        enemy.active = true;
                        enemy.respawnTimer = 0;
                        enemy.rect.x = enemy.startX;
                        enemy.rect.y = enemy.startY;
                        enemy.speed = enemy.startSpeed;
                    }
                }
                gameState = STATE_PLAYING;
            } else if (IsKeyPressed(KEY_ESCAPE)) CloseWindow();
        }

        EndDrawing();
    }

    CloseAudioDevice();
    CloseWindow();
    return 0;
}