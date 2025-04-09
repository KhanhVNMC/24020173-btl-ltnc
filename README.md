# BTL LTNC 2025  
**Modern Tetris VS with Challenging PvE Elements**<br>
This game is not officially licensed by the Tetris® Holdings. For education usage only!

## Branding Legal Notice
Tetris® & © 1985 ~ 2025 Tetris Holding.

Tetris logos, Tetris theme song and Tetriminos are trademarks of Tetris Holding.
The Tetris trade dress is owned by Tetris Holding.
Licensed to The Tetris Company.

Tetris Game Design by Alexey Pajitnov.
Tetris Logo Design by Roger Dean.

All Rights Reserved.

## Project Overview  
This project offers a modern take on competitive Tetris (VS mode), enriched with dynamic Player vs Environment (PvE) elements. Departing from traditional Tetris formats, it introduces interactive enemy mechanics alongside contemporary gameplay features such as the Super Rotation System (SRS), Auto-Repeat Rate (ARR), Delayed Auto Shift (DAS), Hold system, Seven-Bag Generator and more. These enhancements create a more engaging and fun experience for players (not so sure about this)

## How To Run
You may either download the **precompiled executable** along with necessary game assets from the GitHub Releases tab or build the project manually using CMake

### How to Build and Run This Project

This project uses CMake for its build system and assumes you have a compatible C++ compiler installed

#### **1. Prerequisites**
- **CMake** (version 3.10 or later)
- **C++ Compiler** (e.g., `g++`, `clang++`, or MSVC)
- **SDL2** and **SDL2_mixer** development libraries (headers and import libraries)

#### **2. Building the Project**

Open a terminal or command prompt and follow these steps:

```bash
cd /path/to/project
mkdir build
cd build
cmake ..
# build the thing
cmake --build . 
```

This will produce the executable in the `build` directory.

#### **3. Running the Application**

To run the built application successfully, ensure the following dynamic libraries are present in the same directory as the executable:

- `SDL2.dll`
- `SDL2_mixer.dll`

These files are required at runtime. You can obtain them from the official [SDL2](https://github.com/libsdl-org/SDL/releases/tag/release-3.2.10) and [SDL2_mixer](https://github.com/libsdl-org/SDL_mixer/releases/tag/release-2.8.1) distribution websites

## Reference Source(s)

**Tetris Design Guidelines (2009):**
[Tetris Guideline 2009 – Archive.org](https://archive.org/details/2009-tetris-variant-concepts_202201/2009%20Tetris%20Design%20Guideline/page/30/mode/2up?view=theater)

---
© 2025 GiaKhanhVN. All rights reserved.
