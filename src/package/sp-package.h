#ifndef SPPACKAGE_H
#define SPPACKAGE_H

#include "package.h"
#include "card.h"
#include "standard.h"
#include "generaloverview.h"

class SPPackage: public Package{
    Q_OBJECT

public:
    SPPackage();
};

class PozhenCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE PozhenCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class xiaozhihengCard:public SkillCard{

public:

    Q_INVOKABLE xiaozhihengCard();
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};



class taoyuanjieyiDialog: public GeneralOverview{
    Q_OBJECT

public:
    taoyuanjieyiDialog();

public slots:
    void popup();
};

class SPCardPackage: public Package{
    Q_OBJECT

public:
    SPCardPackage();
};

#endif // SPPACKAGE_H
