#include <vector>
#include "sdl_components.h"
#include "../engine/javalibs/jsystemstd.h"

namespace disk_cache {
    // BMP Cache
    static unordered_map<string, SDL_Texture*> cache;

    /**
     * Load RAMDISK based BMP files
     *
     * @param renderer the associated renderer
     * @param filename the file name to load
     * @return
     */
    SDL_Texture* bmp_load_and_cache(SDL_Renderer* renderer, const string& refName) {
        string filename = std::string(ASSETS_FOLDER) + "/" + refName;
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

    // preload
    std::vector<std::string> spriteSheets = {
            MAIN_SPRITE_SHEET,
            GAME_LOGO_SHEET,
            FONT_SHEET,
            PLAYER_SHEET,
            BACKGROUND_SHEET,
            TETROMINOES,
            PARALLAX_SHEET,
            REDGGA_SHEET,
            BLUGGA_SHEET,
            GRIGGA_SHEET,
            NIGGA_SHEET,
            FAIRY_DISTRACTOR_SHEET,
            FAIRY_BLINDER_SHEET,
            FAIRY_DISTURBER_SHEET,
            FAIRY_WEAKENER_SHEET
    };
    void preload_defined_sheets(SDL_Renderer* renderer) {
        for (std::string sheet : spriteSheets) {
            if (bmp_load_and_cache(renderer, sheet) == nullptr) {
                SDL_ShowSimpleMessageBox(
                        SDL_MESSAGEBOX_ERROR,
                        "Tetris VS: Engine Error",
                        ("Required game asset not found or failed to load:\n\"" + sheet + "\"\n\nClick OK to exit the game").c_str(),
                        nullptr
                );
                exit(12); // missing sprites
            }
        }
    }
}
