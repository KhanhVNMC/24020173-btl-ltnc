//
// Created by GiaKhanhVN on 2/26/2025.
//

#include "../spritesystem/sprite.h"
class BigBlackBox final : public Sprite {
public:
    explicit BigBlackBox(const int width = 50, const int height = 50, const int initialRotation = 0): Sprite(nullptr, width, height, initialRotation) {
        this->texture = new SpriteTexture{ 12, 12, 64, 64 };
        this->originalTextureX = texture->textureX;
        this->originalTextureY = texture->textureY;
        this->x = 100;
        this->y = 100;
        this->scale(4);
    }

    int dx = 2, dy = 2;
    void onDrawCall() override {
        x += dx;
        y += dy;
        if (x <= 0 || x + width >= 1080) {
            this->scale(scalar + 0.05);
            dx = -dx;
        }
        if (y <= 0 || y + height >= 680) {
            this->scale(scalar - 0.05);
            dy = -dy;
        }
        this->teleport(x, y);
        this->rotate(rotationState + 1);
    }


    ~BigBlackBox() override {
        delete this->texture;
    }
};