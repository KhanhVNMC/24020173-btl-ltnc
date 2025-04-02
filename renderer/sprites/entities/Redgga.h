#ifndef REDGGA_H
#define REDGGA_H
#include "normal_entity.h"

class Redgga final : public NormalEntity {
public:
    Redgga() : NormalEntity() {
        this->setTextureFile("../assets/enemy_01.bmp"); // red nigga
        this->setDamageThresholds(3, 5); // 3-5 dmg
        this->setAttackDelaySeconds(15); // 15s between attacks
    }
};

#endif //REDGGA_H
