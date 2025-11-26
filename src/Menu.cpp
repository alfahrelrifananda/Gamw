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
        titleFont = TTF_OpenFont(path, 72);
        if (titleFont) break;
    }
    
    // Load item font (medium)
    for (const char* path : font_paths) {
        itemFont = TTF_OpenFont(path, 28);
        if (itemFont) break;
    }
    
    // Load small font
    for (const char* path : font_paths) {
        smallFont = TTF_OpenFont(path, 16);
        if (smallFont) break;
    }
    
    if (!titleFont || !itemFont || !smallFont) {
        std::cerr << "Warning: Some fonts failed to load. Install DejaVu fonts or add PressStart2P font." << std::endl;
    }
    
    // Create menu items centered on screen
    int startY = windowHeight / 2 + 20;
    int spacing = 70;
    int itemWidth = 320;
    int itemHeight = 55;
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
    for (int i = 0; i < 5; ++i) {
        Cloud cloud;
        cloud.x = static_cast<float>(std::rand() % windowWidth);
        cloud.y = static_cast<float>(50 + std::rand() % 200);
        cloud.speed = 10.0f + static_cast<float>(std::rand() % 20);
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
            cloud.y = static_cast<float>(50 + std::rand() % 200);
        }
    }
}

void Menu::render(SDL_Renderer* renderer) {
    renderBackground(renderer);
    renderClouds(renderer);
    renderGround(renderer);
    renderTitle(renderer);
    renderItems(renderer);
    
    // Coins decoration
    renderCoin(renderer, windowWidth / 2 - 200, windowHeight / 2 - 100, coinRotation);
    renderCoin(renderer, windowWidth / 2 + 200, windowHeight / 2 - 100, coinRotation + 1.0f);
    
    // Version info
    if (smallFont) {
        SDL_Color white = {255, 255, 255, 200};
        renderText(renderer, "v1.0 - Arrow Keys/WASD to navigate - ENTER to select", 
                windowWidth / 2, windowHeight - 25, smallFont, white, true);
    }
}

void Menu::renderBackground(SDL_Renderer* renderer) {
    // Sky blue gradient (Mario sky)
    for (int y = 0; y < windowHeight - 100; ++y) {
        float t = static_cast<float>(y) / (windowHeight - 100);
        Uint8 r = static_cast<Uint8>(92 + t * 10);
        Uint8 g = static_cast<Uint8>(148 + t * 20);
        Uint8 b = static_cast<Uint8>(252 - t * 40);
        SDL_SetRenderDrawColor(renderer, r, g, b, 255);
        SDL_RenderDrawLine(renderer, 0, y, windowWidth, y);
    }
}

void Menu::renderClouds(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    
    for (const auto& cloud : clouds) {
        // Simple cloud shape (3 circles approximated with rects)
        int cx = static_cast<int>(cloud.x);
        int cy = static_cast<int>(cloud.y);
        
        SDL_Rect parts[] = {
            {cx, cy + 10, 40, 20},
            {cx + 15, cy, 30, 25},
            {cx + 30, cy + 8, 35, 22}
        };
        
        for (const auto& part : parts) {
            SDL_RenderFillRect(renderer, &part);
        }
    }
}

void Menu::renderGround(SDL_Renderer* renderer) {
    int groundY = windowHeight - 100;
    
    // Green ground
    SDL_SetRenderDrawColor(renderer, 123, 192, 67, 255);
    SDL_Rect ground = {0, groundY, windowWidth, 100};
    SDL_RenderFillRect(renderer, &ground);
    
    // Brown dirt layer
    SDL_SetRenderDrawColor(renderer, 139, 90, 43, 255);
    SDL_Rect dirt = {0, groundY + 40, windowWidth, 60};
    SDL_RenderFillRect(renderer, &dirt);
    
    // Grass blades (simple lines)
    SDL_SetRenderDrawColor(renderer, 100, 160, 50, 255);
    for (int x = 0; x < windowWidth; x += 20) {
        SDL_RenderDrawLine(renderer, x, groundY, x, groundY - 8);
        SDL_RenderDrawLine(renderer, x + 10, groundY, x + 10, groundY - 12);
    }
}

void Menu::renderTitle(SDL_Renderer* renderer) {
    if (!titleFont) return;
    
    // Title with shadow
    float bounce = std::sin(pulsePhase) * 8.0f;
    
    // Shadow
    SDL_Color shadow = {0, 0, 0, static_cast<Uint8>(200 * fadeIn)};
    renderText(renderer, "SUPER GAMW", windowWidth / 2 + 4, 
            static_cast<int>(80 + bounce + 4), titleFont, shadow, true);
    
    // Main title - Red color (Mario style)
    SDL_Color red = {
        static_cast<Uint8>(228),
        static_cast<Uint8>(0),
        static_cast<Uint8>(0),
        static_cast<Uint8>(255 * fadeIn)
    };
    renderText(renderer, "SUPER GAMW", windowWidth / 2, 
            static_cast<int>(80 + bounce), titleFont, red, true);
    
    // Subtitle
    if (itemFont) {
        SDL_Color yellow = {255, 220, 0, static_cast<Uint8>(255 * fadeIn)};
        renderText(renderer, "BROS", windowWidth / 2, 
                static_cast<int>(150 + bounce * 0.5f), itemFont, yellow, true);
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
    
    // Box background - brick style
    if (isSelected) {
        // Orange/yellow brick (question block style)
        SDL_SetRenderDrawColor(renderer, 243, 168, 59, static_cast<Uint8>(255 * fadeIn));
    } else {
        // Brown brick
        SDL_SetRenderDrawColor(renderer, 184, 111, 80, static_cast<Uint8>(200 * fadeIn));
    }
    SDL_RenderFillRect(renderer, &r);
    
    // Brick outline
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, static_cast<Uint8>(255 * fadeIn));
    SDL_RenderDrawRect(renderer, &r);
    
    // Inner brick details
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, static_cast<Uint8>(100 * fadeIn));
    int midX = r.x + r.w / 2;
    int midY = r.y + r.h / 2;
    SDL_RenderDrawLine(renderer, r.x, midY, r.x + r.w, midY);
    SDL_RenderDrawLine(renderer, midX, r.y, midX, r.y + r.h);
    
    // Selection indicator - Mario star
    if (isSelected) {
        float starBounce = std::sin(pulsePhase * 3.0f) * 3.0f;
        int starSize = 12;
        
        // Left star
        int leftX = r.x - 35;
        int leftY = midY + static_cast<int>(starBounce);
        renderMushroom(renderer, leftX, leftY - starSize);
        
        // Right star
        int rightX = r.x + r.w + 20;
        int rightY = midY + static_cast<int>(starBounce);
        renderMushroom(renderer, rightX, rightY - starSize);
    }
    
    // Render text
    if (itemFont) {
        SDL_Color textColor;
        if (isSelected) {
            textColor = {255, 255, 255, static_cast<Uint8>(255 * fadeIn)};
        } else {
            textColor = {240, 230, 200, static_cast<Uint8>(230 * fadeIn)};
        }
        
        renderText(renderer, item.text.c_str(), midX, 
                midY - 14, itemFont, textColor, true);
    }
}

void Menu::renderCoin(SDL_Renderer* renderer, int x, int y, float rotation) {
    // Simple coin (yellow circle approximation)
    int size = 20;
    float scale = std::abs(std::cos(rotation));
    int width = static_cast<int>(size * scale);
    
    // Outer circle
    SDL_SetRenderDrawColor(renderer, 255, 200, 0, static_cast<Uint8>(255 * fadeIn));
    SDL_Rect coin = {x - width / 2, y - size / 2, width, size};
    SDL_RenderFillRect(renderer, &coin);
    
    // Inner detail
    if (scale > 0.3f) {
        SDL_SetRenderDrawColor(renderer, 200, 150, 0, static_cast<Uint8>(255 * fadeIn));
        int innerWidth = static_cast<int>(width * 0.6f);
        SDL_Rect inner = {x - innerWidth / 2, y - size / 3, innerWidth, size * 2 / 3};
        SDL_RenderFillRect(renderer, &inner);
    }
}

void Menu::renderMushroom(SDL_Renderer* renderer, int x, int y) {
    // Simple mushroom (Mario power-up style)
    int size = 24;
    
    // Stem (beige)
    SDL_SetRenderDrawColor(renderer, 240, 220, 180, static_cast<Uint8>(255 * fadeIn));
    SDL_Rect stem = {x - size / 4, y + size / 2, size / 2, size / 2};
    SDL_RenderFillRect(renderer, &stem);
    
    // Cap (red with white spots)
    SDL_SetRenderDrawColor(renderer, 255, 40, 40, static_cast<Uint8>(255 * fadeIn));
    SDL_Rect cap = {x - size / 2, y, size, size / 2};
    SDL_RenderFillRect(renderer, &cap);
    
    // White spots
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, static_cast<Uint8>(255 * fadeIn));
    SDL_Rect spot1 = {x - size / 3, y + 4, 6, 6};
    SDL_Rect spot2 = {x + size / 6, y + 4, 6, 6};
    SDL_RenderFillRect(renderer, &spot1);
    SDL_RenderFillRect(renderer, &spot2);
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
