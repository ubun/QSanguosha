#ifndef KUSO_H
#define KUSO_H

#include "package.h"
#include "card.h"
#include "standard.h"

class LiaotingCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE LiaotingCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class KusoPackage: public Package{
    Q_OBJECT

public:
    KusoPackage();
};

//cards
class KawaiiDress: public Armor{
    Q_OBJECT

public:
    Q_INVOKABLE KawaiiDress(Card::Suit suit, int number);

    virtual void onUninstall(ServerPlayer *player) const;
};

class Fiveline: public Armor{
    Q_OBJECT

public:
    Q_INVOKABLE Fiveline(Card::Suit suit, int number);

    virtual void onInstall(ServerPlayer *player) const;
    virtual void onUninstall(ServerPlayer *player) const;
};

class KusoCardPackage: public Package{
    Q_OBJECT

public:
    KusoCardPackage();
};

#endif // KUSO_H
