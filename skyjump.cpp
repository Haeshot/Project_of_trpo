#include "raylib.h"
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>

// ==========================================
// КОНСТАНТЫ ИГРЫ
// ==========================================
const int SCREEN_WIDTH = 400;
const int SCREEN_HEIGHT = 600;
const float GRAVITY = 0.4f;
const float JUMP_FORCE = -10.5f;
const float PLAYER_SPEED = 6.0f;

// ==========================================
// СТРУКТУРЫ ДАННЫХ
// ==========================================
struct Player {
    Vector2 position;
    Vector2 velocity;
    float width, height;
};

// ИСПРАВЛЕНО: Теперь платформа сама знает, какое украшение на ней находится
struct Platform {
    Rectangle rec;
    int decorationType; // Случайное число, генерируемое один раз при создании
};

enum LocationType { LOC_FOREST = 0, LOC_DESERT, LOC_SNOW, LOC_SPACE };

struct VisualParticle {
    Vector2 pos;
    Vector2 vel;
    Color color;
    float size;
    float life;
};

// ==========================================
// ГЛОБАЛЬНЫЕ ПЕРЕМЕННЫЕ
// ==========================================
Player player;
std::vector<Platform> platforms;
int score = 0;
bool gameOver = false;

std::vector<VisualParticle> visualParticles;
LocationType currentVisualLocation = LOC_FOREST;
float visualTime = 0.0f;

// ==========================================
// ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ (ВИЗУАЛ)
// ==========================================

// Определение локации по высоте
LocationType GetLocationByScore(int currentScore) {
    int cycle = (currentScore / 1000) % 4;
    return (LocationType)cycle;
}

// Контрастный цвет текста для интерфейса
Color GetUITextColor(LocationType loc) {
    switch (loc) {
        case LOC_FOREST: return DARKGRAY;
        case LOC_DESERT: return { 60, 20, 20, 255 }; 
        case LOC_SNOW:   return { 20, 40, 80, 255 }; 
        case LOC_SPACE:  return RAYWHITE;       
        default: return BLACK;
    }
}

void UpdateVisualParticles(LocationType loc, int screenWidth, int screenHeight) {
    visualTime += GetFrameTime();
    
    if (loc != currentVisualLocation) {
        visualParticles.clear();
        currentVisualLocation = loc;
    }

    if (visualParticles.size() < 100) {
        VisualParticle p;
        p.pos = { (float)GetRandomValue(0, screenWidth), (float)GetRandomValue(-50, screenHeight) };
        p.life = (float)GetRandomValue(50, 150) / 100.0f;

        if (loc == LOC_FOREST) { 
            p.vel = { (float)GetRandomValue(-10, 10) / 10.0f, (float)GetRandomValue(10, 30) / 10.0f };
            p.color = GetRandomValue(0, 1) ? DARKGREEN : MAROON;
            p.size = (float)GetRandomValue(3, 6);
        } 
        else if (loc == LOC_DESERT) { 
            p.vel = { (float)GetRandomValue(20, 50) / 10.0f, (float)GetRandomValue(-5, 5) / 10.0f };
            p.color = Fade(ORANGE, 0.5f);
            p.size = (float)GetRandomValue(1, 3);
        }
        else if (loc == LOC_SNOW) { 
            p.vel = { (float)GetRandomValue(-20, 20) / 10.0f, (float)GetRandomValue(20, 50) / 10.0f };
            p.color = Fade(WHITE, 0.8f);
            p.size = (float)GetRandomValue(2, 4);
        }
        else if (loc == LOC_SPACE) { 
            p.vel = { 0, (float)GetRandomValue(5, 15) / 10.0f };
            p.color = Fade(WHITE, (float)GetRandomValue(30, 80) / 100.0f);
            p.size = (float)GetRandomValue(1, 3);
        }
        visualParticles.push_back(p);
    }

    for (auto& p : visualParticles) {
        p.pos.x += p.vel.x;
        p.pos.y += p.vel.y;
        
        if (loc == LOC_FOREST) p.pos.x += sinf(visualTime * 2.0f + p.pos.y) * 0.5f;

        if (p.pos.y > screenHeight + 20) p.pos.y = -20;
        if (p.pos.x > screenWidth + 20) p.pos.x = -20;
        if (p.pos.x < -20) p.pos.x = screenWidth + 20;
    }
}

void DrawDynamicBackground(LocationType loc, int currentScore, int screenWidth, int screenHeight) {
    float parallaxOffset = (float)(currentScore % 1000) * 0.2f; 
    
    if (loc == LOC_FOREST) {
        DrawRectangleGradientV(0, 0, screenWidth, screenHeight, {173, 216, 230, 255}, {144, 238, 144, 255});
        DrawCircle(screenWidth / 4, screenHeight + 50 - parallaxOffset * 0.5f, 200, Fade(DARKGREEN, 0.4f));
        DrawCircle(screenWidth, screenHeight + 100 - parallaxOffset * 0.5f, 250, Fade(GREEN, 0.3f));
    } 
    else if (loc == LOC_DESERT) {
        DrawRectangleGradientV(0, 0, screenWidth, screenHeight, {255, 165, 0, 255}, {255, 222, 173, 255});
        DrawCircle(screenWidth - 60, 80 + parallaxOffset * 0.2f, 40, YELLOW);
        DrawEllipse(screenWidth / 2, screenHeight + 50 - parallaxOffset * 0.4f, 300, 150, Fade(DARKBROWN, 0.3f));
    } 
    else if (loc == LOC_SNOW) {
        DrawRectangleGradientV(0, 0, screenWidth, screenHeight, {224, 255, 255, 255}, {240, 248, 255, 255});
        DrawTriangle({(float)screenWidth/2, 200 + parallaxOffset * 0.3f}, 
                     {-100, (float)screenHeight}, 
                     {(float)screenWidth + 100, (float)screenHeight}, Fade(LIGHTGRAY, 0.5f));
    } 
    else if (loc == LOC_SPACE) {
        DrawRectangleGradientV(0, 0, screenWidth, screenHeight, {10, 10, 30, 255}, {0, 0, 0, 255});
        DrawCircleGradient(screenWidth/3, screenHeight/3 + parallaxOffset*0.1f, 150, Fade(PURPLE, 0.2f), BLANK);
        DrawCircle(screenWidth - 50, 150 + parallaxOffset*0.3f, 60, Fade(GRAY, 0.8f));
        DrawCircle(screenWidth - 30, 130 + parallaxOffset*0.3f, 10, Fade(DARKGRAY, 0.8f)); 
    }

    for (const auto& p : visualParticles) {
        if (loc == LOC_FOREST) DrawRectangle(p.pos.x, p.pos.y, p.size, p.size*1.5f, p.color);
        else if (loc == LOC_DESERT) DrawLine(p.pos.x, p.pos.y, p.pos.x + p.size*3, p.pos.y, p.color);
        else DrawCircleV(p.pos, p.size, p.color);
    }
}

// ИСПРАВЛЕНО: Теперь функция принимает готовую платформу и использует запеченный plat.decorationType
void DrawFancyPlatform(Platform plat, LocationType loc) {
    Rectangle rec = plat.rec;
    int hash = plat.decorationType;

    if (loc == LOC_FOREST) {
        DrawRectangleRec(rec, DARKBROWN);
        DrawLine(rec.x, rec.y + rec.height/2, rec.x + rec.width, rec.y + rec.height/2, BLACK); 
        if (hash % 2 == 0) DrawCircle(rec.x + 10, rec.y, 8, GREEN);
        if (hash % 3 == 0) DrawCircle(rec.x + rec.width - 15, rec.y, 10, DARKGREEN);
    } 
    else if (loc == LOC_DESERT) {
        Color terracotta = { 204, 78, 92, 255 };
        DrawRectangleRec(rec, terracotta);
        DrawLine(rec.x + 10, rec.y, rec.x + 15, rec.y + rec.height, MAROON);
        DrawLine(rec.x + rec.width - 20, rec.y, rec.x + rec.width - 15, rec.y + rec.height, MAROON);
        if (hash % 3 == 0) {
            DrawRectangle(rec.x + rec.width/2, rec.y - 20, 8, 20, GREEN); 
            DrawRectangle(rec.x + rec.width/2 + 8, rec.y - 15, 6, 4, GREEN); 
        }
    } 
    else if (loc == LOC_SNOW) {
        DrawRectangleRec(rec, SKYBLUE); 
        DrawRectangle(rec.x, rec.y, rec.width, 3, Fade(WHITE, 0.8f)); 
        if (hash % 2 == 0) DrawEllipse(rec.x + 15, rec.y, 15, 6, WHITE);
        if (hash % 4 == 0) DrawEllipse(rec.x + rec.width - 20, rec.y, 20, 8, WHITE);
    } 
    else if (loc == LOC_SPACE) {
        DrawRectangleRec(rec, GRAY);
        DrawRectangleLinesEx(rec, 2, BLUE);
        DrawCircle(rec.x + 5, rec.y + rec.height/2, 2, LIGHTGRAY);
        DrawCircle(rec.x + rec.width - 5, rec.y + rec.height/2, 2, LIGHTGRAY);
    }
}

void DrawFancyPlayer(Vector2 pos, Vector2 vel, float width, float height, LocationType loc) {
    float cx = pos.x + width / 2;
    float cy = pos.y + height / 2;

    float scaleX = 1.0f;
    float scaleY = 1.0f;
    
    if (vel.y < -3.0f) { 
        scaleX = 0.8f; scaleY = 1.2f;
    } else if (vel.y > 5.0f) { 
        scaleX = 0.9f; scaleY = 1.1f;
        DrawLine(cx - 15, cy - 30, cx - 15, cy - 10, Fade(GRAY, 0.5f));
        DrawLine(cx + 15, cy - 40, cx + 15, cy - 15, Fade(GRAY, 0.5f));
    }

    float currentWidth = width * scaleX;
    float currentHeight = height * scaleY;
    float headRadius = 15.0f * scaleX;
    
    float bodyY = pos.y + height - currentHeight; 
    float headY = bodyY - headRadius + 5; 

    DrawRectangle(cx - currentWidth/2, bodyY, currentWidth, currentHeight, SKYBLUE);
    DrawRectangle(cx - currentWidth/2 + 2, bodyY + currentHeight, 6, 8, DARKBLUE);
    DrawRectangle(cx + currentWidth/2 - 8, bodyY + currentHeight, 6, 8, DARKBLUE);

    if (vel.y < 0) { 
        DrawRectangle(cx - currentWidth/2 - 6, bodyY - 5, 6, 12, SKYBLUE);
        DrawRectangle(cx + currentWidth/2, bodyY - 5, 6, 12, SKYBLUE);
    } else { 
        DrawRectangle(cx - currentWidth/2 - 6, bodyY + 5, 6, 12, SKYBLUE);
        DrawRectangle(cx + currentWidth/2, bodyY + 5, 6, 12, SKYBLUE);
    }

    DrawCircle(cx, headY, headRadius, BEIGE);

    bool isBlinking = std::fmod(visualTime, 2.0f) < 0.15f; 
    float pupilOffsetX = (vel.x > 0) ? 3.0f : ((vel.x < 0) ? -3.0f : 0.0f); 
    
    if (vel.y > 6.0f) { 
        DrawCircle(cx - 5, headY - 2, 6, WHITE);
        DrawCircle(cx + 5, headY - 2, 6, WHITE);
        DrawCircle(cx - 5, headY - 2, 2, BLACK);
        DrawCircle(cx + 5, headY - 2, 2, BLACK);
        DrawEllipse(cx, headY + 7, 4, 6, BLACK); 
    }
    else if (isBlinking) { 
        DrawLine(cx - 8, headY, cx - 2, headY, BLACK);
        DrawLine(cx + 2, headY, cx + 8, headY, BLACK);
    } else { 
        DrawCircle(cx - 5, headY - 2, 4, WHITE);
        DrawCircle(cx + 5, headY - 2, 4, WHITE);
        DrawCircle(cx - 5 + pupilOffsetX, headY - 2, 1.5f, BLACK);
        DrawCircle(cx + 5 + pupilOffsetX, headY - 2, 1.5f, BLACK);
    }

    if (loc == LOC_FOREST) {
        DrawEllipse(cx + 5, headY - headRadius, 8, 4, GREEN);
        DrawLine(cx, headY - headRadius, cx + 5, headY - headRadius, DARKGREEN);
    } 
    else if (loc == LOC_DESERT) {
        DrawRectangle(cx - headRadius - 5, headY - headRadius, headRadius*2 + 10, 4, YELLOW);
        DrawTriangle({cx - 10, headY - headRadius}, {cx + 10, headY - headRadius}, {cx, headY - headRadius - 10}, YELLOW);
        DrawRectangle(cx - 8, headY - 4, 16, 6, BLACK);
    } 
    else if (loc == LOC_SNOW) {
        DrawCircle(cx, headY - headRadius - 2, 6, RED); 
        DrawRectangle(cx - headRadius, headY - headRadius + 2, headRadius*2, 6, RED); 
        DrawCircle(cx - 8, headY + 4, 3, Fade(PINK, 0.6f));
        DrawCircle(cx + 8, headY + 4, 3, Fade(PINK, 0.6f));
        DrawRectangle(cx - currentWidth/2 - 2, bodyY, currentWidth + 4, 6, RED);
        DrawRectangle(cx + 5, bodyY, 6, 15, RED); 
    } 
    else if (loc == LOC_SPACE) {
        DrawCircle(cx, headY, headRadius + 4, Fade(BLUE, 0.3f));
        DrawCircleLines(cx, headY, headRadius + 4, BLUE);
        DrawLine(cx, headY - headRadius - 4, cx, headY - headRadius - 12, GRAY);
        DrawCircle(cx, headY - headRadius - 12, 3, RED);
    }
}

void DrawForegroundEffects(LocationType loc, int screenWidth, int screenHeight) {
    if (loc == LOC_DESERT) {
        DrawRectangle(0, 0, screenWidth, screenHeight, Fade(RED, 0.05f));
        DrawRectangleLinesEx({0, 0, (float)screenWidth, (float)screenHeight}, 15, Fade(ORANGE, 0.1f));
    }
}

// ==========================================
// ОСНОВНАЯ ЛОГИКА ИГРЫ
// ==========================================
void InitGame() {
    player.width = 30;
    player.height = 30;
    player.position = { (float)SCREEN_WIDTH / 2 - player.width / 2, (float)SCREEN_HEIGHT - 100 };
    player.velocity = { 0, 0 };

    score = 0;
    gameOver = false;
    platforms.clear();
    visualParticles.clear();

    // ИСПРАВЛЕНО: Добавляем случайный decorationType при создании стартовой платформы
    platforms.push_back({ { (float)SCREEN_WIDTH / 2 - 40, (float)SCREEN_HEIGHT - 50, 80, 15 }, GetRandomValue(0, 1000) });

    // ИСПРАВЛЕНО: Добавляем случайный decorationType при генерации начального уровня
    float currentY = SCREEN_HEIGHT - 150;
    while (currentY > 0) {
        float x = (float)GetRandomValue(0, SCREEN_WIDTH - 80);
        platforms.push_back({ { x, currentY, 80, 15 }, GetRandomValue(0, 1000) });
        currentY -= GetRandomValue(60, 110);
    }
}

int main() {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Sky Jumper: Perfect Visuals");
    SetTargetFPS(60);

    InitGame();

    while (!WindowShouldClose()) {
        // --- 1. ОБНОВЛЕНИЕ ЛОГИКИ (UPDATE) ---
        if (!gameOver) {
            if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) player.position.x -= PLAYER_SPEED;
            if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) player.position.x += PLAYER_SPEED;

            if (player.position.x > SCREEN_WIDTH) player.position.x = -player.width;
            else if (player.position.x < -player.width) player.position.x = SCREEN_WIDTH;

            player.velocity.y += GRAVITY;
            player.position.y += player.velocity.y;

            if (player.velocity.y > 0) {
                Rectangle playerRec = { player.position.x, player.position.y, player.width, player.height };
                for (auto& plat : platforms) {
                    if (CheckCollisionRecs(playerRec, plat.rec)) {
                        if (player.position.y + player.height - player.velocity.y <= plat.rec.y + 5) {
                            player.velocity.y = JUMP_FORCE;
                        }
                    }
                }
            }

            if (player.position.y < SCREEN_HEIGHT / 2) {
                float diff = (SCREEN_HEIGHT / 2) - player.position.y;
                player.position.y = SCREEN_HEIGHT / 2; 
                score += (int)diff; 

                for (auto& plat : platforms) plat.rec.y += diff;
            }

            platforms.erase(std::remove_if(platforms.begin(), platforms.end(),
                [](const Platform& p) { return p.rec.y > SCREEN_HEIGHT; }), 
                platforms.end());

            float minY = SCREEN_HEIGHT;
            for (auto& plat : platforms) {
                if (plat.rec.y < minY) minY = plat.rec.y;
            }

            // ИСПРАВЛЕНО: Инициализируем decorationType один раз при генерации новых платформ на лету
            while (minY > -50) {
                float newY = minY - GetRandomValue(60, 110);
                float newX = (float)GetRandomValue(0, SCREEN_WIDTH - 80);
                platforms.push_back({ { newX, newY, 80, 15 }, GetRandomValue(0, 1000) });
                minY = newY;
            }

            if (player.position.y > SCREEN_HEIGHT) gameOver = true;

            LocationType currentLocation = GetLocationByScore(score);
            UpdateVisualParticles(currentLocation, SCREEN_WIDTH, SCREEN_HEIGHT);

        } else {
            if (IsKeyPressed(KEY_R)) InitGame();
        }

        // --- 2. ОТРИСОВКА (DRAW) ---
        BeginDrawing();
        ClearBackground(BLANK); 

        LocationType currentLocation = GetLocationByScore(score);

        // Рисуем фон
        DrawDynamicBackground(currentLocation, score, SCREEN_WIDTH, SCREEN_HEIGHT);

        if (!gameOver) {
            // ИСПРАВЛЕНО: Передаем саму структуру платформы в метод отрисовки, хэш стабилен!
            for (const auto& plat : platforms) {
                DrawFancyPlatform(plat, currentLocation);
            }

            DrawFancyPlayer(player.position, player.velocity, player.width, player.height, currentLocation);

            // Текст UI с адаптивным цветом
            Color uiColor = GetUITextColor(currentLocation);
            DrawText(TextFormat("Height: %d", score), 10, 10, 20, uiColor);
            
            const char* locNames[] = {"FOREST", "DESERT", "SNOW", "SPACE"};
            DrawText(TextFormat("Location: %s", locNames[currentLocation]), 10, 35, 20, Fade(uiColor, 0.7f));

            DrawForegroundEffects(currentLocation, SCREEN_WIDTH, SCREEN_HEIGHT);

        } else {
            DrawText("GAME OVER", SCREEN_WIDTH / 2 - MeasureText("GAME OVER", 40) / 2, SCREEN_HEIGHT / 2 - 50, 40, RED);
            DrawText(TextFormat("Final Height: %d", score), SCREEN_WIDTH / 2 - MeasureText(TextFormat("Final Height: %d", score), 20) / 2, SCREEN_HEIGHT / 2, 20, BLACK);
            DrawText("Press 'R' to Restart", SCREEN_WIDTH / 2 - MeasureText("Press 'R' to Restart", 20) / 2, SCREEN_HEIGHT / 2 + 30, 20, DARKGRAY);
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}