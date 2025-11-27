#include "GameBox.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <cmath>
#include <iostream>
#include <vector>
#include <string>

// Game constants
const float GRAVITY = 1200.0f;
const float JUMP_FORCE = -700.0f;
const float MOVE_SPEED = 250.0f;
const int PLAYER_SIZE = 32;
const int TILE_SIZE = 32;

// Camera offset - jarak player dari tepi kiri layar
const int CAMERA_OFFSET_X = 200;

void renderText(SDL_Renderer* renderer, TTF_Font* font, const char* text, int x, int y, SDL_Color color, bool centered) {
    if (!font) return;
    
    SDL_Surface* surface = TTF_RenderText_Solid(font, text, color);
    if (surface) {
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_Rect rect = {
            centered ? x - surface->w / 2 : x,
            y - surface->h / 2,
            surface->w,
            surface->h
        };
        SDL_RenderCopy(renderer, texture, nullptr, &rect);
        SDL_DestroyTexture(texture);
        SDL_FreeSurface(surface);
    }
}

// ========================================
// LEVEL DESIGN - BUAT LEVEL ANDA DI SINI!
// ========================================
// Legend:
// ' ' = empty space
// 'G' = ground/grass block
// 'B' = brick block
// '?' = question block (coin block)
// 'C' = coin
// 'E' = enemy (moving right)
// 'e' = enemy (moving left)
// 'P' = player start position
//
// CATATAN: 
// - Baris harus cukup banyak (minimal 20 baris) agar posisi block dan enemy tepat
// - Level bisa sepanjang yang Anda mau (horizontal)
// - Player spawn di 'P'

std::vector<std::string> mainLevel = {
      "                                                                                                                                                                          ",  // Baris 0
      "                                                                                                                                                                          ",  // Baris 1
      "                                                                                                                                                                          ",  // Baris 2
      "                                                                                                                                                                          ",  // Baris 3
      "                                                                                                                                                                          ",  // Baris 4
      "                                                                                                                                                                          ",  // Baris 5
      "                                                                                                                                                                          ",  // Baris 6
      "                                                                                                                                                                          ",  // Baris 7
      "                                                                                                                                                                          ",  // Baris 8
      "                                                                                                                                                                          ",  // Baris 9
      "                                                                                                                                                                          ",  // Baris 10
      "                                                                                                                                                                          ",  // Baris 11
      "                                                                                                                                                                          ",  // Baris 12
      "                                                                                                                                                                          ",  // Baris 13
      "             ?           ?                      C   C   C                   ?       ?       ?                                                                            ",  // Baris 14
      "                                                                                                                                                                          ",  // Baris 15
      "      C     BBBB         BBBB                       BBB                 BBB   BBB   BBB                                         C                                        ",  // Baris 16
      "    BBBB                                                                                                                    BBBBBBBB                                      ",  // Baris 17
      "                                                                                                                                                                          ",  // Baris 18
      "  P            E              e                 E           e                   E               e                       E                   e                           ",  // Baris 19 - Ground level
};

// Function to parse level from string array
void parseLevelFromArray(const std::vector<std::string>& levelData, 
                        std::vector<Platform>& platforms,
                        std::vector<Coin>& coins,
                        std::vector<Enemy>& enemies,
                        float& playerStartX, float& playerStartY,
                        int windowWidth, int windowHeight) {
    
    // Clear existing data
    platforms.clear();
    coins.clear();
    enemies.clear();
    
    // Calculate level dimensions
    int levelHeight = levelData.size();
    int levelWidth = 0;
    for (const auto& row : levelData) {
        if (row.length() > levelWidth) levelWidth = row.length();
    }
    
    // Ground level
    int groundY = windowHeight - 80;
    
    // Parse from top to bottom
    for (int row = 0; row < levelHeight; row++) {
        const std::string& line = levelData[row];
        
        for (int col = 0; col < line.length(); col++) {
            char tile = line[col];
            int x = col * TILE_SIZE;
            int y = row * TILE_SIZE;
            
            switch (tile) {
                case 'G':  // Ground / Grass
                    platforms.push_back({{x, y, TILE_SIZE, TILE_SIZE}, false, true, false});
                    break;
                    
                case 'B':  // Brick platform
                    platforms.push_back({{x, y, TILE_SIZE, TILE_SIZE}, false, true, false});
                    break;
                    
                case '?':  // Question block (coin block)
                    platforms.push_back({{x, y, TILE_SIZE, TILE_SIZE}, true, false, false});
                    break;
                    
                case 'C':  // Coin
                    coins.push_back({x + TILE_SIZE/2, y + TILE_SIZE/2, false, 0.0f});
                    break;
                    
                case 'E':  // Enemy (moving right)
                    {
                        Enemy e;
                        e.x = static_cast<float>(x);
                        e.y = static_cast<float>(y);
                        e.vx = 50.0f;  // Moving right
                        e.rect = {x, y, 28, 28};
                        e.active = true;
                        enemies.push_back(e);
                    }
                    break;
                    
                case 'e':  // Enemy (moving left)
                    {
                        Enemy e;
                        e.x = static_cast<float>(x);
                        e.y = static_cast<float>(y);
                        e.vx = -50.0f;  // Moving left
                        e.rect = {x, y, 28, 28};
                        e.active = true;
                        enemies.push_back(e);
                    }
                    break;
                    
                case 'P':  // Player start position
                    playerStartX = static_cast<float>(x);
                    playerStartY = static_cast<float>(y);
                    break;
                    
                case ' ':  // Empty space
                default:
                    break;
            }
        }
    }
    
    // Add full ground at bottom
    int levelWidthPixels = levelWidth * TILE_SIZE;
    for (int x = 0; x < levelWidthPixels; x += TILE_SIZE) {
        platforms.push_back({{x, groundY, TILE_SIZE, 80}, false, true, false});
    }
}

bool runGameBox(SDL_Renderer* renderer)
{
    // Load font for UI
    TTF_Font* gameFont = nullptr;
    TTF_Font* smallFont = nullptr;
    const char* font_paths[] = {
          "assets/PressStart2P-Regular.ttf  ",
          "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf  ",
          "C:    Windows    Fonts    arial.ttf  "
    };
    
    for (const char* path : font_paths) {
        gameFont = TTF_OpenFont(path, 20);
        smallFont = TTF_OpenFont(path, 16);
        if (gameFont && smallFont) break;
    }
    
    // Get window size
    int windowWidth, windowHeight;
    SDL_GetRendererOutputSize(renderer, &windowWidth, &windowHeight);
    
    // Player state
    float playerX = 100.0f;
    float playerY = 100.0f;
    float playerStartX = 100.0f;
    float playerStartY = 100.0f;
    float velocityX = 0.0f;
    float velocityY = 0.0f;
    bool isOnGround = false;
    bool facingRight = true;
    
    // Camera position (world coordinate)
    float cameraX = 0.0f;
    
    // Calculate level width
    int levelWidth = 0;
    for (const auto& row : mainLevel) {
        if (row.length() > levelWidth) levelWidth = row.length();
    }
    int levelWidthPixels = levelWidth * TILE_SIZE;
    
    // Game state
    int score = 0;
    int lives = 3;
    bool gameOver = false;
    bool levelComplete = false;
    Uint32 deathTime = 0;
    
    // Animation
    float animPhase = 0.0f;
    
    // Floating texts
    std::vector<FloatingText> floatingTexts;
    
    // Game objects
    std::vector<Platform> platforms;
    std::vector<Coin> coins;
    std::vector<Enemy> enemies;
    
    // Parse level from array
    parseLevelFromArray(mainLevel, platforms, coins, enemies, 
                       playerStartX, playerStartY, windowWidth, windowHeight);
    
    // Set player to start position
    playerX = playerStartX;
    playerY = playerStartY;
    
    SDL_Event event;
    bool running = true;
    Uint32 lastTime = SDL_GetTicks();
    
    std::cout <<   "=== Cat Mario Style Game Started ===  " << std::endl;
    std::cout <<   "Level loaded:   " << platforms.size() <<   " platforms,   "
              << coins.size() <<   " coins,   "
              << enemies.size() <<   " enemies  " << std::endl;
    std::cout <<   "Level width:   " << levelWidthPixels <<   " pixels  " << std::endl;
    std::cout <<   "Controls: A/D = Move, Space/W = Jump  " << std::endl;
    
    while (running)
    {
        // Calculate delta time
        Uint32 currentTime = SDL_GetTicks();
        float deltaTime = (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;
        
        if (deltaTime > 0.05f) deltaTime = 0.05f;
        
        // ------- EVENTS -------
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
                return false;
                
            if (event.type == SDL_KEYDOWN)
            {
                switch (event.key.keysym.sym)
                {
                case SDLK_ESCAPE:
                    return false;
                case SDLK_SPACE:
                case SDLK_UP:
                case SDLK_w:
                    if (isOnGround && !gameOver && !levelComplete) {
                        velocityY = JUMP_FORCE;
                        isOnGround = false;
                    }
                    break;
                case SDLK_r:
                    if (gameOver) {
                        return true;  // Restart
                    }
                    break;
                }
            }
        }
        
        if (!gameOver && !levelComplete) {
            // ------- INPUT -------
            const Uint8* keystate = SDL_GetKeyboardState(NULL);
            velocityX = 0.0f;
            
            if (keystate[SDL_SCANCODE_LEFT] || keystate[SDL_SCANCODE_A]) {
                velocityX = -MOVE_SPEED;
                facingRight = false;
            }
            if (keystate[SDL_SCANCODE_RIGHT] || keystate[SDL_SCANCODE_D]) {
                velocityX = MOVE_SPEED;
                facingRight = true;
            }
            
            // ------- PHYSICS -------
            velocityY += GRAVITY * deltaTime;
            if (velocityY > 600.0f) velocityY = 600.0f;
            
            float oldX = playerX;
            float oldY = playerY;
            
            playerX += velocityX * deltaTime;
            playerY += velocityY * deltaTime;
            
            // ===== BATAS KIRI - Player tidak bisa mundur melewati camera =====
            float minPlayerX = cameraX + 50.0f;  // 50px dari tepi kiri layar
            if (playerX < minPlayerX) {
                playerX = minPlayerX;
            }
            
            // ===== UPDATE CAMERA - Smooth follow player =====
            float targetCameraX = playerX - CAMERA_OFFSET_X;
            if (targetCameraX > cameraX) {
                cameraX = targetCameraX;
            }
            
            // Batas kamera tidak melewati level
            if (cameraX < 0) cameraX = 0;
            if (cameraX > levelWidthPixels - windowWidth) {
                cameraX = levelWidthPixels - windowWidth;
            }
            
            // Check level complete
            if (playerX >= levelWidthPixels - 100) {
                levelComplete = true;
                std::cout <<   "=== LEVEL COMPLETE! ===  " << std::endl;
                std::cout <<   "Final Score:   " << score << std::endl;
            }
            
            SDL_Rect playerRect = {
                static_cast<int>(playerX),
                static_cast<int>(playerY),
                PLAYER_SIZE,
                PLAYER_SIZE
            };
            
            // ===== COLLISION WITH PLATFORMS =====
            isOnGround = false;
            
            for (auto& platform : platforms) {
                bool overlapsX = playerX + PLAYER_SIZE > platform.rect.x && 
                                playerX < platform.rect.x + platform.rect.w;
                bool overlapsY = playerY + PLAYER_SIZE > platform.rect.y && 
                                playerY < platform.rect.y + platform.rect.h;
                
                if (overlapsX && overlapsY) {
                    if (oldY + PLAYER_SIZE <= platform.rect.y && velocityY > 0) {
                        playerY = platform.rect.y - PLAYER_SIZE;
                        velocityY = 0;
                        isOnGround = true;
                    }
                    else if (oldY >= platform.rect.y + platform.rect.h && velocityY < 0) {
                        playerY = platform.rect.y + platform.rect.h;
                        velocityY = 0;
                        
                        // Hit question block from below
                        if (platform.isBreakable && !platform.isHit) {
                            platform.isHit = true;
                            score += 100;
                            std::cout <<   "Block hit! Score:   " << score << std::endl;
                            
                            // Create floating text
                            FloatingText ft;
                            ft.x = platform.rect.x + platform.rect.w / 2.0f;
                            ft.y = platform.rect.y - 10.0f;
                            ft.vy = -100.0f;
                            ft.value = 100;
                            ft.spawnTime = currentTime;
                            ft.active = true;
                            floatingTexts.push_back(ft);
                            
                            // Create coin that pops out
                            Coin newCoin;
                            newCoin.x = platform.rect.x + platform.rect.w / 2;
                            newCoin.y = platform.rect.y - 20;
                            newCoin.collected = false;
                            newCoin.animPhase = 0.0f;
                            coins.push_back(newCoin);
                        }
                    }
                    else if (velocityY >= 0) {
                        if (oldX + PLAYER_SIZE <= platform.rect.x) {
                            playerX = platform.rect.x - PLAYER_SIZE;
                        } else if (oldX >= platform.rect.x + platform.rect.w) {
                            playerX = platform.rect.x + platform.rect.w;
                        }
                    }
                }
            }
            
            playerRect.x = static_cast<int>(playerX);
            playerRect.y = static_cast<int>(playerY);
            
            // Coin collection
            SDL_Rect coinCollect = {playerRect.x + 4, playerRect.y + 4, playerRect.w - 8, playerRect.h - 8};
            for (auto& coin : coins) {
                if (!coin.collected) {
                    SDL_Rect coinRect = {coin.x - 8, coin.y - 8, 16, 16};
                    if (SDL_HasIntersection(&coinCollect, &coinRect)) {
                        coin.collected = true;
                        score += 50;
                        std::cout <<   "Coin collected! Score:   " << score << std::endl;
                        
                        // Create floating text for coin
                        FloatingText ft;
                        ft.x = coin.x;
                        ft.y = coin.y - 10.0f;
                        ft.vy = -80.0f;
                        ft.value = 50;
                        ft.spawnTime = currentTime;
                        ft.active = true;
                        floatingTexts.push_back(ft);
                    }
                }
            }
            
            // Update floating texts
            for (auto& ft : floatingTexts) {
                if (!ft.active) continue;
                
                ft.y += ft.vy * deltaTime;
                ft.vy += 50.0f * deltaTime;
                
                if (currentTime - ft.spawnTime > 1000) {
                    ft.active = false;
                }
            }
            
            // Update enemies
            for (auto& enemy : enemies) {
                if (!enemy.active) continue;
                
                enemy.x += enemy.vx * deltaTime;
                
                enemy.rect.x = static_cast<int>(enemy.x);
                enemy.rect.y = static_cast<int>(enemy.y);
                
                // Bounce off level edges
                if (enemy.x < 0 || enemy.x > levelWidthPixels - enemy.rect.w) {
                    enemy.vx = -enemy.vx;
                }
                
                // Enemy collision with player
                if (SDL_HasIntersection(&playerRect, &enemy.rect)) {
                    if (velocityY > 0 && oldY + PLAYER_SIZE <= enemy.rect.y + 10) {
                        enemy.active = false;
                        velocityY = JUMP_FORCE * 0.5f;
                        score += 200;
                        std::cout <<   "Enemy defeated! Score:   " << score << std::endl;
                        
                        // Floating text for enemy defeat
                        FloatingText ft;
                        ft.x = enemy.rect.x + enemy.rect.w / 2.0f;
                        ft.y = enemy.rect.y - 10.0f;
                        ft.vy = -120.0f;
                        ft.value = 200;
                        ft.spawnTime = currentTime;
                        ft.active = true;
                        floatingTexts.push_back(ft);
                    }
                    else {
                        lives--;
                        std::cout <<   "Hit! Lives remaining:   " << lives << std::endl;
                        
                        if (lives <= 0) {
                            gameOver = true;
                            deathTime = currentTime;
                            std::cout <<   "Game Over! Final Score:   " << score << std::endl;
                        } else {
                            playerX = playerStartX;
                            playerY = playerStartY;
                            velocityX = 0.0f;
                            velocityY = 0.0f;
                            cameraX = 0.0f;
                        }
                    }
                }
            }
            
            // Fall death
            if (playerY > windowHeight + 50) {
                lives--;
                std::cout <<   "Fell! Lives remaining:   " << lives << std::endl;
                
                if (lives <= 0) {
                    gameOver = true;
                    deathTime = currentTime;
                    std::cout <<   "Game Over! Final Score:   " << score << std::endl;
                } else {
                    playerX = playerStartX;
                    playerY = playerStartY;
                    velocityX = 0.0f;
                    velocityY = 0.0f;
                    cameraX = 0.0f;
                }
            }
            
            // Update animation
            if (velocityX != 0 && isOnGround) {
                animPhase += deltaTime * 10.0f;
            }
            
            for (auto& coin : coins) {
                coin.animPhase += deltaTime * 3.0f;
            }
        }
        
        // ======================================
        // ========== RENDERING =================
        // ======================================
        
        // Sky background
        SDL_SetRenderDrawColor(renderer, 92, 148, 252, 255);
        SDL_RenderClear(renderer);
        
        // Clouds with parallax
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        for (int i = 0; i < 5; i++) {
            int cx = static_cast<int>(200 + i * 350 - cameraX * 0.3f);
            int cy = 80 + i * 30;
            if (cx > -100 && cx < windowWidth + 100) {
                SDL_Rect cloud = {cx, cy, 60, 30};
                SDL_RenderFillRect(renderer, &cloud);
            }
        }
        
        // ===== PLATFORMS =====
        int groundY = windowHeight - 80;
        
        for (const auto& platform : platforms) {
            // Cull objects outside camera view
            if (platform.rect.x + platform.rect.w < cameraX - 100) continue;
            if (platform.rect.x > cameraX + windowWidth + 100) continue;
            
            SDL_Rect screenRect = {
                static_cast<int>(platform.rect.x - cameraX),
                platform.rect.y,
                platform.rect.w,
                platform.rect.h
            };
            
            if (platform.isBreakable) {
                // Question block
                if (platform.isHit) {
                    // Used block
                    SDL_SetRenderDrawColor(renderer, 160, 130, 90, 255);
                    SDL_RenderFillRect(renderer, &screenRect);
                    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                    SDL_RenderDrawRect(renderer, &screenRect);
                } else {
                    // Active question block
                    SDL_SetRenderDrawColor(renderer, 243, 168, 59, 255);
                    SDL_RenderFillRect(renderer, &screenRect);
                    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                    SDL_RenderDrawRect(renderer, &screenRect);
                    
                    // Draw   "?  "
                    SDL_Rect qmark = {screenRect.x + 12, screenRect.y + 8, 8, 16};
                    SDL_RenderFillRect(renderer, &qmark);
                }
            } 
            else if (platform.isBrick) {
                // Ground or brick platform
                if (platform.rect.y >= groundY - 5) {
                    // Ground
                    SDL_SetRenderDrawColor(renderer, 123, 192, 67, 255);
                    SDL_Rect grass = {screenRect.x, screenRect.y, screenRect.w, 20};
                    SDL_RenderFillRect(renderer, &grass);
                    
                    SDL_SetRenderDrawColor(renderer, 139, 90, 43, 255);
                    SDL_Rect dirt = {screenRect.x, screenRect.y + 20, screenRect.w, screenRect.h - 20};
                    SDL_RenderFillRect(renderer, &dirt);
                } else {
                    // Floating platform or placed blocks
                    SDL_SetRenderDrawColor(renderer, 184, 111, 80, 255);
                    SDL_RenderFillRect(renderer, &screenRect);
                    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                    SDL_RenderDrawRect(renderer, &screenRect);
                }
            }
        }
        
        // Coins
        for (const auto& coin : coins) {
            if (!coin.collected) {
                if (coin.x < cameraX - 100 || coin.x > cameraX + windowWidth + 100) continue;
                
                float scale = std::abs(std::cos(coin.animPhase));
                int width = static_cast<int>(16 * scale);
                if (width < 4) width = 4;
                
                int screenX = static_cast<int>(coin.x - cameraX);
                SDL_SetRenderDrawColor(renderer, 255, 215, 0, 255);
                SDL_Rect coinRect = {screenX - width / 2, coin.y - 8, width, 16};
                SDL_RenderFillRect(renderer, &coinRect);
                
                SDL_SetRenderDrawColor(renderer, 200, 160, 0, 255);
                SDL_RenderDrawRect(renderer, &coinRect);
            }
        }
        
        // Enemies
        for (const auto& enemy : enemies) {
            if (!enemy.active) continue;
            if (enemy.rect.x < cameraX - 100 || enemy.rect.x > cameraX + windowWidth + 100) continue;
            
            SDL_Rect screenRect = {
                static_cast<int>(enemy.rect.x - cameraX),
                enemy.rect.y,
                enemy.rect.w,
                enemy.rect.h
            };
            
            SDL_SetRenderDrawColor(renderer, 139, 69, 19, 255);
            SDL_RenderFillRect(renderer, &screenRect);
            
            // Eyes
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_Rect eye1 = {screenRect.x + 6, screenRect.y + 8, 6, 6};
            SDL_Rect eye2 = {screenRect.x + 16, screenRect.y + 8, 6, 6};
            SDL_RenderFillRect(renderer, &eye1);
            SDL_RenderFillRect(renderer, &eye2);
            
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_Rect pupil1 = {screenRect.x + 8, screenRect.y + 10, 3, 3};
            SDL_Rect pupil2 = {screenRect.x + 18, screenRect.y + 10, 3, 3};
            SDL_RenderFillRect(renderer, &pupil1);
            SDL_RenderFillRect(renderer, &pupil2);
        }
        
        // Player
        if (!gameOver && !levelComplete) {
            SDL_Rect playerScreenRect = {
                static_cast<int>(playerX - cameraX),
                static_cast<int>(playerY),
                PLAYER_SIZE,
                PLAYER_SIZE
            };
            
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            SDL_Rect body = {playerScreenRect.x + 4, playerScreenRect.y + 8, 24, 16};
            SDL_RenderFillRect(renderer, &body);
            
            SDL_SetRenderDrawColor(renderer, 255, 200, 150, 255);
            SDL_Rect head = {playerScreenRect.x + 8, playerScreenRect.y, 16, 16};
            SDL_RenderFillRect(renderer, &head);
            
            SDL_SetRenderDrawColor(renderer, 200, 0, 0, 255);
            SDL_Rect cap = {playerScreenRect.x + 6, playerScreenRect.y - 4, 20, 8};
            SDL_RenderFillRect(renderer, &cap);
            
            SDL_SetRenderDrawColor(renderer, 0, 0, 200, 255);
            if (isOnGround) {
                int legOffset = static_cast<int>(std::sin(animPhase) * 3);
                SDL_Rect leg1 = {playerScreenRect.x + 8 + legOffset, playerScreenRect.y + 24, 6, 8};
                SDL_Rect leg2 = {playerScreenRect.x + 18 - legOffset, playerScreenRect.y + 24, 6, 8};
                SDL_RenderFillRect(renderer, &leg1);
                SDL_RenderFillRect(renderer, &leg2);
            } else {
                SDL_Rect leg = {playerScreenRect.x + 10, playerScreenRect.y + 24, 12, 8};
                SDL_RenderFillRect(renderer, &leg);
            }
        }
        
        // Floating texts
        if (smallFont) {
            for (const auto& ft : floatingTexts) {
                if (!ft.active) continue;
                if (ft.x < cameraX - 100 || ft.x > cameraX + windowWidth + 100) continue;
                
                Uint32 age = currentTime - ft.spawnTime;
                int alpha = 255 - (age * 255 / 1000);
                if (alpha < 0) alpha = 0;
                
                char scoreStr[16];
                snprintf(scoreStr, sizeof(scoreStr),   "+%d  ", ft.value);
                
                int screenX = static_cast<int>(ft.x - cameraX);
                SDL_Color color = {255, 255, 0, static_cast<Uint8>(alpha)};
                renderText(renderer, smallFont, scoreStr, screenX, static_cast<int>(ft.y), color, true);
            }
        }
        
        // ===== UI =====
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200);
        SDL_Rect scoreBox = {10, 10, 260, 40};
        SDL_RenderFillRect(renderer, &scoreBox);
        
        SDL_SetRenderDrawColor(renderer, 255, 220, 0, 255);
        SDL_RenderDrawRect(renderer, &scoreBox);
        
        if (gameFont) {
            char scoreText[32];
            snprintf(scoreText, sizeof(scoreText),   "SCORE: %d  ", score);
            SDL_Color yellow = {255, 220, 0, 255};
            renderText(renderer, gameFont, scoreText, 18, 28, yellow, false);
        }
        
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200);
        SDL_Rect livesBox = {285, 10, 250, 40};
        SDL_RenderFillRect(renderer, &livesBox);
        
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderDrawRect(renderer, &livesBox);
        
        if (gameFont) {
            SDL_Color white = {255, 255, 255, 255};
            renderText(renderer, gameFont,   "LIVES:  ", 295, 28, white, false);
        }
        
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        for (int i = 0; i < lives; i++) {
            SDL_Rect heart = {415 + i * 32, 19, 18, 18};
            SDL_RenderFillRect(renderer, &heart);
        }
        
        // Level Complete Screen
        if (levelComplete) {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200);
            SDL_Rect overlay = {0, 0, windowWidth, windowHeight};
            SDL_RenderFillRect(renderer, &overlay);
            
            SDL_SetRenderDrawColor(renderer, 0, 139, 0, 255);
            SDL_Rect completeBox = {windowWidth / 2 - 250, windowHeight / 2 - 100, 500, 200};
            SDL_RenderFillRect(renderer, &completeBox);
            
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderDrawRect(renderer, &completeBox);
            
            if (gameFont) {
                SDL_Color white = {255, 255, 255, 255};
                renderText(renderer, gameFont,   "LEVEL COMPLETE!  ", windowWidth / 2, windowHeight / 2 - 50, white, true);
                
                char finalScore[64];
                snprintf(finalScore, sizeof(finalScore),   "SCORE: %d  ", score);
                renderText(renderer, gameFont, finalScore, windowWidth / 2, windowHeight / 2, white, true);
                
                renderText(renderer, smallFont,   "Press ESC to exit  ", windowWidth / 2, windowHeight / 2 + 50, white, true);
            }
        }
        
        // Game Over Screen
        if (gameOver) {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200);
            SDL_Rect overlay = {0, 0, windowWidth, windowHeight};
            SDL_RenderFillRect(renderer, &overlay);
            
            SDL_SetRenderDrawColor(renderer, 139, 0, 0, 255);
            SDL_Rect gameOverBox = {windowWidth / 2 - 250, windowHeight / 2 - 100, 500, 200};
            SDL_RenderFillRect(renderer, &gameOverBox);
            
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderDrawRect(renderer, &gameOverBox);
            
            if (gameFont) {
                SDL_Color white = {255, 255, 255, 255};
                renderText(renderer, gameFont,   "GAME OVER  ", windowWidth / 2, windowHeight / 2 - 50, white, true);
                
                char finalScore[64];
                snprintf(finalScore, sizeof(finalScore),   "FINAL SCORE: %d  ", score);
                renderText(renderer, gameFont, finalScore, windowWidth / 2, windowHeight / 2, white, true);
            }
            
            if (smallFont) {
                SDL_Color white = {255, 255, 255, 255};
                renderText(renderer, smallFont,   "Press R to restart or ESC to exit  ", windowWidth / 2, windowHeight / 2 + 50, white, true);
            }
        }
        
        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }
    
    if (gameFont) TTF_CloseFont(gameFont);
    if (smallFont) TTF_CloseFont(smallFont);
    return true;
}