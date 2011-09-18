#ifndef RED_H
#define RED_H

#include "package.h"
#include "card.h"
#include "standard.h"

class TongmouCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE TongmouCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class XianhaiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE XianhaiCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class BaichuCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE BaichuCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class RedPackage: public Package{
    Q_OBJECT

public:
    RedPackage();
};

#endif // RED_H
