    #ifndef GAMEBOX_H
    #define GAMEBOX_H

    #include <SDL2/SDL.h>
    #include <vector>
    #include <string>
    
    // Add this enum BEFORE the Platform struct
enum class ItemType {
    SWORD,           // Buff: faster movement
    POISON_MUSHROOM, // Debuff: slower movement
    POWER_MUSHROOM,  // Buff: invincibility for a short time
    EXTRA_LIFE       // Gives +1 life
};

    struct Platform {
        SDL_Rect rect;
        bool isBreakable;
        bool isBrick;
        bool isHit;
        ItemType containedItem; 
    };

    struct Enemy {
        float x, y;
        float vx;
        SDL_Rect rect;
        bool active;
        float leftBound;    // Tambahan: batas kiri enemy patrol
        float rightBound;   // Tambahan: batas kanan enemy patrol
    };

    struct Coin {
        int x, y;
        bool collected;
        float animPhase;
    };

    struct FloatingText {
        float x, y;
        float vy;
        int value;
        Uint32 spawnTime;
        bool active;
    };

    // Struct baru untuk level chunk system
    struct LevelChunk {
        std::vector<Platform> platforms;
        std::vector<Coin> coins;
        std::vector<Enemy> enemies;
        int startX;         // Posisi X awal chunk
        int width;          // Lebar chunk dalam pixels
    };

    bool runGameBox(SDL_Renderer* renderer);
    extern int currentStage;

    #endif
