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

#endif // KUSO_H
