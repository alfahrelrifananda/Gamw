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
    // Initialize TTF if not already initialized
    static bool ttfInitialized = false;
    if (!ttfInitialized) {
        if (TTF_Init() == -1) {
            std::cout << "TTF_Init failed: " << TTF_GetError() << std::endl;
        } else {
            ttfInitialized = true;
            std::cout << "TTF initialized successfully" << std::endl;
        }
    }
    
    // Load font for UI
    TTF_Font* gameFont = nullptr;
    TTF_Font* smallFont = nullptr;
    const char* font_paths[] = {
        "assets/PressStart2P-Regular.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",
        "C:\\Windows\\Fonts\\arial.ttf"
    };
    
    for (const char* path : font_paths) {
        gameFont = TTF_OpenFont(path, 20);
        smallFont = TTF_OpenFont(path, 16);
        if (gameFont && smallFont) {
            std::cout << "Fonts loaded successfully from: " << path << std::endl;
            break;
        } else {
            std::cout << "Failed to load font from: " << path << std::endl;
            if (TTF_GetError()) {
                std::cout << "TTF Error: " << TTF_GetError() << std::endl;
            }
        }
    }
    
    if (!gameFont || !smallFont) {
        std::cout << "WARNING: No fonts loaded! Text will not display." << std::endl;
        std::cout << "Make sure font files exist in one of the specified paths." << std::endl;
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
    bool isDying = false;
    Uint32 dyingStartTime = 0;
    float deathFallVelocity = 0.0f;
    int deathCount = 0;
    
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
    
    std::cout << "=== Cat Mario Style Game Started ===" << std::endl;
    std::cout << "Level loaded: " << platforms.size() << " platforms, "
              << coins.size() << " coins, "
              << enemies.size() << " enemies" << std::endl;
    std::cout << "Level width: " << levelWidthPixels << " pixels" << std::endl;
    std::cout << "Controls: A/D = Move, Space/W = Jump" << std::endl;
    
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
                    if (gameOver || levelComplete) {
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
            
            // Only allow input if not dying
            if (!isDying) {
                if (keystate[SDL_SCANCODE_LEFT] || keystate[SDL_SCANCODE_A]) {
                    velocityX = -MOVE_SPEED;
                    facingRight = false;
                }
                if (keystate[SDL_SCANCODE_RIGHT] || keystate[SDL_SCANCODE_D]) {
                    velocityX = MOVE_SPEED;
                    facingRight = true;
                }
            }
            
            // ------- PHYSICS -------
            // Handle death animation
            if (isDying) {
                Uint32 timeSinceDeath = currentTime - dyingStartTime;
                
                // Phase 1: Freeze for 500ms
                if (timeSinceDeath < 500) {
                    velocityX = 0.0f;
                    velocityY = 0.0f;
                }
                // Phase 2: Fall down (500ms - 2000ms)
                else if (timeSinceDeath < 2000) {
                    velocityX = 0.0f;
                    deathFallVelocity += GRAVITY * deltaTime * 0.5f;
                    playerY += deathFallVelocity * deltaTime;
                }
                // Phase 3: Show death screen (2000ms - 4000ms)
                else if (timeSinceDeath < 4000) {
                    // Just wait, death screen is shown
                }
                // Phase 4: Respawn
                else {
                    if (lives <= 0) {
                        gameOver = true;
                        deathTime = currentTime;
                        std::cout << "Game Over! Final Score: " << score << std::endl;
                    } else {
                        // Respawn player
                        isDying = false;
                        playerX = playerStartX;
                        playerY = playerStartY;
                        velocityX = 0.0f;
                        velocityY = 0.0f;
                        deathFallVelocity = 0.0f;
                        cameraX = 0.0f;
                    }
                }
                
                // Skip normal physics when dying
                if (isDying) {
                    // Continue to rendering
                    goto skip_normal_physics;
                }
            }
            
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
                std::cout << "=== LEVEL COMPLETE! ===" << std::endl;
                std::cout << "Final Score: " << score << std::endl;
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
                            std::cout << "Block hit! Score: " << score << std::endl;
                            
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
                        std::cout << "Coin collected! Score: " << score << std::endl;
                        
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
                        std::cout << "Enemy defeated! Score: " << score << std::endl;
                        
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
                        deathCount++;
                        std::cout << "Hit! Lives remaining: " << lives << std::endl;
                        
                        // Start death animation
                        isDying = true;
                        dyingStartTime = currentTime;
                        velocityX = 0.0f;
                        velocityY = 0.0f;
                        deathFallVelocity = 0.0f;
                    }
                }
            }
            
            // Fall death
            if (playerY > windowHeight + 50 && !isDying) {
                lives--;
                deathCount++;
                std::cout << "Fell! Lives remaining: " << lives << std::endl;
                
                // Start death animation
                isDying = true;
                dyingStartTime = currentTime;
                velocityX = 0.0f;
                velocityY = 0.0f;
                deathFallVelocity = 0.0f;
            }
            
            // Update animation
            if (velocityX != 0 && isOnGround) {
                animPhase += deltaTime * 10.0f;
            }
            
            for (auto& coin : coins) {
                coin.animPhase += deltaTime * 3.0f;
            }
        }
        
        skip_normal_physics:
        
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
                // Question block with more texture
                if (platform.isHit) {
                    // Used block - darker with texture
                    SDL_SetRenderDrawColor(renderer, 140, 110, 70, 255);
                    SDL_RenderFillRect(renderer, &screenRect);
                    
                    // Add texture lines
                    SDL_SetRenderDrawColor(renderer, 100, 80, 50, 255);
                    for (int i = 0; i < 4; i++) {
                        SDL_Rect line = {screenRect.x + i * 8, screenRect.y, 4, screenRect.h};
                        SDL_RenderFillRect(renderer, &line);
                    }
                    
                    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                    SDL_RenderDrawRect(renderer, &screenRect);
                } else {
                    // Active question block - animated and textured
                    float bounce = std::sin(currentTime * 0.005f) * 2;
                    SDL_Rect animRect = {screenRect.x, screenRect.y + static_cast<int>(bounce), screenRect.w, screenRect.h};
                    
                    // Gradient effect - light to dark orange
                    SDL_SetRenderDrawColor(renderer, 255, 200, 100, 255);
                    SDL_RenderFillRect(renderer, &animRect);
                    
                    // Top highlight
                    SDL_SetRenderDrawColor(renderer, 255, 230, 150, 255);
                    SDL_Rect highlight = {animRect.x + 2, animRect.y + 2, animRect.w - 4, 8};
                    SDL_RenderFillRect(renderer, &highlight);
                    
                    // Bottom shadow
                    SDL_SetRenderDrawColor(renderer, 200, 140, 60, 255);
                    SDL_Rect shadow = {animRect.x + 2, animRect.y + animRect.h - 10, animRect.w - 4, 8};
                    SDL_RenderFillRect(renderer, &shadow);
                    
                    // Border
                    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                    SDL_RenderDrawRect(renderer, &animRect);
                    
                    // Draw "?" with more detail
                    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                    SDL_Rect qTop = {animRect.x + 10, animRect.y + 6, 12, 8};
                    SDL_RenderFillRect(renderer, &qTop);
                    SDL_Rect qMid = {animRect.x + 14, animRect.y + 12, 8, 6};
                    SDL_RenderFillRect(renderer, &qMid);
                    SDL_Rect qDot = {animRect.x + 14, animRect.y + 20, 6, 6};
                    SDL_RenderFillRect(renderer, &qDot);
                }
            } 
            else if (platform.isBrick) {
                // Ground or brick platform with detailed texture
                if (platform.rect.y >= groundY - 5) {
                    // Ground with grass texture
                    // Grass layer with detail
                    SDL_SetRenderDrawColor(renderer, 123, 192, 67, 255);
                    SDL_Rect grass = {screenRect.x, screenRect.y, screenRect.w, 20};
                    SDL_RenderFillRect(renderer, &grass);
                    
                    // Grass blades
                    SDL_SetRenderDrawColor(renderer, 100, 170, 50, 255);
                    for (int i = 0; i < screenRect.w; i += 4) {
                        SDL_Rect blade = {screenRect.x + i, screenRect.y, 2, 12 + (i % 8)};
                        SDL_RenderFillRect(renderer, &blade);
                    }
                    
                    // Dirt layer with texture
                    SDL_SetRenderDrawColor(renderer, 139, 90, 43, 255);
                    SDL_Rect dirt = {screenRect.x, screenRect.y + 20, screenRect.w, screenRect.h - 20};
                    SDL_RenderFillRect(renderer, &dirt);
                    
                    // Add dirt texture - random dots and patterns
                    SDL_SetRenderDrawColor(renderer, 120, 75, 35, 255);
                    for (int y = 0; y < screenRect.h - 20; y += 6) {
                        for (int x = 0; x < screenRect.w; x += 8) {
                            int dotSize = ((screenRect.x + x + y) % 3) + 1;
                            SDL_Rect dot = {screenRect.x + x + ((x + y) % 4), screenRect.y + 20 + y, dotSize, dotSize};
                            SDL_RenderFillRect(renderer, &dot);
                        }
                    }
                    
                    // Lighter dirt spots
                    SDL_SetRenderDrawColor(renderer, 160, 110, 60, 255);
                    for (int y = 0; y < screenRect.h - 20; y += 8) {
                        for (int x = 0; x < screenRect.w; x += 12) {
                            if ((x + y) % 5 == 0) {
                                SDL_Rect lightSpot = {screenRect.x + x, screenRect.y + 22 + y, 3, 3};
                                SDL_RenderFillRect(renderer, &lightSpot);
                            }
                        }
                    }
                } else {
                    // Floating brick platform with detailed texture
                    // Base brick color
                    SDL_SetRenderDrawColor(renderer, 184, 111, 80, 255);
                    SDL_RenderFillRect(renderer, &screenRect);
                    
                    // Brick pattern - individual bricks
                    int brickW = 16;
                    int brickH = 16;
                    
                    for (int by = 0; by < screenRect.h; by += brickH) {
                        for (int bx = 0; bx < screenRect.w; bx += brickW) {
                            // Offset every other row
                            int offset = (by / brickH) % 2 == 0 ? 0 : brickW / 2;
                            int actualX = screenRect.x + bx + offset;
                            
                            if (actualX >= screenRect.x && actualX < screenRect.x + screenRect.w) {
                                // Brick highlight (top-left)
                                SDL_SetRenderDrawColor(renderer, 210, 140, 100, 255);
                                SDL_Rect highlight = {actualX, screenRect.y + by, brickW - 2, 2};
                                SDL_RenderFillRect(renderer, &highlight);
                                SDL_Rect highlightL = {actualX, screenRect.y + by, 2, brickH - 2};
                                SDL_RenderFillRect(renderer, &highlightL);
                                
                                // Brick shadow (bottom-right)
                                SDL_SetRenderDrawColor(renderer, 140, 80, 60, 255);
                                SDL_Rect shadow = {actualX + 2, screenRect.y + by + brickH - 2, brickW - 2, 2};
                                SDL_RenderFillRect(renderer, &shadow);
                                SDL_Rect shadowR = {actualX + brickW - 2, screenRect.y + by + 2, 2, brickH - 2};
                                SDL_RenderFillRect(renderer, &shadowR);
                                
                                // Mortar lines (dark gray between bricks)
                                SDL_SetRenderDrawColor(renderer, 100, 70, 50, 255);
                                SDL_Rect mortarH = {actualX, screenRect.y + by + brickH - 1, brickW, 1};
                                SDL_RenderFillRect(renderer, &mortarH);
                                SDL_Rect mortarV = {actualX + brickW - 1, screenRect.y + by, 1, brickH};
                                SDL_RenderFillRect(renderer, &mortarV);
                            }
                        }
                    }
                    
                    // Outer border
                    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                    SDL_RenderDrawRect(renderer, &screenRect);
                }
            }
        }
        
        // Coins with better visual
        for (const auto& coin : coins) {
            if (!coin.collected) {
                if (coin.x < cameraX - 100 || coin.x > cameraX + windowWidth + 100) continue;
                
                float scale = std::abs(std::cos(coin.animPhase));
                int width = static_cast<int>(16 * scale);
                if (width < 4) width = 4;
                
                int screenX = static_cast<int>(coin.x - cameraX);
                
                // Gold coin with shine effect
                SDL_SetRenderDrawColor(renderer, 255, 215, 0, 255);
                SDL_Rect coinRect = {screenX - width / 2, coin.y - 8, width, 16};
                SDL_RenderFillRect(renderer, &coinRect);
                
                // Inner darker gold
                SDL_SetRenderDrawColor(renderer, 218, 165, 32, 255);
                SDL_Rect innerCoin = {screenX - width / 2 + 2, coin.y - 6, width > 4 ? width - 4 : 2, 12};
                SDL_RenderFillRect(renderer, &innerCoin);
                
                // Shine highlight
                if (width > 6) {
                    SDL_SetRenderDrawColor(renderer, 255, 250, 205, 255);
                    SDL_Rect shine = {screenX - width / 2 + 2, coin.y - 6, width / 3, 4};
                    SDL_RenderFillRect(renderer, &shine);
                }
                
                // Border
                SDL_SetRenderDrawColor(renderer, 184, 134, 11, 255);
                SDL_RenderDrawRect(renderer, &coinRect);
            }
        }
        
        // Enemies with more detail
        for (const auto& enemy : enemies) {
            if (!enemy.active) continue;
            if (enemy.rect.x < cameraX - 100 || enemy.rect.x > cameraX + windowWidth + 100) continue;
            
            SDL_Rect screenRect = {
                static_cast<int>(enemy.rect.x - cameraX),
                enemy.rect.y,
                enemy.rect.w,
                enemy.rect.h
            };
            
            // Body - brown mushroom/goomba style with texture
            SDL_SetRenderDrawColor(renderer, 139, 69, 19, 255);
            SDL_RenderFillRect(renderer, &screenRect);
            
            // Add texture lines to body
            SDL_SetRenderDrawColor(renderer, 115, 55, 15, 255);
            for (int i = 0; i < 3; i++) {
                SDL_Rect line = {screenRect.x + 4 + i * 7, screenRect.y + 4, 3, screenRect.h - 8};
                SDL_RenderFillRect(renderer, &line);
            }
            
            // Top cap highlight
            SDL_SetRenderDrawColor(renderer, 160, 82, 45, 255);
            SDL_Rect capHighlight = {screenRect.x + 2, screenRect.y + 2, screenRect.w - 4, 6};
            SDL_RenderFillRect(renderer, &capHighlight);
            
            // Eyes with white sclera
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_Rect eye1 = {screenRect.x + 5, screenRect.y + 10, 7, 7};
            SDL_Rect eye2 = {screenRect.x + 16, screenRect.y + 10, 7, 7};
            SDL_RenderFillRect(renderer, &eye1);
            SDL_RenderFillRect(renderer, &eye2);
            
            // Pupils - looking in direction of movement
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            int pupilOffset = enemy.vx > 0 ? 2 : 0;
            SDL_Rect pupil1 = {screenRect.x + 7 + pupilOffset, screenRect.y + 12, 3, 4};
            SDL_Rect pupil2 = {screenRect.x + 18 + pupilOffset, screenRect.y + 12, 3, 4};
            SDL_RenderFillRect(renderer, &pupil1);
            SDL_RenderFillRect(renderer, &pupil2);
            
            // Angry eyebrows
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_Rect brow1 = {screenRect.x + 4, screenRect.y + 8, 8, 2};
            SDL_Rect brow2 = {screenRect.x + 16, screenRect.y + 8, 8, 2};
            SDL_RenderFillRect(renderer, &brow1);
            SDL_RenderFillRect(renderer, &brow2);
            
            // Frown mouth
            SDL_Rect mouth = {screenRect.x + 10, screenRect.y + 20, 8, 2};
            SDL_RenderFillRect(renderer, &mouth);
            
            // Body outline
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderDrawRect(renderer, &screenRect);
        }
        
        // Player with more detail
        if (!gameOver && !levelComplete) {
            SDL_Rect playerScreenRect = {
                static_cast<int>(playerX - cameraX),
                static_cast<int>(playerY),
                PLAYER_SIZE,
                PLAYER_SIZE
            };
            
            // If dying, make player flash or change appearance
            bool shouldDraw = true;
            if (isDying) {
                Uint32 timeSinceDeath = currentTime - dyingStartTime;
                // Flash effect during freeze phase
                if (timeSinceDeath < 500) {
                    shouldDraw = (timeSinceDeath / 100) % 2 == 0;
                }
            }
            
            if (shouldDraw) {
                // Red shirt/body with shading
                SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
                SDL_Rect body = {playerScreenRect.x + 4, playerScreenRect.y + 8, 24, 16};
                SDL_RenderFillRect(renderer, &body);
                
                // Shirt highlight
                SDL_SetRenderDrawColor(renderer, 255, 100, 100, 255);
                SDL_Rect bodyHighlight = {playerScreenRect.x + 6, playerScreenRect.y + 10, 20, 4};
                SDL_RenderFillRect(renderer, &bodyHighlight);
                
                // Buttons on shirt
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                SDL_Rect button1 = {playerScreenRect.x + 14, playerScreenRect.y + 14, 2, 2};
                SDL_Rect button2 = {playerScreenRect.x + 14, playerScreenRect.y + 19, 2, 2};
                SDL_RenderFillRect(renderer, &button1);
                SDL_RenderFillRect(renderer, &button2);
                
                // Skin tone head with shading
                SDL_SetRenderDrawColor(renderer, 255, 200, 150, 255);
                SDL_Rect head = {playerScreenRect.x + 8, playerScreenRect.y, 16, 16};
                SDL_RenderFillRect(renderer, &head);
                
                // Face shadow
                SDL_SetRenderDrawColor(renderer, 230, 180, 130, 255);
                SDL_Rect faceShadow = {playerScreenRect.x + 8, playerScreenRect.y + 10, 16, 6};
                SDL_RenderFillRect(renderer, &faceShadow);
                
                // Eyes
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                int eyeY = playerScreenRect.y + 6;
                if (isDying) eyeY += 2; // Eyes lower when dying
                SDL_Rect eye1 = {playerScreenRect.x + 10, eyeY, 3, 3};
                SDL_Rect eye2 = {playerScreenRect.x + 17, eyeY, 3, 3};
                SDL_RenderFillRect(renderer, &eye1);
                SDL_RenderFillRect(renderer, &eye2);
                
                // Mustache
                SDL_SetRenderDrawColor(renderer, 60, 40, 20, 255);
                SDL_Rect mustache = {playerScreenRect.x + 10, playerScreenRect.y + 10, 12, 3};
                SDL_RenderFillRect(renderer, &mustache);
                
                // Red cap with detail
                SDL_SetRenderDrawColor(renderer, 200, 0, 0, 255);
                SDL_Rect cap = {playerScreenRect.x + 6, playerScreenRect.y - 4, 20, 8};
                SDL_RenderFillRect(renderer, &cap);
                
                // Cap highlight
                SDL_SetRenderDrawColor(renderer, 255, 50, 50, 255);
                SDL_Rect capHighlight = {playerScreenRect.x + 8, playerScreenRect.y - 2, 16, 3};
                SDL_RenderFillRect(renderer, &capHighlight);
                
                // Cap logo "M"
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                SDL_Rect mLogo = {playerScreenRect.x + 14, playerScreenRect.y, 4, 4};
                SDL_RenderFillRect(renderer, &mLogo);
                
                // Blue overalls/legs with detail
                SDL_SetRenderDrawColor(renderer, 0, 0, 200, 255);
                if (isOnGround && !isDying) {
                    int legOffset = static_cast<int>(std::sin(animPhase) * 3);
                    SDL_Rect leg1 = {playerScreenRect.x + 8 + legOffset, playerScreenRect.y + 24, 6, 8};
                    SDL_Rect leg2 = {playerScreenRect.x + 18 - legOffset, playerScreenRect.y + 24, 6, 8};
                    SDL_RenderFillRect(renderer, &leg1);
                    SDL_RenderFillRect(renderer, &leg2);
                    
                    // Shoe highlights
                    SDL_SetRenderDrawColor(renderer, 100, 50, 0, 255);
                    SDL_Rect shoe1 = {playerScreenRect.x + 7 + legOffset, playerScreenRect.y + 29, 8, 3};
                    SDL_Rect shoe2 = {playerScreenRect.x + 17 - legOffset, playerScreenRect.y + 29, 8, 3};
                    SDL_RenderFillRect(renderer, &shoe1);
                    SDL_RenderFillRect(renderer, &shoe2);
                } else {
                    SDL_Rect leg = {playerScreenRect.x + 10, playerScreenRect.y + 24, 12, 8};
                    SDL_RenderFillRect(renderer, &leg);
                    
                    // Shoe
                    SDL_SetRenderDrawColor(renderer, 100, 50, 0, 255);
                    SDL_Rect shoe = {playerScreenRect.x + 9, playerScreenRect.y + 29, 14, 3};
                    SDL_RenderFillRect(renderer, &shoe);
                }
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
                snprintf(scoreStr, sizeof(scoreStr), "+%d", ft.value);
                
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
            snprintf(scoreText, sizeof(scoreText), "SCORE: %d", score);
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
            renderText(renderer, gameFont, "LIVES:", 295, 28, white, false);
        }
        
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        for (int i = 0; i < lives; i++) {
            SDL_Rect heart = {415 + i * 32, 19, 18, 18};
            SDL_RenderFillRect(renderer, &heart);
        }
        
        // Death Screen (Cat Mario style)
        if (isDying) {
            Uint32 timeSinceDeath = currentTime - dyingStartTime;
            
            // Show black screen with death count during phase 3 (2000-4000ms)
            if (timeSinceDeath >= 2000 && timeSinceDeath < 4000) {
                // Black screen overlay
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                SDL_Rect blackScreen = {0, 0, windowWidth, windowHeight};
                SDL_RenderFillRect(renderer, &blackScreen);
                
                // Draw UI box
                SDL_SetRenderDrawColor(renderer, 139, 0, 0, 255);
                SDL_Rect deathBox = {windowWidth / 2 - 300, windowHeight / 2 - 120, 600, 240};
                SDL_RenderFillRect(renderer, &deathBox);
                
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                SDL_RenderDrawRect(renderer, &deathBox);
                
                // Draw inner border
                SDL_Rect innerBorder = {windowWidth / 2 - 290, windowHeight / 2 - 110, 580, 220};
                SDL_RenderDrawRect(renderer, &innerBorder);
                
                if (gameFont) {
                    SDL_Color red = {255, 0, 0, 255};
                    
                    // Show death message
                    char deathMsg[64];
                    const char* deathMessages[] = {
                        "YOU DIED!",
                        "OUCH!",
                        "TRY AGAIN!",
                        "GAME OVER... NOT!",
                        "SO CLOSE!",
                        "KEEP TRYING!"
                    };
                    int msgIndex = deathCount % 6;
                    snprintf(deathMsg, sizeof(deathMsg), "%s", deathMessages[msgIndex]);
                    renderText(renderer, gameFont, deathMsg, windowWidth / 2, windowHeight / 2 - 60, red, true);
                    
                    // Show death count
                    char deathCountText[64];
                    snprintf(deathCountText, sizeof(deathCountText), "Deaths: %d", deathCount);
                    SDL_Color white = {255, 255, 255, 255};
                    renderText(renderer, gameFont, deathCountText, windowWidth / 2, windowHeight / 2 - 10, white, true);
                    
                    // Show current score
                    char currentScore[64];
                    snprintf(currentScore, sizeof(currentScore), "Score: %d", score);
                    renderText(renderer, gameFont, currentScore, windowWidth / 2, windowHeight / 2 + 30, white, true);
                }
                
                if (smallFont) {
                    SDL_Color gray = {200, 200, 200, 255};
                    renderText(renderer, smallFont, "Respawning...", windowWidth / 2, windowHeight / 2 + 80, gray, true);
                }
            }
        }
        
        // Level Complete Screen
        if (levelComplete) {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200);
            SDL_Rect overlay = {0, 0, windowWidth, windowHeight};
            SDL_RenderFillRect(renderer, &overlay);
            
            SDL_SetRenderDrawColor(renderer, 0, 139, 0, 255);
            SDL_Rect completeBox = {windowWidth / 2 - 300, windowHeight / 2 - 150, 600, 300};
            SDL_RenderFillRect(renderer, &completeBox);
            
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderDrawRect(renderer, &completeBox);
            
            // Draw inner border
            SDL_Rect innerBorder = {windowWidth / 2 - 290, windowHeight / 2 - 140, 580, 280};
            SDL_RenderDrawRect(renderer, &innerBorder);
            
            if (gameFont) {
                SDL_Color yellow = {255, 255, 0, 255};
                renderText(renderer, gameFont, "LEVEL COMPLETE!", windowWidth / 2, windowHeight / 2 - 80, yellow, true);
                
                SDL_Color white = {255, 255, 255, 255};
                char finalScore[64];
                snprintf(finalScore, sizeof(finalScore), "SCORE: %d", score);
                renderText(renderer, gameFont, finalScore, windowWidth / 2, windowHeight / 2 - 20, white, true);
                
                char deaths[64];
                snprintf(deaths, sizeof(deaths), "Deaths: %d", deathCount);
                renderText(renderer, gameFont, deaths, windowWidth / 2, windowHeight / 2 + 30, white, true);
            }
            
            if (smallFont) {
                SDL_Color gray = {200, 200, 200, 255};
                renderText(renderer, smallFont, "Press R to restart", windowWidth / 2, windowHeight / 2 + 80, gray, true);
                renderText(renderer, smallFont, "Press ESC to exit", windowWidth / 2, windowHeight / 2 + 110, gray, true);
            }
        }
        
        // Game Over Screen
        if (gameOver) {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200);
            SDL_Rect overlay = {0, 0, windowWidth, windowHeight};
            SDL_RenderFillRect(renderer, &overlay);
            
            SDL_SetRenderDrawColor(renderer, 139, 0, 0, 255);
            SDL_Rect gameOverBox = {windowWidth / 2 - 300, windowHeight / 2 - 150, 600, 300};
            SDL_RenderFillRect(renderer, &gameOverBox);
            
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderDrawRect(renderer, &gameOverBox);
            
            // Draw inner border
            SDL_Rect innerBorder = {windowWidth / 2 - 290, windowHeight / 2 - 140, 580, 280};
            SDL_RenderDrawRect(renderer, &innerBorder);
            
            if (gameFont) {
                SDL_Color red = {255, 50, 50, 255};
                renderText(renderer, gameFont, "GAME OVER", windowWidth / 2, windowHeight / 2 - 80, red, true);
                
                SDL_Color white = {255, 255, 255, 255};
                char finalScore[64];
                snprintf(finalScore, sizeof(finalScore), "FINAL SCORE: %d", score);
                renderText(renderer, gameFont, finalScore, windowWidth / 2, windowHeight / 2 - 20, white, true);
                
                char deaths[64];
                snprintf(deaths, sizeof(deaths), "Total Deaths: %d", deathCount);
                renderText(renderer, gameFont, deaths, windowWidth / 2, windowHeight / 2 + 30, white, true);
            }
            
            if (smallFont) {
                SDL_Color gray = {200, 200, 200, 255};
                renderText(renderer, smallFont, "Press R to restart", windowWidth / 2, windowHeight / 2 + 80, gray, true);
                renderText(renderer, smallFont, "Press ESC to exit", windowWidth / 2, windowHeight / 2 + 110, gray, true);
            }
        }
        
        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }
    
    if (gameFont) TTF_CloseFont(gameFont);
    if (smallFont) TTF_CloseFont(smallFont);
    return true;
}