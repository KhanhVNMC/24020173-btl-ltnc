//
// Created by GiaKhanhVN on 2/26/2025.
//

#include "../spritesystem/sprite.h"
class BigBlackBox final : public Sprite {
public:
    explicit BigBlackBox(const int width = 50, const int height = 50, const int initialRotation = 0): Sprite(nullptr, width, height, initialRotation) {
        this->texture = new SpriteTexture{ 0, 0, 78, 96 };
        this->originalTextureX = texture->textureX;
        this->originalTextureY = texture->textureY;
        this->x = 100;
        this->y = 100;
        this->scale(1);
    }

    int dx = 5, dy = 5;
    void onDrawCall() override {
        setDirection(300, 0);
    }


    ~BigBlackBox() override {
        delete this->texture;
    }
};