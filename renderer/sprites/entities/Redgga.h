#ifndef REDGGA_H
#define REDGGA_H
#include "normal_entity.h"

class Redgga final : public NormalEntity {
public:
    Redgga() : NormalEntity() {
        this->setTextureFile("../assets/enemy_01.bmp");
    }
};

#endif //REDGGA_H
