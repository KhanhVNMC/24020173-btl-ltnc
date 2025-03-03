//
// Created by GiaKhanhVN on 2/26/2025.
//

#include "../spritesystem/sprite.h"
#include <cmath>

class FlandreScarlet final : public Sprite {
public:
    explicit FlandreScarlet(const SDL_RendererFlip flip, const int width = 50, const int height = 50, const int initialRotation = 0): Sprite(nullptr, width, height, initialRotation) {
        this->setupTexture(new SpriteTexture{ 0, 41, 110, 93 }, "../assets/flandre.bmp");
        this->x = 100;
        this->y = 100;
        this->scale(5);
        this->flipSprite(flip);
    }

    int calls = 0;
    int frames = 0;
    void onDrawCall() override {
        this->texture->textureX = this->originalTextureX + (128 * calls);
        //this->teleport(x, 100 + (sin(frames / 20.0) * 40));
        if (frames % 5 == 0) {
            calls = (calls + 1) % 8;
        }
        ++frames;
    }


    ~FlandreScarlet() override {
        delete this->texture;
    }
};

class BackgroundScroll final : public Sprite {
private:
    int spriteScrollSpeed = 2;
public:
    explicit BackgroundScroll(const SDL_RendererFlip flip, int x, int y, const int spriteScrollSpeed = 2, const int width = 512, const int height = 256, const int initialRotation = 0): Sprite(nullptr, width, height, initialRotation) {
        this->setupTexture(new SpriteTexture{ 0, 0, 512, 256 }, "../assets/starlight.bmp");
        this->x = x;
        this->y = y;
        this->spriteScrollSpeed = spriteScrollSpeed;
        this->scale(4);
        this->flipSprite(flip);
    }

    int calls = 0;
    int frames = 0;
    void onDrawCall() override {
        this->x -= spriteScrollSpeed;
        // if the starmap scrolls pass its ending (edge), snap it back to its "offscreen-right" location
        if (x == -texture->width * scalar) {
            x = texture->width * scalar;
        }
    }


    ~BackgroundScroll() override {
        delete this->texture;
    }
};