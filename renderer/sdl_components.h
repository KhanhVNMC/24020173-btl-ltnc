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
inline char CHAR_LIST[69] = {
        ' ', '!', '"', '#', '$', '%', '&', '\'', '(', ')', '*', '+', ',', '-', '.',
        '/', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ':', ';', '<', '=',
        '>', '?', '@','a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j','k', 'l','m','n',
        'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '[',
        '\\', ']', '^', '_', '{', '|', '}', '~', '`'
};

inline std::unordered_map<char, int> CHAR_MAP;
// initialize the font system before use
inline void initFontSystem() {
    for (size_t i = 0; i < 69; i++) {
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

/**
 * Render the given "struct_render_component", this will render a static sprite
 * for dynamically maneuverable sprites, use {@link Sprite} (Heap allocated)
 *
 * @param renderer the target SDL renderer
 * @param texture the texture (get from CACHE, BMP)
 * @param component the struct_render_component
 * @param opacity opacity from 0.0 - 1.0 (0: transparent; 1: opaque)
 */
inline void render_component(SDL_Renderer* renderer, SDL_Texture* texture, const struct_render_component& component, const float opacity, const int angle = 0) {
    SDL_SetTextureAlphaMod(texture, static_cast<Uint8>(opacity * 255));
    if (angle > 0) {
        SDL_RenderCopyEx(renderer, texture, &component.source, &component.dest, angle, nullptr, SDL_FLIP_NONE);
        return;
    }
    SDL_RenderCopy(renderer, texture, &component.source, &component.dest);
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
        cout << "[DEBUG] Loaded texture into VRAM " << filename << "" << endl;

        return texture;
    }
}

inline static std::string str_printf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    int size = std::vsnprintf(nullptr, 0, format, args);
    va_end(args);
    if (size < 0) {
        return "";
    }
    std::vector<char> buffer(size + 1);
    va_start(args, format);
    std::vsnprintf(buffer.data(), buffer.size(), format, args);
    va_end(args);
    return std::string(buffer.data());
}
#endif //DISK_CACHE_H
