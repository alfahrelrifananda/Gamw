#ifndef MENU_H
#define MENU_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <vector>
#include <string>

enum GameState {
    MENU,
    PLAYING,
    SETTINGS,
    PAUSED
};

struct MenuItem {
    std::string text;
    SDL_Rect rect;
    bool hovered;
    float selectAnim;
    
    MenuItem(const std::string& t, int x, int y, int w, int h)
        : text(t), rect{x, y, w, h}, hovered(false), selectAnim(0.0f) {}
};

struct Cloud {
    float x, y;
    float speed;
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
    // Event handling
    void handleKeyboard(SDL_Event& e);
    void handleMouse(SDL_Event& e, GameState& state, bool& running);
    void selectItem(GameState& state, bool& running);
    
    // Rendering functions
    void renderBackground(SDL_Renderer* renderer);
    void renderClouds(SDL_Renderer* renderer);
    void renderGround(SDL_Renderer* renderer);
    void renderDecorations(SDL_Renderer* renderer);
    void renderTitle(SDL_Renderer* renderer);
    void renderItems(SDL_Renderer* renderer);
    void renderMenuItem(SDL_Renderer* renderer, MenuItem& item, bool isSelected);
    void renderFooter(SDL_Renderer* renderer);
    
    // Decoration rendering
    void renderQuestionBlock(SDL_Renderer* renderer, int x, int y);
    void renderPipe(SDL_Renderer* renderer, int x, int y);
    void renderStar(SDL_Renderer* renderer, int x, int y);
    void renderCoin(SDL_Renderer* renderer, int x, int y, float rotation);
    void renderMushroom(SDL_Renderer* renderer, int x, int y);
    
    // Text rendering
    void renderText(SDL_Renderer* renderer, const char* text, int x, int y, 
                   TTF_Font* font, SDL_Color color, bool centered);
    
    // Utility functions
    void initClouds();
    float easeInOutCubic(float t);
    SDL_Color lerpColor(SDL_Color a, SDL_Color b, float t);
    
    // Member variables
    std::vector<MenuItem> items;
    std::vector<Cloud> clouds;
    int selectedItem;
    
    TTF_Font* titleFont;
    TTF_Font* itemFont;
    TTF_Font* smallFont;
    
    float pulsePhase;
    float fadeIn;
    float coinRotation;
    
    Uint32 lastSelectTime;
    Uint32 lastKeyTime;
    
    int windowWidth;
    int windowHeight;
    
    static constexpr int KEY_REPEAT_DELAY = 150;
};

#endif // MENU_H