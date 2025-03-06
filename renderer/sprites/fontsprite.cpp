//
// Created by GiaKhanhVN on 3/5/2025.
//
#include "../spritesystem/sprite.h"
#include "../sdl_components.h"

class RenComSprite : public Sprite {
     explicit RenComSprite(struct_render_component rc, const string& texturePath)
     : Sprite({ rc.source.x, rc.source.y, rc.source.w, rc.source.h }, 0, 0, 0) {
        this->setTextureFile(texturePath);
        this->x = rc.dest.x;
        this->y = rc.dest.y;
        this->width = rc.dest.w;
        this->height = rc.dest.h;
    }

    ~RenComSprite() override {}
};