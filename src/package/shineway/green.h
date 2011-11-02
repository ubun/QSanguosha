#ifndef GREEN_H
#define GREEN_H

#include "package.h"
#include "card.h"
#include "standard.h"

class YuanlvCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE YuanlvCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class ZhongjianCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ZhongjianCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class GreenPackage: public Package{
    Q_OBJECT

public:
    GreenPackage();
};

#endif // GREEN_H
