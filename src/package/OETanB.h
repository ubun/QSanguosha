#ifndef OETANB_H
#define OETANB_H

#include "package.h"
#include "card.h"
#include "standard.h"
#include "generaloverview.h"

class GuayanCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE GuayanCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};
/*
class TuiyinCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE TuiyinCard();

    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};
*/
class OETanBPackage : public Package
{
    Q_OBJECT

public:
    OETanBPackage();
};

#endif // OETANB_H
