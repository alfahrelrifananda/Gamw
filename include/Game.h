#ifndef GAME_H
#define GAME_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <vector>
#include <string>

class Game {
public:
    Game();
    ~Game();
    
    bool init();
    void run();
    void cleanup();

private:
    // Core SDL components
    SDL_Window* window;
    SDL_Renderer* renderer;
    TTF_Font* font;
    TTF_Font* titleFont;
    bool running;
    
    // Display settings
    SDL_DisplayMode desktop_mode;
    static const int LOGICAL_WIDTH = 384;   // Increased for better readability
    static const int LOGICAL_HEIGHT = 288;
    static const int windowed_width = 1280;
    static const int windowed_height = 960;
    bool fullscreen;
    
    // Menu state
    int selected_item;
    int current_menu;  // 0=main, 1=settings, 2=play_submenu
    
    // Animation & effects
    float menu_fade;
    Uint32 selection_time;
    float pulse_phase;
    int bg_stars[50][3];  // x, y, brightness
    
    // Input handling
    Uint32 last_key_time;
    static const Uint32 KEY_REPEAT_DELAY = 150;
    
    // Core functions
    void handleEvents();
    void update();
    void render();
    
    // Rendering functions
    void renderBackground();
    void renderStarfield();
    void renderTitle();
    void renderMenuItems(const char* items[], int num_items, int y_offset = 0);
    void renderMenuBox(int x, int y, int w, int h, bool selected);
    void renderText(const char* text, int x, int y, SDL_Color color, bool centered = true);
    void renderFPS();
    
    // Menu functions
    void toggleFullscreen();
    void initStarfield();
    
    // Utility
    SDL_Color lerpColor(SDL_Color a, SDL_Color b, float t);
    float easeInOutCubic(float t);
};

#endif