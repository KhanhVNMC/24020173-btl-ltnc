//
// Created by GiaKhanhVN on 2/26/2025.
//
#include <functional>
#include "../spritesystem/sprite.h"

/**
 * The star-night background (overlay on top of each other for parallax effect)
 */
class BackgroundScroll final : public Sprite {
private:
    int spriteScrollSpeed = 2;
public:
    bool scroll = false;
    explicit BackgroundScroll(const SDL_RendererFlip flip, int x, int y, const int spriteScrollSpeed = 2, const int width = 512, const int height = 256, const int initialRotation = 0):
    Sprite({ 0, 0, 512, 256 }, width, height, initialRotation) {
        this->setTextureFile("../assets/starlight.bmp");
        this->x = x; this->y = y;
        this->spriteScrollSpeed = spriteScrollSpeed;
        this->scale(4);
        this->flipSprite(flip);
    }

    void onDrawCall() override {
        this->x -= spriteScrollSpeed - (!scroll ? 1 : 0);
        // if the starmap scrolls pass its ending (edge), snap it back to its "offscreen-right" location
        if (x <= -texture.width * scalar) {
            x = static_cast<int>(texture.width * scalar);
        }
    }

    ~BackgroundScroll() override = default;
};