#ifndef GOLDSEINTOVIVA_H
#define GOLDSEINTOVIVA_H

#include "package.h"
#include "card.h"

class XingmieCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE XingmieCard();
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class HaojiaoCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE HaojiaoCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class HuanlongCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE HuanlongCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class ShiqiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ShiqiCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class BaolunCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE BaolunCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
};

class HongzhenCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE HongzhenCard();

    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
};

class ShengjianCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ShengjianCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class GoldSeintoViVAPackage: public Package{
    Q_OBJECT

public:
    GoldSeintoViVAPackage();
};

#endif
