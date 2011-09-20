#ifndef GOLDSEINTO_H
#define GOLDSEINTO_H

#include "package.h"
#include "card.h"

class XiufuCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE XiufuCard();
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const;
};

class Huan2longCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE Huan2longCard();

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
/*class BaolunCard: public SkillCard{
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
*/

class GoldSeintoPackage: public Package{
    Q_OBJECT

public:
    GoldSeintoPackage();
};

#endif
