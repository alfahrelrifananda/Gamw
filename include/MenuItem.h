#ifndef MENUITEM_H
#define MENUITEM_H

#include <string>
#include <SDL2/SDL.h>

struct MenuItem {
    std::string text;
    SDL_Rect rect;
    bool hovered;
    
    MenuItem(const std::string& t, int x, int y, int w, int h)
        : text(t), hovered(false) {
        rect = {x, y, w, h};
    }
};

#endif
