#ifndef GOLDSEINTO_H
#define GOLDSEINTO_H

#include "package.h"
#include "card.h"

class ZhuyiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ZhuyiCard();
    void onEffect(const CardEffectStruct &effect) const;
};

class JiaohuangCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE JiaohuangCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
};

class HuanlongCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE HuanlongCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class BaolunCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE BaolunCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class ZhiyanCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ZhiyanCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class ShengjianCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ShengjianCard();
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class GoldSeintoPackage: public Package{
    Q_OBJECT

public:
    GoldSeintoPackage();
};

#endif
