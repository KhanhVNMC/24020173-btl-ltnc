//
// Created by GiaKhanhVN on 2/23/2025.
//

#ifndef DISK_CACHE_H
#define DISK_CACHE_H
#include <SDL.h>
#include <iostream>
#include <unordered_map>

using namespace std;

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
