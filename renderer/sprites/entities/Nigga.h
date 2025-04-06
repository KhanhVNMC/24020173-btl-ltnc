//
// Created by GiaKhanhVN on 4/7/2025.
//

#ifndef NIGGA_H
#define NIGGA_H
#include "normal_entity.h"

class Nigga final : public NormalEntity {
public:
    Nigga(const void* tetrisPlayer) : NormalEntity(tetrisPlayer) {
        this->setTextureFile("../assets/enemy_04.bmp"); // very strong mob
        this->setDamageThresholds(3, 6); // 3-6 dmg
        this->setAttackSpeed(15); // 20s between attacks
        this->setDifficulty(HARD); // this is a hard boss
        this->setMaxHealth(20); // 20HP (5 tetris(s) to clear)
    }
};
#endif //NIGGA_H
