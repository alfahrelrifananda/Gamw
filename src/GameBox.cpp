#include "GameBox.h"
#include <SDL2/SDL.h>
#include <cmath>
#include <iostream>
#include <vector>

// Game constants
const float GRAVITY = 800.0f;
const float JUMP_FORCE = -400.0f;
const float MOVE_SPEED = 200.0f;
const int PLAYER_SIZE = 32;

bool runGameBox(SDL_Renderer* renderer)
{
    // Get window size
    int windowWidth, windowHeight;
    SDL_GetRendererOutputSize(renderer, &windowWidth, &windowHeight);
    
    // Player state
    float playerX = 100.0f;
    float playerY = 300.0f;
    float velocityX = 0.0f;
    float velocityY = 0.0f;
    bool isOnGround = false;
    bool facingRight = true;
    
    // Game state
    int score = 0;
    int lives = 3;
    bool gameOver = false;
    
    // Animation
    float animPhase = 0.0f;
    
    // Create platforms (Mario-style)
    std::vector<Platform> platforms;
    
    // Ground platforms
    for (int x = 0; x < windowWidth; x += 64) {
        Platform p;
        p.rect = {x, windowHeight - 64, 64, 64};
        p.isBreakable = false;
        p.isBrick = true;
        platforms.push_back(p);
    }
    
    // Floating platforms
    platforms.push_back({{250, 400, 128, 32}, false, true});
    platforms.push_back({{450, 300, 128, 32}, false, true});
    platforms.push_back({{700, 350, 96, 32}, false, true});
    platforms.push_back({{900, 250, 128, 32}, false, true});
    
    // Question blocks
    platforms.push_back({{300, 250, 32, 32}, true, false});
    platforms.push_back({{500, 200, 32, 32}, true, false});
    platforms.push_back({{800, 180, 32, 32}, true, false});
    
    // Coins
    std::vector<Coin> coins;
    coins.push_back({350, 200, false, 0.0f});
    coins.push_back({550, 150, false, 0.0f});
    coins.push_back({750, 300, false, 0.0f});
    coins.push_back({950, 200, false, 0.0f});
    
    // Simple enemies (Goomba-like)
    std::vector<Enemy> enemies;
    Enemy e1 = {400.0f, windowHeight - 96.0f, 50.0f, {0, 0, 28, 28}, true};
    Enemy e2 = {650.0f, windowHeight - 96.0f, -60.0f, {0, 0, 28, 28}, true};
    enemies.push_back(e1);
    enemies.push_back(e2);
    
    SDL_Event event;
    bool running = true;
    Uint32 lastTime = SDL_GetTicks();
    
    while (running && !gameOver)
    {
        // Calculate delta time
        Uint32 currentTime = SDL_GetTicks();
        float deltaTime = (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;
        
        if (deltaTime > 0.1f) deltaTime = 0.1f; // Cap delta time
        
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
                    if (isOnGround) {
                        velocityY = JUMP_FORCE;
                        isOnGround = false;
                    }
                    break;
                }
            }
        }
        
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
        // Apply gravity
        velocityY += GRAVITY * deltaTime;
        
        // Update position
        playerX += velocityX * deltaTime;
        playerY += velocityY * deltaTime;
        
        // Screen wrap
        if (playerX < -PLAYER_SIZE) playerX = windowWidth;
        if (playerX > windowWidth) playerX = -PLAYER_SIZE;
        
        // Create player rect
        SDL_Rect playerRect = {
            static_cast<int>(playerX),
            static_cast<int>(playerY),
            PLAYER_SIZE,
            PLAYER_SIZE
        };
        
        // Check ground collision
        isOnGround = false;
        
        for (auto& platform : platforms) {
            // Simple AABB collision
            if (playerRect.x + playerRect.w > platform.rect.x &&
                playerRect.x < platform.rect.x + platform.rect.w &&
                playerRect.y + playerRect.h > platform.rect.y &&
                playerRect.y < platform.rect.y + platform.rect.h)
            {
                // Landing on top
                if (velocityY > 0 && playerY + PLAYER_SIZE - velocityY * deltaTime <= platform.rect.y) {
                    playerY = platform.rect.y - PLAYER_SIZE;
                    velocityY = 0;
                    isOnGround = true;
                }
                // Hitting from below (question block)
                else if (velocityY < 0 && playerY - velocityY * deltaTime >= platform.rect.y + platform.rect.h) {
                    playerY = platform.rect.y + platform.rect.h;
                    velocityY = 0;
                    
                    if (platform.isBreakable) {
                        score += 100;
                        std::cout << "Coin! Score: " << score << std::endl;
                    }
                }
            }
        }
        
        // Coin collection
        SDL_Rect coinCollect = {playerRect.x + 4, playerRect.y + 4, playerRect.w - 8, playerRect.h - 8};
        for (auto& coin : coins) {
            if (!coin.collected) {
                SDL_Rect coinRect = {coin.x - 8, coin.y - 8, 16, 16};
                if (SDL_HasIntersection(&coinCollect, &coinRect)) {
                    coin.collected = true;
                    score += 50;
                    std::cout << "Coin collected! Score: " << score << std::endl;
                }
            }
        }
        
        // Update enemies
        for (auto& enemy : enemies) {
            if (!enemy.active) continue;
            
            enemy.x += enemy.vx * deltaTime;
            enemy.rect.x = static_cast<int>(enemy.x);
            enemy.rect.y = static_cast<int>(enemy.y);
            
            // Bounce off walls
            if (enemy.x < 0 || enemy.x > windowWidth - enemy.rect.w) {
                enemy.vx = -enemy.vx;
            }
            
            // Enemy collision with player
            if (SDL_HasIntersection(&playerRect, &enemy.rect)) {
                // Stomp on enemy
                if (velocityY > 0 && playerY + PLAYER_SIZE - velocityY * deltaTime <= enemy.rect.y + 10) {
                    enemy.active = false;
                    velocityY = JUMP_FORCE * 0.5f;
                    score += 200;
                    std::cout << "Enemy defeated! Score: " << score << std::endl;
                }
                // Hit by enemy
                else {
                    lives--;
                    std::cout << "Hit! Lives remaining: " << lives << std::endl;
                    
                    if (lives <= 0) {
                        gameOver = true;
                        std::cout << "Game Over! Final Score: " << score << std::endl;
                    } else {
                        // Respawn
                        playerX = 100.0f;
                        playerY = 300.0f;
                        velocityX = 0.0f;
                        velocityY = 0.0f;
                    }
                }
            }
        }
        
        // Fall death
        if (playerY > windowHeight + 100) {
            lives--;
            std::cout << "Fell! Lives remaining: " << lives << std::endl;
            
            if (lives <= 0) {
                gameOver = true;
                std::cout << "Game Over! Final Score: " << score << std::endl;
            } else {
                playerX = 100.0f;
                playerY = 300.0f;
                velocityX = 0.0f;
                velocityY = 0.0f;
            }
        }
        
        // Update animation
        if (velocityX != 0 && isOnGround) {
            animPhase += deltaTime * 10.0f;
        }
        
        for (auto& coin : coins) {
            coin.animPhase += deltaTime * 3.0f;
        }
        
        // ------- RENDER -------
        // Sky background
        SDL_SetRenderDrawColor(renderer, 92, 148, 252, 255);
        SDL_RenderClear(renderer);
        
        // Clouds (simple)
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        for (int i = 0; i < 3; i++) {
            int cx = 200 + i * 300;
            int cy = 80 + i * 40;
            SDL_Rect cloud = {cx, cy, 60, 30};
            SDL_RenderFillRect(renderer, &cloud);
        }
        
        // Platforms
        for (const auto& platform : platforms) {
            if (platform.isBreakable) {
                // Question block (yellow with ?)
                SDL_SetRenderDrawColor(renderer, 243, 168, 59, 255);
                SDL_RenderFillRect(renderer, &platform.rect);
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                SDL_RenderDrawRect(renderer, &platform.rect);
            } else if (platform.isBrick) {
                // Brown brick
                SDL_SetRenderDrawColor(renderer, 184, 111, 80, 255);
                SDL_RenderFillRect(renderer, &platform.rect);
                SDL_SetRenderDrawColor(renderer, 139, 90, 43, 255);
                SDL_RenderDrawRect(renderer, &platform.rect);
                
                // Brick lines
                int midX = platform.rect.x + platform.rect.w / 2;
                int midY = platform.rect.y + platform.rect.h / 2;
                SDL_RenderDrawLine(renderer, platform.rect.x, midY, 
                                 platform.rect.x + platform.rect.w, midY);
            }
        }
        
        // Coins
        for (const auto& coin : coins) {
            if (!coin.collected) {
                float scale = std::abs(std::cos(coin.animPhase));
                int width = static_cast<int>(16 * scale);
                
                SDL_SetRenderDrawColor(renderer, 255, 215, 0, 255);
                SDL_Rect coinRect = {coin.x - width / 2, coin.y - 8, width, 16};
                SDL_RenderFillRect(renderer, &coinRect);
                
                SDL_SetRenderDrawColor(renderer, 200, 160, 0, 255);
                SDL_RenderDrawRect(renderer, &coinRect);
            }
        }
        
        // Enemies
        for (const auto& enemy : enemies) {
            if (!enemy.active) continue;
            
            // Brown goomba
            SDL_SetRenderDrawColor(renderer, 139, 69, 19, 255);
            SDL_RenderFillRect(renderer, &enemy.rect);
            
            // Eyes
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_Rect eye1 = {enemy.rect.x + 6, enemy.rect.y + 8, 6, 6};
            SDL_Rect eye2 = {enemy.rect.x + 16, enemy.rect.y + 8, 6, 6};
            SDL_RenderFillRect(renderer, &eye1);
            SDL_RenderFillRect(renderer, &eye2);
            
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_Rect pupil1 = {enemy.rect.x + 8, enemy.rect.y + 10, 3, 3};
            SDL_Rect pupil2 = {enemy.rect.x + 18, enemy.rect.y + 10, 3, 3};
            SDL_RenderFillRect(renderer, &pupil1);
            SDL_RenderFillRect(renderer, &pupil2);
        }
        
        // Player (Mario-like)
        // Body
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_Rect body = {playerRect.x + 4, playerRect.y + 8, 24, 16};
        SDL_RenderFillRect(renderer, &body);
        
        // Head
        SDL_SetRenderDrawColor(renderer, 255, 200, 150, 255);
        SDL_Rect head = {playerRect.x + 8, playerRect.y, 16, 16};
        SDL_RenderFillRect(renderer, &head);
        
        // Cap
        SDL_SetRenderDrawColor(renderer, 200, 0, 0, 255);
        SDL_Rect cap = {playerRect.x + 6, playerRect.y - 4, 20, 8};
        SDL_RenderFillRect(renderer, &cap);
        
        // Legs (animated when moving)
        SDL_SetRenderDrawColor(renderer, 0, 0, 200, 255);
        if (isOnGround) {
            int legOffset = static_cast<int>(std::sin(animPhase) * 3);
            SDL_Rect leg1 = {playerRect.x + 8 + legOffset, playerRect.y + 24, 6, 8};
            SDL_Rect leg2 = {playerRect.x + 18 - legOffset, playerRect.y + 24, 6, 8};
            SDL_RenderFillRect(renderer, &leg1);
            SDL_RenderFillRect(renderer, &leg2);
        } else {
            SDL_Rect leg = {playerRect.x + 10, playerRect.y + 24, 12, 8};
            SDL_RenderFillRect(renderer, &leg);
        }
        
        // Direction indicator (mustache)
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        if (facingRight) {
            SDL_Rect mustache = {playerRect.x + 12, playerRect.y + 10, 8, 3};
            SDL_RenderFillRect(renderer, &mustache);
        } else {
            SDL_Rect mustache = {playerRect.x + 12, playerRect.y + 10, 8, 3};
            SDL_RenderFillRect(renderer, &mustache);
        }
        
        // UI - Score and Lives
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
        SDL_Rect uiBox = {10, 10, 200, 40};
        SDL_RenderFillRect(renderer, &uiBox);
        
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawRect(renderer, &uiBox);
        
        // Draw hearts for lives
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        for (int i = 0; i < lives; i++) {
            SDL_Rect heart = {150 + i * 18, 20, 12, 12};
            SDL_RenderFillRect(renderer, &heart);
        }
        
        SDL_RenderPresent(renderer);
        SDL_Delay(16); // ~60 FPS
    }
    
    // Game over screen
    if (gameOver) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_Rect gameOverBox = {windowWidth / 2 - 150, windowHeight / 2 - 50, 300, 100};
        SDL_RenderFillRect(renderer, &gameOverBox);
        
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawRect(renderer, &gameOverBox);
        
        SDL_RenderPresent(renderer);
        SDL_Delay(2000);
    }
    
    return true;
}
