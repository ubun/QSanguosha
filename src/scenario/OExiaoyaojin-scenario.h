#ifndef OEXIAOYAOJINSCENARIO_H
#define OEXIAOYAOJINSCENARIO_H

#include "scenario.h"
#include "card.h"

class Skill;
class Card;
class Player;

#include <QObject>
#include <QHash>
#include <QStringList>
#include <QMap>

class PozhenCard:public SkillCard{
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

class OEXiaoyaojinScenario: public Scenario{
    Q_OBJECT

public:
    explicit OEXiaoyaojinScenario();

    virtual void getRoles(char *roles) const;

    virtual void onTagSet(Room *room, const QString &key) const;
};
#endif // OEXIAOYAOJINSCENARIO_H
