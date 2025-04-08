//
// Created by GiaKhanhVN on 4/8/2025.
//

#ifndef TETISENGINE_REAL_LOADING_SCREEN_H
#define TETISENGINE_REAL_LOADING_SCREEN_H

#include <SDL_render.h>
#include <cmath>
#include "../gamescene.h"
#include "../hooker.h"
#include "../../renderer/sdl_components.h"
#include "../../renderer/spritesystem/sprite.h"
#include "menu_btn.h"
#include "../../engine/javalibs/jsystemstd.h"

class GameLoadingScreen : public GameScene {
    ExecutionContext* context;
    SDL_Renderer* renderer;

    int hookId;

    public: GameLoadingScreen(ExecutionContext* executionContext, SDL_Renderer* sdlRenderer) {
        this->context = executionContext;
        this->renderer = sdlRenderer;
    }

    int clock;
    function<void()> onLoadingScreenInit = nullptr;
    function<void(ExecutionContext*, SDL_Renderer*)> onLoadingScreenComplete = nullptr;
    int fakeLoadFor = 60; // 60 frames/1s

    SDL_Texture* cachedTexture = nullptr;
    void menuLoop() {
        // stop loading screen
        if (fakeLoadFor <= 0) {
            this->stopScene();
            return;
        }
        // render background
        if (cachedTexture == nullptr) cachedTexture = disk_cache::bmp_load_and_cache(renderer, "../assets/load_scr.bmp");
        const struct_render_component component = {
                0, 0, 1720, 860,
                0, 0, 1720, 860
        };
        render_component(renderer, cachedTexture, component, 0.65 + (cos(clock++ / 35.0) * 0.35));
        // render the loading text
        Button::renderString(renderer, 1200, 750 + (sin(clock++ / 25.0) * 20), "loading...", 4, 1, 45);
        // decrement IC
        fakeLoadFor--;
    }

    void stopScene() override {
        context->unhook(this->hookId, [&]() {
            onLoadingScreenComplete(this->context, this->renderer);
            delete this;
        });
    }

    void startScene() override {
        // clean current rendering context to begin a new life
        SpritesRenderingPipeline::stopAndCleanCurrentContext();
        this->hookId = context->hook([&]() { onContextTick(); });
        if (onLoadingScreenInit) onLoadingScreenInit();
    }

    void onContextTick() {
        // begin render the scene
        SDL_RenderClear(renderer);
        this->menuLoop();
        Thread::sleep(16);
        // what the fuck
        SDL_RenderPresent(renderer); // Show updated frame
    }
};

#endif //TETISENGINE_LOADING_SCREEN_H
