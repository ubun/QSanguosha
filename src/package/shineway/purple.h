#ifndef PURPLE_H
#define PURPLE_H

#include "package.h"
#include "card.h"
#include "standard.h"

class ShouguoCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ShouguoCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class YaofaCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE YaofaCard();

    virtual void onEffect(const CardEffectStruct &effect) const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
};

class PurplePackage: public Package{
    Q_OBJECT

public:
    PurplePackage();
};

#endif // PURPLE_H
