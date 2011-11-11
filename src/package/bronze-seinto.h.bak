#ifndef BRONZE_H
#define BRONZE_H

#include "package.h"
#include "card.h"

class ShenglongCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ShenglongCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class XingyunCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE XingyunCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class HuanmoCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE HuanmoCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class BronzeSeintoPackage: public Package{
    Q_OBJECT

public:
    BronzeSeintoPackage();
};

#endif
