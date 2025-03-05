//
// Created by GiaKhanhVN on 3/5/2025.
//
#include "../spritesystem/sprite.h"
#include "../sdl_components.h"

class RenComSprite : public Sprite {
     explicit RenComSprite(struct_render_component rc, const string& texturePath)
     : Sprite(nullptr, 0, 0, 0) {
        this->setupTexture(new SpriteTexture{ rc.source.x, rc.source.y, rc.source.w, rc.source.h }, texturePath);
        this->x = rc.dest.x;
        this->y = rc.dest.y;
        this->width = rc.dest.w;
        this->height = rc.dest.h;
    }

    ~RenComSprite() override {
        delete this->texture;
    }
};