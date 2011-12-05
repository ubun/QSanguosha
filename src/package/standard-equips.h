#ifndef STANDARDEQUIPS_H
#define STANDARDEQUIPS_H

#include "standard.h"

class Crossbow:public Weapon{
    Q_OBJECT

public:
    Q_INVOKABLE Crossbow(Card::Suit suit, int number = 1);
};

class StandardCardPackage: public Package{
    Q_OBJECT

public:
    StandardCardPackage();
};

#endif // STANDARDEQUIPS_H
