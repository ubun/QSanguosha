#ifndef RED_H
#define RED_H

#include "package.h"
#include "card.h"
#include "standard.h"

class XianjuCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE XianjuCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class BaichuCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE BaichuCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class TongluCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE TongluCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class XiefangCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE XiefangCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
};

class GoulianCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE GoulianCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class AmaterasuCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE AmaterasuCard();

    virtual void onEffect(const CardEffectStruct &effect) const;
};

class SusaCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE SusaCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class RedPackage: public Package{
    Q_OBJECT

public:
    RedPackage();
};

#endif // RED_H
