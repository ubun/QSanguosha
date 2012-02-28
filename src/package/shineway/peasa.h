#ifndef PEASAPACKAGE_H
#define PEASAPACKAGE_H

#include "package.h"
#include "card.h"
#include "generaloverview.h"

class PeasaPackage: public Package{
    Q_OBJECT

public:
    PeasaPackage();
};

class GuiouCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE GuiouCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class ZhonglianCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ZhonglianCard();

    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
};

class MingwangCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE MingwangCard();

    virtual void onEffect(const CardEffectStruct &effect) const;
};

class GuishuDialog: public GeneralOverview{
    Q_OBJECT

public:
    GuishuDialog();

public slots:
    void popup();
};

class YuguiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE YuguiCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
};

#endif // PEASAPACKAGE_H
