//
// Created by GiaKhanhVN on 2/23/2025.
//

#ifndef DISK_CACHE_H
#define DISK_CACHE_H
#include <SDL.h>
#include <iostream>
#include <unordered_map>

using namespace std;

#define TETROMINOES "../assets/tetrominoes.bmp"
#define FONT_SHEET "../assets/font.bmp"

// begin text section
inline char CHAR_LIST[68] = {
        ' ', '!', '"', '#', '$', '%', '&', '\'', '(', ')', '*', '+', ',', '-', '.',
        '/', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ':', ';', '<', '=',
        '>', '?', '@','a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j','k', 'l','m','n',
        'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '[',
        '\\', ']', '^', '_', '{', '|', '}', '~'
};

inline std::unordered_map<char, int> CHAR_MAP;
// initialize the font system before use
inline void initFontSystem() {
    for (size_t i = 0; i < 68; i++) {
        CHAR_MAP[CHAR_LIST[i]] = static_cast<int>(i);
    }
}

/**
 * General purpose component used to put images
 * from RAM to VRAM
 */
typedef struct {
    SDL_Rect source;
    SDL_Rect dest;
} struct_render_component;

#define FONT_SHEET_X 1
#define FONT_SHEET_Y 2
#define FONT_SHEET_GAP 8

#define FONT_WIDTH 18
#define FONT_HEIGHT 14

/**
 * Puts a single character on the renderer's screen (GPU too)
 * @param x, y coordinates
 * @param scale the scalar
 * @param c the character to put
 * @param width
 * @return the struct ready for render
 */
inline struct_render_component puts_component_char(const int x, const int y, const double scale, const char c, const int width = 18) {
    const int sheetIndex = CHAR_MAP[c];
    const int sheetRow = (sheetIndex / 15);
    return {
            {FONT_SHEET_X + ((FONT_WIDTH + 2) * (sheetIndex % 15)), FONT_SHEET_Y + (sheetRow * (FONT_HEIGHT - 2 + FONT_SHEET_GAP)), FONT_WIDTH, FONT_HEIGHT},
            {x, y, static_cast<int>(width * scale), static_cast<int>(14 * scale)},
    };
}

namespace disk_cache {
    // BMP Cache
    inline static unordered_map<string, SDL_Texture*> cache;

    /**
     * Load RAMDISK based BMP files
     *
     * @param renderer the associated renderer
     * @param filename the file name to load
     * @return
     */
    static SDL_Texture* bmp_load_and_cache(SDL_Renderer* renderer, const string& filename) {
        if (cache.find(filename) != cache.end()) {
            return cache[filename];
        }

        // bmp as surface
        SDL_Surface* surface = SDL_LoadBMP(filename.c_str());
        if (!surface) {
            cerr << "[ASSETS ERROR] Failed to load BMP: " << SDL_GetError() << endl;
            return nullptr;
        }

        // convert to SDL-compliance texture
        SDL_SetColorKey(surface, SDL_TRUE, SDL_MapRGB(surface->format, 255, 0, 255));
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);

        if (!texture) {
            cerr << "[ASSETS ERROR] Failed to create texture: " << SDL_GetError() << endl;
            return nullptr;
        }

        // cache the bmp file up so that i can call it
        cache[filename] = texture;
        return texture;
    }
}
#endif //DISK_CACHE_H
