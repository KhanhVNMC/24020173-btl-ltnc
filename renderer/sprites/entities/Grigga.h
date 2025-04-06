//
// Created by GiaKhanhVN on 4/7/2025.
//

#ifndef GRIGGA_H
#define GRIGGA_H
#include "normal_entity.h"

class Grigga final : public NormalEntity {
public:
    Grigga(const void* tetrisPlayer) : NormalEntity(tetrisPlayer) {
        this->setTextureFile("../assets/enemy_03.bmp"); // blue nigga
        this->setDamageThresholds(1, 2); // 1-2 dmg
        this->setAttackSpeed(30); // 30s between attacks
        this->setDifficulty(EASY); // this is a easy mob
        this->setMaxHealth(4); // 4HP (1 tetris to clear)
    }
};

#endif //GRIGGA_H
