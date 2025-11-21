# Gamw

> A sleek, animated menu system built with C++ and SDL2

**Gamw** is a minimalist game framework featuring a polished menu system with smooth animations, starfield effects, and full Wayland/X11 compatibility. Built from scratch with raw C++ and SDL2 - no bloat, maximum performance.

## Features

* Animated UI - Pulsing title effects, smooth transitions, glowing selections
* Starfield Background - Parallax scrolling stars with dynamic brightness
* Dual Input - Full keyboard (WASD/Arrows) and mouse support
* Wayland Ready - Tested on Hyprland, works on X11 too
* Display Modes - Seamless fullscreen/windowed toggle (F11)
* Responsive - Smooth 60 FPS with proper delta timing
* Font Fallback - Automatic font detection across Linux/Windows

## Controls

| Action | Keys |
|--------|------|
| Navigate | Up/Down or W/S |
| Select | Enter or Space |
| Back/Exit | ESC |
| Toggle Fullscreen | F11 |
| Mouse | Hover + Click |

## Build Instructions

### Prerequisites
- C++11 compiler (g++, clang, MSVC)
- SDL2 and SDL2_ttf development libraries

---

### Linux (Arch/Manjaro/Hyprland)

```bash
# Install dependencies
sudo pacman -S sdl2 sdl2_ttf

# Compile
g++ -o gamw main.cpp Menu.cpp -lSDL2 -lSDL2_ttf -lm -std=c++11

# Run
./gamw
```

### Linux (Debian/Ubuntu)

```bash
# Install dependencies
sudo apt install libsdl2-dev libsdl2-ttf-dev

# Compile
g++ -o gamw main.cpp Menu.cpp -lSDL2 -lSDL2_ttf -lm -std=c++11

# Run
./gamw
```

---

### Windows (MSYS2 + MinGW-w64 + VS Code)

This is the recommended setup for Windows development with a Unix-like environment.

#### Step 1: Install MSYS2

1. Download and install MSYS2 from https://www.msys2.org/
2. Run MSYS2 MinGW64 terminal (NOT MSYS2 MSYS)
3. Update the package database:
   ```bash
   pacman -Syu
   ```
4. Close and reopen the terminal if prompted, then update again:
   ```bash
   pacman -Su
   ```

#### Step 2: Install Development Tools

In the MSYS2 MinGW64 terminal:

```bash
# Install compiler and build tools
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-gdb mingw-w64-x86_64-make

# Install SDL2 libraries
pacman -S mingw-w64-x86_64-SDL2 mingw-w64-x86_64-SDL2_ttf
```

#### Step 3: Configure VS Code

1. Install VS Code from https://code.visualstudio.com/
2. Install the C/C++ extension (ms-vscode.cpptools)
3. Add MinGW64 to your system PATH:
   - Default location: `C:\msys64\mingw64\bin`
   - Add this to your Windows environment variables

4. Create `.vscode/tasks.json` in your project folder:

```json
{
    "version": "2.0.0",
    "tasks": [
        {
            "label": "Build Gamw",
            "type": "shell",
            "command": "g++",
            "args": [
                "-o",
                "gamw.exe",
                "main.cpp",
                "Menu.cpp",
                "-lmingw32",
                "-lSDL2main",
                "-lSDL2",
                "-lSDL2_ttf",
                "-lm",
                "-std=c++11",
                "-O2"
            ],
            "group": {
                "kind": "build",
                "isDefault": true
            },
            "problemMatcher": ["$gcc"]
        }
    ]
}
```

5. Create `.vscode/launch.json` for debugging:

```json
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "Debug Gamw",
            "type": "cppdbg",
            "request": "launch",
            "program": "${workspaceFolder}/gamw.exe",
            "args": [],
            "stopAtEntry": false,
            "cwd": "${workspaceFolder}",
            "environment": [],
            "externalConsole": false,
            "MIMode": "gdb",
            "miDebuggerPath": "C:/msys64/mingw64/bin/gdb.exe",
            "setupCommands": [
                {
                    "description": "Enable pretty-printing for gdb",
                    "text": "-enable-pretty-printing",
                    "ignoreFailures": true
                }
            ],
            "preLaunchTask": "Build Gamw"
        }
    ]
}
```

#### Step 4: Build and Run

In VS Code:
- Press `Ctrl+Shift+B` to build
- Press `F5` to build and debug
- Or use the terminal: `g++ -o gamw.exe main.cpp Menu.cpp -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf -lm -std=c++11`

#### Troubleshooting Windows Build

If you get "SDL2 not found" errors:
```bash
# Verify SDL2 installation
pacman -Qs SDL2

# Reinstall if needed
pacman -S mingw-w64-x86_64-SDL2 mingw-w64-x86_64-SDL2_ttf
```

If the executable crashes immediately:
- Make sure you're running from MSYS2 MinGW64 terminal, not CMD or PowerShell
- Or copy SDL2.dll and SDL2_ttf.dll from `C:\msys64\mingw64\bin` to your project folder

---

### macOS

```bash
# Install dependencies
brew install sdl2 sdl2_ttf

# Compile
g++ -o gamw main.cpp Menu.cpp -lSDL2 -lSDL2_ttf -lm -std=c++11

# Run
./gamw
```

---

## Project Structure

```
gamw/
├── main.cpp          # Game loop and state management
├── Menu.cpp          # Menu implementation with animations
├── Menu.h            # Menu class definition
├── .vscode/          # VS Code configuration (Windows)
│   ├── tasks.json
│   └── launch.json
├── assets/
│   └── fonts/        # Font files (optional, uses system fonts as fallback)
└── README.md
```

## Menu Features

### Visual Effects
- Gradient background - Smooth dark blue to purple transition
- Animated starfield - 100 parallax stars with varied brightness
- Pulsing title - Cyan glow effect on "GAMW" title
- Selection animations - Smooth easing with cubic interpolation
- Arrow indicators - Animated side arrows that pulse when selected
- Glow effects - Inner borders on selected items

### Menu Options
1. START GAME - Launch single player
2. HOST SERVER - Start multiplayer server
3. JOIN SERVER - Connect to multiplayer
4. SETTINGS - Configuration options
5. QUIT - Exit game

## Technical Details

- Rendering: Hardware-accelerated SDL2 renderer with VSync
- Frame Rate: Locked 60 FPS with delta time calculations
- Input Handling: 150ms key repeat delay for smooth navigation
- Window Management: Dynamic resolution with fullscreen support
- Font System: Multiple fallback paths for cross-platform compatibility

## Wayland Compatibility

Fully tested on:
- Hyprland
- Sway
- GNOME Wayland
- X11 (backwards compatible)

## Future Plans

- Actual gameplay implementation
- Network multiplayer functionality
- Settings menu (audio, graphics, controls)
- Save/load system
- Controller support

## License

Do whatever you want with it. No restrictions.

---

**Now go build something cool.**