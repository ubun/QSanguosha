#ifndef LIXIANJI_H
#define LIXIANJI_H

#include "package.h"
#include "card.h"
#include "standard.h"
#include "generaloverview.h"

class XJfengliuCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE XJfengliuCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class XJjielveCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE XJjielveCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class XJduwuCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE XJduwuCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class XJzizhuCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE XJzizhuCard();

    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};
/*
class XJyizhongCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE XJyizhongCard();

    virtual void onEffect(const CardEffectStruct &effect) const;
};
*/
class LiXianJiPackage : public Package
{
    Q_OBJECT

public:
    LiXianJiPackage();
};
#endif // LIXIANJI_H
