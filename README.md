# 🎮 Gamw

> *A sleek, animated menu system built with C++ and SDL2*

**Gamw** is a minimalist game framework featuring a polished menu system with smooth animations, starfield effects, and full Wayland/X11 compatibility. Built from scratch with raw C++ and SDL2 - no bloat, maximum performance.

## ✨ Features

* **🎨 Animated UI** - Pulsing title effects, smooth transitions, glowing selections
* **⭐ Starfield Background** - Parallax scrolling stars with dynamic brightness
* **🖱️ Dual Input** - Full keyboard (WASD/Arrows) and mouse support
* **🖥️ Wayland Ready** - Tested on Hyprland, works on X11 too
* **📺 Display Modes** - Seamless fullscreen/windowed toggle (F11)
* **🎯 Responsive** - Smooth 60 FPS with proper delta timing
* **🔤 Font Fallback** - Automatic font detection across Linux/Windows

## 🎮 Controls

| Action | Keys |
|--------|------|
| **Navigate** | ↑↓ or W/S |
| **Select** | Enter or Space |
| **Back/Exit** | ESC |
| **Toggle Fullscreen** | F11 |
| **Mouse** | Hover + Click |

## 🛠️ Build Instructions

### Prerequisites
- C++11 compiler (g++, clang, MSVC)
- SDL2 and SDL2_ttf development libraries
- CMake (optional but recommended)

---

### 🐧 Linux (Arch/Manjaro/Hyprland)

```bash
# Install dependencies
sudo pacman -S sdl2 sdl2_ttf

# Compile
g++ -o gamw main.cpp Menu.cpp -lSDL2 -lSDL2_ttf -lm -std=c++11

# Run
./gamw
```

### 🐧 Linux (Debian/Ubuntu)

```bash
# Install dependencies
sudo apt install libsdl2-dev libsdl2-ttf-dev

# Compile
g++ -o gamw main.cpp Menu.cpp -lSDL2 -lSDL2_ttf -lm -std=c++11

# Run
./gamw
```

---

### 🖥️ Windows (MinGW-w64 via MSYS2)

```bash
# In MSYS2 MinGW64 terminal
pacman -S mingw-w64-x86_64-SDL2 mingw-w64-x86_64-SDL2_ttf

# Compile
g++ -o gamw.exe main.cpp Menu.cpp -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf -lm -std=c++11

# Run
./gamw.exe
```

### 🖥️ Windows (Visual Studio)

1. Install [vcpkg](https://vcpkg.io/)
2. Install dependencies: `vcpkg install sdl2 sdl2-ttf:x64-windows`
3. Open project in Visual Studio
4. Build and run

---

### 🍏 macOS

```bash
# Install dependencies
brew install sdl2 sdl2_ttf

# Compile
g++ -o gamw main.cpp Menu.cpp -lSDL2 -lSDL2_ttf -lm -std=c++11

# Run
./gamw
```

---

## 📂 Project Structure

```
gamw/
├── main.cpp          # Game loop and state management
├── Menu.cpp          # Menu implementation with animations
├── Menu.h            # Menu class definition
├── assets/
│   └── fonts/        # Font files (optional, uses system fonts as fallback)
└── README.md
```

## 🎨 Menu Features

### Visual Effects
- **Gradient background** - Smooth dark blue to purple transition
- **Animated starfield** - 100 parallax stars with varied brightness
- **Pulsing title** - Cyan glow effect on "GAMW" title
- **Selection animations** - Smooth easing with cubic interpolation
- **Arrow indicators** - Animated side arrows that pulse when selected
- **Glow effects** - Inner borders on selected items

### Menu Options
1. **START GAME** - Launch single player
2. **HOST SERVER** - Start multiplayer server
3. **JOIN SERVER** - Connect to multiplayer
4. **SETTINGS** - Configuration options
5. **QUIT** - Exit game

## 🔧 Technical Details

- **Rendering**: Hardware-accelerated SDL2 renderer with VSync
- **Frame Rate**: Locked 60 FPS with delta time calculations
- **Input Handling**: 150ms key repeat delay for smooth navigation
- **Window Management**: Dynamic resolution with fullscreen support
- **Font System**: Multiple fallback paths for cross-platform compatibility

## 🎯 Wayland Compatibility

Fully tested on:
- ✅ Hyprland
- ✅ Sway
- ✅ GNOME Wayland
- ✅ X11 (backwards compatible)

## 🚀 Future Plans

- [ ] Actual gameplay implementation
- [ ] Network multiplayer functionality
- [ ] Settings menu (audio, graphics, controls)
- [ ] Save/load system
- [ ] Controller support

## 📝 License

Do whatever you want with it. No restrictions.

---

**Now go build something cool.** 🚀