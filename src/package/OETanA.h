#ifndef OETANA_H
#define OETANA_H

#include "package.h"
#include "card.h"
#include "standard.h"
#include "generaloverview.h"

class MaimengCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE MaimengCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
};

class ShaobingCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ShaobingCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class DuihuanCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE DuihuanCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class BaiheCard:public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE BaiheCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class OETanAPackage : public Package
{
    Q_OBJECT

public:
    OETanAPackage();
};

#endif // OETANA_H
