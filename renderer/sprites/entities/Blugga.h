#ifndef BLUGGA_H
#define BLUGGA_H
#include "normal_entity.h"

class Blugga final : public NormalEntity {
public:
    Blugga(const void* tetrisPlayer) : NormalEntity(tetrisPlayer) {
        this->setTextureFile("../assets/enemy_02.bmp"); // blue nigga
        this->setDamageThresholds(1, 4); // 1-4 dmg
        this->setAttackSpeed(30); // 30s between attacks
        this->setDifficulty(MEDIUM); // this is a medium mob
        this->setMaxHealth(12); // 12HP (3 tetris(s) to clear)
    }
};

#endif //BLUGGA_H
