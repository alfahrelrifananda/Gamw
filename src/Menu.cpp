#include "Menu.h"
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <ctime>

Menu::Menu() 
    : selectedItem(0), titleFont(nullptr), itemFont(nullptr), smallFont(nullptr),
      pulsePhase(0.0f), fadeIn(0.0f), lastSelectTime(0), lastKeyTime(0),
      windowWidth(800), windowHeight(600), coinRotation(0.0f) {
    std::srand(static_cast<unsigned>(std::time(nullptr)));
}

Menu::~Menu() {
    cleanup();
}

bool Menu::init(int wWidth, int wHeight) {
    windowWidth = wWidth;
    windowHeight = wHeight;
    
    // Try to load fonts with multiple fallback paths
    const char* font_paths[] = {
        "assets/PressStart2P-Regular.ttf",
        "assets/fonts/arial.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",
        "/usr/share/fonts/TTF/DejaVuSans-Bold.ttf",
        "C:\\Windows\\Fonts\\arial.ttf"
    };
    
    // Load title font (large)
    for (const char* path : font_paths) {
        titleFont = TTF_OpenFont(path, 48);
        if (titleFont) break;
    }
    
    // Load item font (medium)
    for (const char* path : font_paths) {
        itemFont = TTF_OpenFont(path, 24);
        if (itemFont) break;
    }
    
    // Load small font
    for (const char* path : font_paths) {
        smallFont = TTF_OpenFont(path, 14);
        if (smallFont) break;
    }
    
    if (!titleFont || !itemFont || !smallFont) {
        std::cerr << "Warning: Some fonts failed to load. Install DejaVu fonts or add PressStart2P font." << std::endl;
    }
    
    // Create menu items centered on screen
    int startY = windowHeight / 2 + 50;
    int spacing = 65;
    int itemWidth = 280;
    int itemHeight = 50;
    int itemX = (windowWidth - itemWidth) / 2;
    
    items.clear();
    items.push_back(MenuItem("START GAME", itemX, startY, itemWidth, itemHeight));
    items.push_back(MenuItem("SETTINGS", itemX, startY + spacing, itemWidth, itemHeight));
    items.push_back(MenuItem("QUIT", itemX, startY + spacing * 2, itemWidth, itemHeight));
    
    initClouds();
    lastSelectTime = SDL_GetTicks();
    
    std::cout << "Menu initialized with " << items.size() << " items" << std::endl;
    return true;
}

void Menu::initClouds() {
    clouds.clear();
    for (int i = 0; i < 6; ++i) {
        Cloud cloud;
        cloud.x = static_cast<float>(std::rand() % windowWidth);
        cloud.y = static_cast<float>(50 + std::rand() % 150);
        cloud.speed = 15.0f + static_cast<float>(std::rand() % 25);
        clouds.push_back(cloud);
    }
}

void Menu::handleEvent(SDL_Event& e, GameState& state, bool& running) {
    if (e.type == SDL_KEYDOWN) {
        handleKeyboard(e);
        if (e.key.keysym.sym == SDLK_RETURN || e.key.keysym.sym == SDLK_SPACE) {
            selectItem(state, running);
        }
    }
    
    handleMouse(e, state, running);
}

void Menu::handleKeyboard(SDL_Event& e) {
    Uint32 currentTime = SDL_GetTicks();
    if (currentTime - lastKeyTime < KEY_REPEAT_DELAY) return;
    
    switch (e.key.keysym.sym) {
        case SDLK_UP:
        case SDLK_w:
            lastKeyTime = currentTime;
            selectedItem = (selectedItem - 1 + items.size()) % items.size();
            lastSelectTime = currentTime;
            break;
            
        case SDLK_DOWN:
        case SDLK_s:
            lastKeyTime = currentTime;
            selectedItem = (selectedItem + 1) % items.size();
            lastSelectTime = currentTime;
            break;
            
        case SDLK_ESCAPE:
            lastKeyTime = currentTime;
            std::cout << "ESC pressed - exiting" << std::endl;
            break;
    }
}

void Menu::handleMouse(SDL_Event& e, GameState& state, bool& running) {
    if (e.type == SDL_MOUSEMOTION || e.type == SDL_MOUSEBUTTONDOWN) {
        int mx, my;
        SDL_GetMouseState(&mx, &my);
        
        for (size_t i = 0; i < items.size(); i++) {
            SDL_Rect& r = items[i].rect;
            bool wasHovered = items[i].hovered;
            
            if (mx >= r.x && mx <= r.x + r.w && my >= r.y && my <= r.y + r.h) {
                items[i].hovered = true;
                if (!wasHovered) {
                    selectedItem = i;
                    lastSelectTime = SDL_GetTicks();
                }
                
                if (e.type == SDL_MOUSEBUTTONDOWN) {
                    selectItem(state, running);
                }
            } else {
                items[i].hovered = false;
            }
        }
    }
}

void Menu::selectItem(GameState& state, bool& running) {
    std::cout << "Selected: " << items[selectedItem].text << std::endl;
    
    switch (selectedItem) {
        case 0:  // START GAME
            state = PLAYING;
            std::cout << "[*] Starting game..." << std::endl;
            break;
            
        case 1:  // SETTINGS
            state = SETTINGS;
            std::cout << "[*] Opening settings..." << std::endl;
            break;
            
        case 2:  // QUIT
            std::cout << "[*] Goodbye!" << std::endl;
            running = false;
            break;
    }
}

void Menu::update(float deltaTime) {
    // Fade in animation
    if (fadeIn < 1.0f) {
        fadeIn += deltaTime * 2.0f;
        if (fadeIn > 1.0f) fadeIn = 1.0f;
    }
    
    // Pulse animation
    pulsePhase += deltaTime * 3.0f;
    if (pulsePhase > 2.0f * M_PI) pulsePhase -= 2.0f * M_PI;
    
    // Coin rotation
    coinRotation += deltaTime * 4.0f;
    if (coinRotation > 2.0f * M_PI) coinRotation -= 2.0f * M_PI;
    
    // Update selection animation
    for (size_t i = 0; i < items.size(); ++i) {
        float target = (i == static_cast<size_t>(selectedItem)) ? 1.0f : 0.0f;
        float diff = target - items[i].selectAnim;
        items[i].selectAnim += diff * deltaTime * 10.0f;
    }
    
    // Animate clouds
    for (auto& cloud : clouds) {
        cloud.x += cloud.speed * deltaTime;
        if (cloud.x > windowWidth + 100) {
            cloud.x = -100;
            cloud.y = static_cast<float>(50 + std::rand() % 150);
        }
    }
}

void Menu::render(SDL_Renderer* renderer) {
    renderBackground(renderer);
    renderClouds(renderer);
    renderGround(renderer);
    renderDecorations(renderer);
    renderTitle(renderer);
    renderItems(renderer);
    renderFooter(renderer);
}

void Menu::renderBackground(SDL_Renderer* renderer) {
    // Sky blue gradient matching game
    for (int y = 0; y < windowHeight - 80; ++y) {
        float t = static_cast<float>(y) / (windowHeight - 80);
        Uint8 r = static_cast<Uint8>(92 + (255 - 92) * t * 0.3f);
        Uint8 g = static_cast<Uint8>(148 + (140 - 148) * t * 0.3f);
        Uint8 b = 252;
        SDL_SetRenderDrawColor(renderer, r, g, b, 255);
        SDL_RenderDrawLine(renderer, 0, y, windowWidth, y);
    }
}

void Menu::renderClouds(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    
    for (const auto& cloud : clouds) {
        int cx = static_cast<int>(cloud.x);
        int cy = static_cast<int>(cloud.y);
        
        // Cloud body (matching game style)
        SDL_Rect main = {cx, cy + 10, 50, 25};
        SDL_RenderFillRect(renderer, &main);
        
        SDL_Rect left = {cx + 10, cy, 35, 30};
        SDL_RenderFillRect(renderer, &left);
        
        SDL_Rect right = {cx + 30, cy + 5, 40, 28};
        SDL_RenderFillRect(renderer, &right);
    }
}

void Menu::renderGround(SDL_Renderer* renderer) {
    int groundY = windowHeight - 80;
    
    // Grass layer with texture (matching game)
    SDL_SetRenderDrawColor(renderer, 123, 192, 67, 255);
    SDL_Rect grass = {0, groundY, windowWidth, 20};
    SDL_RenderFillRect(renderer, &grass);
    
    // Grass blades
    SDL_SetRenderDrawColor(renderer, 100, 170, 50, 255);
    for (int i = 0; i < windowWidth; i += 4) {
        SDL_Rect blade = {i, groundY, 2, 12 + (i % 8)};
        SDL_RenderFillRect(renderer, &blade);
    }
    
    // Dirt layer with texture
    SDL_SetRenderDrawColor(renderer, 139, 90, 43, 255);
    SDL_Rect dirt = {0, groundY + 20, windowWidth, 60};
    SDL_RenderFillRect(renderer, &dirt);
    
    // Add dirt texture dots
    SDL_SetRenderDrawColor(renderer, 120, 75, 35, 255);
    for (int y = 0; y < 60; y += 6) {
        for (int x = 0; x < windowWidth; x += 8) {
            int dotSize = ((x + y) % 3) + 1;
            SDL_Rect dot = {x + ((x + y) % 4), groundY + 20 + y, dotSize, dotSize};
            SDL_RenderFillRect(renderer, &dot);
        }
    }
    
    // Lighter dirt spots
    SDL_SetRenderDrawColor(renderer, 160, 110, 60, 255);
    for (int y = 0; y < 60; y += 8) {
        for (int x = 0; x < windowWidth; x += 12) {
            if ((x + y) % 5 == 0) {
                SDL_Rect lightSpot = {x, groundY + 22 + y, 3, 3};
                SDL_RenderFillRect(renderer, &lightSpot);
            }
        }
    }
}

void Menu::renderDecorations(SDL_Renderer* renderer) {
    // Animated coins on both sides
    float coinBounce = std::sin(coinRotation) * 8.0f;
    
    // Left coin
    renderCoin(renderer, 150, static_cast<int>(windowHeight / 2 - 50 + coinBounce), coinRotation);
    
    // Right coin
    renderCoin(renderer, windowWidth - 150, static_cast<int>(windowHeight / 2 - 50 + coinBounce), coinRotation + 1.5f);
    
    // Question blocks
    renderQuestionBlock(renderer, 120, windowHeight / 2 + 80);
    renderQuestionBlock(renderer, windowWidth - 120, windowHeight / 2 + 80);
    
    // Pipes decoration
    renderPipe(renderer, 80, windowHeight - 140);
    renderPipe(renderer, windowWidth - 120, windowHeight - 140);
}

void Menu::renderQuestionBlock(SDL_Renderer* renderer, int x, int y) {
    int size = 32;
    float bounce = std::sin(pulsePhase * 2.0f) * 3.0f;
    SDL_Rect block = {x, static_cast<int>(y + bounce), size, size};
    
    // Orange/yellow base
    SDL_SetRenderDrawColor(renderer, 255, 200, 100, static_cast<Uint8>(255 * fadeIn));
    SDL_RenderFillRect(renderer, &block);
    
    // Top highlight
    SDL_SetRenderDrawColor(renderer, 255, 230, 150, static_cast<Uint8>(255 * fadeIn));
    SDL_Rect highlight = {block.x + 2, block.y + 2, block.w - 4, 8};
    SDL_RenderFillRect(renderer, &highlight);
    
    // Bottom shadow
    SDL_SetRenderDrawColor(renderer, 200, 140, 60, static_cast<Uint8>(255 * fadeIn));
    SDL_Rect shadow = {block.x + 2, block.y + block.h - 10, block.w - 4, 8};
    SDL_RenderFillRect(renderer, &shadow);
    
    // Border
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, static_cast<Uint8>(255 * fadeIn));
    SDL_RenderDrawRect(renderer, &block);
    
    // Draw "?"
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, static_cast<Uint8>(255 * fadeIn));
    SDL_Rect qTop = {block.x + 10, block.y + 6, 12, 8};
    SDL_RenderFillRect(renderer, &qTop);
    SDL_Rect qMid = {block.x + 14, block.y + 12, 8, 6};
    SDL_RenderFillRect(renderer, &qMid);
    SDL_Rect qDot = {block.x + 14, block.y + 20, 6, 6};
    SDL_RenderFillRect(renderer, &qDot);
}

void Menu::renderPipe(SDL_Renderer* renderer, int x, int y) {
    int width = 60;
    int height = 60;
    
    // Pipe top (darker green)
    SDL_SetRenderDrawColor(renderer, 80, 180, 80, static_cast<Uint8>(255 * fadeIn));
    SDL_Rect top = {x - 5, y, width + 10, 12};
    SDL_RenderFillRect(renderer, &top);
    
    // Pipe top highlight
    SDL_SetRenderDrawColor(renderer, 120, 220, 120, static_cast<Uint8>(255 * fadeIn));
    SDL_Rect topHighlight = {x - 3, y + 2, width + 6, 4};
    SDL_RenderFillRect(renderer, &topHighlight);
    
    // Pipe body
    SDL_SetRenderDrawColor(renderer, 90, 190, 90, static_cast<Uint8>(255 * fadeIn));
    SDL_Rect body = {x, y + 12, width, height};
    SDL_RenderFillRect(renderer, &body);
    
    // Pipe highlight (left side)
    SDL_SetRenderDrawColor(renderer, 130, 230, 130, static_cast<Uint8>(255 * fadeIn));
    SDL_Rect bodyHighlight = {x + 4, y + 14, 10, height - 2};
    SDL_RenderFillRect(renderer, &bodyHighlight);
    
    // Pipe shadow (right side)
    SDL_SetRenderDrawColor(renderer, 60, 140, 60, static_cast<Uint8>(255 * fadeIn));
    SDL_Rect bodyShadow = {x + width - 14, y + 14, 10, height - 2};
    SDL_RenderFillRect(renderer, &bodyShadow);
    
    // Borders
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, static_cast<Uint8>(255 * fadeIn));
    SDL_RenderDrawRect(renderer, &top);
    SDL_RenderDrawRect(renderer, &body);
}

void Menu::renderTitle(SDL_Renderer* renderer) {
    if (!titleFont) return;
    
    float bounce = std::sin(pulsePhase * 1.5f) * 5.0f;
    int titleY = static_cast<int>(windowHeight / 2 - 150 + bounce);
    
    // Shadow
    SDL_Color shadow = {0, 0, 0, static_cast<Uint8>(180 * fadeIn)};
    renderText(renderer, "NOT A CAT MARIO", windowWidth / 2 + 3, 
            titleY + 3, titleFont, shadow, true);
    
    // Main title - Red color (Mario style)
    SDL_Color red = {228, 0, 0, static_cast<Uint8>(255 * fadeIn)};
    renderText(renderer, "NOT A CAT MARIO", windowWidth / 2, 
            titleY, titleFont, red, true);
    
    // Subtitle
    if (itemFont) {
        SDL_Color yellow = {255, 220, 0, static_cast<Uint8>(255 * fadeIn)};
        renderText(renderer, "ADVENTURE", windowWidth / 2, 
                titleY + 55, itemFont, yellow, true);
    }
}

void Menu::renderItems(SDL_Renderer* renderer) {
    for (size_t i = 0; i < items.size(); i++) {
        bool isSelected = (i == static_cast<size_t>(selectedItem));
        renderMenuItem(renderer, items[i], isSelected);
    }
}

void Menu::renderMenuItem(SDL_Renderer* renderer, MenuItem& item, bool isSelected) {
    SDL_Rect& r = item.rect;
    float anim = easeInOutCubic(item.selectAnim);
    
    // Brick platform style (matching game)
    int brickW = 16;
    int brickH = 16;
    
    // Base color
    if (isSelected) {
        SDL_SetRenderDrawColor(renderer, 210, 130, 90, static_cast<Uint8>(255 * fadeIn));
    } else {
        SDL_SetRenderDrawColor(renderer, 184, 111, 80, static_cast<Uint8>(230 * fadeIn));
    }
    SDL_RenderFillRect(renderer, &r);
    
    // Brick pattern with proper clipping
    for (int by = 0; by < r.h; by += brickH) {
        for (int bx = 0; bx < r.w; bx += brickW) {
            int offset = (by / brickH) % 2 == 0 ? 0 : brickW / 2;
            int actualX = r.x + bx + offset;
            
            // Skip if brick would be completely outside
            if (actualX >= r.x + r.w || actualX + brickW <= r.x) continue;
            
            // Calculate clipped brick dimensions
            int brickStartX = actualX;
            int brickStartY = r.y + by;
            int brickEndX = actualX + brickW;
            int brickEndY = r.y + by + brickH;
            
            // Clip to menu bounds
            if (brickStartX < r.x) brickStartX = r.x;
            if (brickEndX > r.x + r.w) brickEndX = r.x + r.w;
            if (brickEndY > r.y + r.h) brickEndY = r.y + r.h;
            
            int clippedWidth = brickEndX - brickStartX;
            int clippedHeight = brickEndY - brickStartY;
            
            if (clippedWidth > 0 && clippedHeight > 0) {
                // Brick highlight (top-left)
                SDL_SetRenderDrawColor(renderer, 
                    isSelected ? 230 : 210, 
                    isSelected ? 160 : 140, 
                    isSelected ? 110 : 100, 
                    static_cast<Uint8>(255 * fadeIn));
                
                if (clippedHeight > 2) {
                    SDL_Rect highlight = {brickStartX, brickStartY, clippedWidth - 2, 2};
                    SDL_RenderFillRect(renderer, &highlight);
                }
                if (clippedWidth > 2) {
                    SDL_Rect highlightL = {brickStartX, brickStartY, 2, clippedHeight - 2};
                    SDL_RenderFillRect(renderer, &highlightL);
                }
                
                // Brick shadow (bottom-right)
                SDL_SetRenderDrawColor(renderer, 140, 80, 60, static_cast<Uint8>(200 * fadeIn));
                if (clippedHeight > 2 && clippedWidth > 4) {
                    SDL_Rect shadow = {brickStartX + 2, brickStartY + clippedHeight - 2, clippedWidth - 2, 2};
                    SDL_RenderFillRect(renderer, &shadow);
                }
                if (clippedWidth > 2 && clippedHeight > 4) {
                    SDL_Rect shadowR = {brickStartX + clippedWidth - 2, brickStartY + 2, 2, clippedHeight - 2};
                    SDL_RenderFillRect(renderer, &shadowR);
                }
                
                // Mortar lines (dark lines between bricks)
                SDL_SetRenderDrawColor(renderer, 100, 70, 50, static_cast<Uint8>(150 * fadeIn));
                if (brickEndY <= r.y + r.h) {
                    SDL_Rect mortarH = {brickStartX, brickStartY + clippedHeight - 1, clippedWidth, 1};
                    SDL_RenderFillRect(renderer, &mortarH);
                }
                if (brickEndX <= r.x + r.w) {
                    SDL_Rect mortarV = {brickStartX + clippedWidth - 1, brickStartY, 1, clippedHeight};
                    SDL_RenderFillRect(renderer, &mortarV);
                }
            }
        }
    }
    
    // Outer border
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, static_cast<Uint8>(255 * fadeIn));
    SDL_RenderDrawRect(renderer, &r);
    
    // Selection indicators - Mario stars
    if (isSelected) {
        float starBounce = std::sin(pulsePhase * 4.0f) * 4.0f;
        int midY = r.y + r.h / 2;
        
        // Left indicator
        renderStar(renderer, r.x - 30, midY + static_cast<int>(starBounce));
        
        // Right indicator
        renderStar(renderer, r.x + r.w + 20, midY + static_cast<int>(starBounce));
    }
    
    // Text
    if (itemFont) {
        SDL_Color textColor;
        if (isSelected) {
            textColor = {255, 255, 255, static_cast<Uint8>(255 * fadeIn)};
        } else {
            textColor = {245, 235, 215, static_cast<Uint8>(240 * fadeIn)};
        }
        
        renderText(renderer, item.text.c_str(), r.x + r.w / 2, 
                r.y + r.h / 2 - 10, itemFont, textColor, true);
    }
}

void Menu::renderStar(SDL_Renderer* renderer, int x, int y) {
    int size = 16;
    
    // Yellow star body
    SDL_SetRenderDrawColor(renderer, 255, 220, 0, static_cast<Uint8>(255 * fadeIn));
    
    // Simple star shape using rectangles
    SDL_Rect center = {x - size/4, y - size/4, size/2, size/2};
    SDL_RenderFillRect(renderer, &center);
    
    // Points
    SDL_Rect top = {x - size/8, y - size/2, size/4, size/3};
    SDL_RenderFillRect(renderer, &top);
    
    SDL_Rect bottom = {x - size/8, y + size/6, size/4, size/3};
    SDL_RenderFillRect(renderer, &bottom);
    
    SDL_Rect left = {x - size/2, y - size/8, size/3, size/4};
    SDL_RenderFillRect(renderer, &left);
    
    SDL_Rect right = {x + size/6, y - size/8, size/3, size/4};
    SDL_RenderFillRect(renderer, &right);
    
    // Inner glow
    SDL_SetRenderDrawColor(renderer, 255, 250, 200, static_cast<Uint8>(255 * fadeIn));
    SDL_Rect glow = {x - size/6, y - size/6, size/3, size/3};
    SDL_RenderFillRect(renderer, &glow);
}

void Menu::renderCoin(SDL_Renderer* renderer, int x, int y, float rotation) {
    int size = 20;
    float scale = std::abs(std::cos(rotation));
    int width = static_cast<int>(size * scale);
    if (width < 4) width = 4;
    
    // Gold coin
    SDL_SetRenderDrawColor(renderer, 255, 215, 0, static_cast<Uint8>(255 * fadeIn));
    SDL_Rect coin = {x - width / 2, y - size / 2, width, size};
    SDL_RenderFillRect(renderer, &coin);
    
    // Inner darker gold
    SDL_SetRenderDrawColor(renderer, 218, 165, 32, static_cast<Uint8>(255 * fadeIn));
    SDL_Rect innerCoin = {x - width / 2 + 2, y - 6, width > 4 ? width - 4 : 2, 12};
    SDL_RenderFillRect(renderer, &innerCoin);
    
    // Shine highlight
    if (width > 6) {
        SDL_SetRenderDrawColor(renderer, 255, 250, 205, static_cast<Uint8>(255 * fadeIn));
        SDL_Rect shine = {x - width / 2 + 2, y - 6, width / 3, 4};
        SDL_RenderFillRect(renderer, &shine);
    }
    
    // Border
    SDL_SetRenderDrawColor(renderer, 184, 134, 11, static_cast<Uint8>(255 * fadeIn));
    SDL_RenderDrawRect(renderer, &coin);
}

void Menu::renderFooter(SDL_Renderer* renderer) {
    if (smallFont) {
        // Controls info
        SDL_Color white = {255, 255, 255, static_cast<Uint8>(220 * fadeIn)};
        renderText(renderer, "Controls: Arrow Keys or WASD to navigate", 
                windowWidth / 2, windowHeight - 45, smallFont, white, true);
        
        renderText(renderer, "Press ENTER or SPACE to select", 
                windowWidth / 2, windowHeight - 25, smallFont, white, true);
    }
}

void Menu::renderText(SDL_Renderer* renderer, const char* text, int x, int y, 
                    TTF_Font* font, SDL_Color color, bool centered) {
    if (!font) return;
    
    SDL_Surface* surface = TTF_RenderText_Solid(font, text, color);
    if (surface) {
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_Rect rect = {
            centered ? x - surface->w / 2 : x,
            y,
            surface->w,
            surface->h
        };
        SDL_RenderCopy(renderer, texture, nullptr, &rect);
        SDL_DestroyTexture(texture);
        SDL_FreeSurface(surface);
    }
}

float Menu::easeInOutCubic(float t) {
    return t < 0.5f ? 4.0f * t * t * t : 1.0f - std::pow(-2.0f * t + 2.0f, 3.0f) / 2.0f;
}

SDL_Color Menu::lerpColor(SDL_Color a, SDL_Color b, float t) {
    return {
        static_cast<Uint8>(a.r + (b.r - a.r) * t),
        static_cast<Uint8>(a.g + (b.g - a.g) * t),
        static_cast<Uint8>(a.b + (b.b - a.b) * t),
        static_cast<Uint8>(a.a + (b.a - a.a) * t)
    };
}

void Menu::renderMushroom(SDL_Renderer* renderer, int x, int y) {
    // Not used in new design, but kept for compatibility
}

void Menu::cleanup() {
    if (titleFont) {
        TTF_CloseFont(titleFont);
        titleFont = nullptr;
    }
    if (itemFont) {
        TTF_CloseFont(itemFont);
        itemFont = nullptr;
    }
    if (smallFont) {
        TTF_CloseFont(smallFont);
        smallFont = nullptr;
    }
}