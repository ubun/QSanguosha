#ifndef GOLDSEINTO_H
#define GOLDSEINTO_H

#include "package.h"
#include "card.h"

class XiufuCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE XiufuCard();
    virtual void onEffect(const CardEffectStruct &effect) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class HuanlongCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE HuanlongCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class YinheCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE YinheCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class MohuaCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE MohuaCard();

    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class LianluoCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE LianluoCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class Sheng2jianCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE Sheng2jianCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};


class GoldSeintoPackage: public Package{
    Q_OBJECT

public:
    GoldSeintoPackage();
};

#endif
