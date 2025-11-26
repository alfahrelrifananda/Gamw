#ifndef MENU_H
#define MENU_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <vector>
#include <string>

enum GameState {
    MENU,
    PLAYING,
    SETTINGS
};

struct MenuItem {
    std::string text;
    SDL_Rect rect;
    bool hovered;
    float selectAnim;
    
    MenuItem(const std::string& t, int x, int y, int w, int h)
        : text(t), hovered(false), selectAnim(0.0f) {
        rect = {x, y, w, h};
    }
};

class Menu {
public:
    Menu();
    ~Menu();
    
    bool init(int windowWidth, int windowHeight);
    void handleEvent(SDL_Event& e, GameState& state, bool& running);
    void update(float deltaTime);
    void render(SDL_Renderer* renderer);
    void cleanup();
    
private:
    // Fonts
    TTF_Font* titleFont;
    TTF_Font* itemFont;
    TTF_Font* smallFont;
    
    // Menu state
    std::vector<MenuItem> items;
    int selectedItem;
    int windowWidth;
    int windowHeight;
    
    // Animation
    float pulsePhase;
    float fadeIn;
    Uint32 lastSelectTime;
    float coinRotation;
    
    // Clouds
    struct Cloud {
        float x, y, speed;
    };
    std::vector<Cloud> clouds;
    
    // Input handling
    Uint32 lastKeyTime;
    static const Uint32 KEY_REPEAT_DELAY = 150;
    
    // Helper functions
    void handleKeyboard(SDL_Event& e);
    void handleMouse(SDL_Event& e, GameState& state, bool& running);
    void selectItem(GameState& state, bool& running);
    
    // Rendering
    void renderBackground(SDL_Renderer* renderer);
    void renderClouds(SDL_Renderer* renderer);
    void renderGround(SDL_Renderer* renderer);
    void renderTitle(SDL_Renderer* renderer);
    void renderItems(SDL_Renderer* renderer);
    void renderMenuItem(SDL_Renderer* renderer, MenuItem& item, bool isSelected);
    void renderText(SDL_Renderer* renderer, const char* text, int x, int y, 
                   TTF_Font* font, SDL_Color color, bool centered = true);
    void renderCoin(SDL_Renderer* renderer, int x, int y, float rotation);
    void renderMushroom(SDL_Renderer* renderer, int x, int y);
    
    // Utilities
    void initClouds();
    float easeInOutCubic(float t);
    SDL_Color lerpColor(SDL_Color a, SDL_Color b, float t);
};

#endif
