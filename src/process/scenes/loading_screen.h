//
// Created by GiaKhanhVN on 4/8/2025.
//

#ifndef TETISENGINE_LOADING_SCREEN_H
#define TETISENGINE_LOADING_SCREEN_H

#include <SDL_render.h>
#include <cmath>
#include "../gamescene.h"
#include "../hooker.h"
#include "menu_btn.h"
#include "../../engine/javalibs/jsystemstd.h"
#include "../../game/sdl_components.h"
#include "../../game/spritesystem/sprite.h"

class LoadingScreen : public GameScene {
    ExecutionContext* context;
    SDL_Renderer* renderer;

    int hookId;

    public: LoadingScreen(ExecutionContext* executionContext, SDL_Renderer* sdlRenderer) {
        this->context = executionContext;
        this->renderer = sdlRenderer;
    }

    function<void()> onLoadingScreenInit = nullptr;
    function<void(ExecutionContext*, SDL_Renderer*)> onLoadingScreenComplete = nullptr;
    int fakeLoadFor = 60; // 60 frames/1s

    int icClock;
    void renderWavyString(float centerX, float centerY, const std::string& text, int scale, int strgap) {
        float startX = centerX - (text.length() * strgap) / 2.0f;
        for (size_t i = 0; i < text.length(); ++i) {
            float x = startX + i * strgap;
            float y = centerY + sin(i + ((icClock++) / 20) * 0.25) * 5;

            Button::renderString(renderer, x, y, std::string(1, text[i]), scale, 1, 0);
        }
    }

    SDL_Texture* cachedTexture = nullptr;
    int pulseClock = 63; // extrema (lowest)
    void menuLoop() {
        // stop loading screen
        if (fakeLoadFor <= 0) {
            this->stopScene();
            return;
        }
        // render background
        if (cachedTexture == nullptr) cachedTexture = disk_cache::bmp_load_and_cache(renderer, BACKGROUND_SHEET);
        const struct_render_component component = {
                0, 0, 1720, 860,
                0, 0, 1720, 860
        };
        render_component(renderer, cachedTexture, component, 0.65 + (cos(pulseClock++ / 20.0) * 0.35));
        renderWavyString(1200 + 230, 750, "loading...", 4, 45);
        // decrement IC
        fakeLoadFor--;
    }

    void stopScene() override {
        context->unhook(this->hookId, [this]() {
            onLoadingScreenComplete(this->context, this->renderer);
            delete this;
        });
    }

    void startScene() override {
        // clean current rendering context to begin a new life
        SpritesRenderingPipeline::stopAndCleanCurrentContext();
        this->hookId = context->hook([&]() { onContextTick(); }, "Loading Screen");
        if (onLoadingScreenInit) onLoadingScreenInit();
    }

    void onContextTick() {
        // begin render the scene
        SDL_RenderClear(renderer);
        this->menuLoop();
        Thread::sleep(16); // 60.0FPS, std of the thing
        // what the fuck
        SDL_RenderPresent(renderer); // Show updated frame
    }
};

#endif //TETISENGINE_LOADING_SCREEN_H
