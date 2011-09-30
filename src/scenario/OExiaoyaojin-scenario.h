#ifndef OEXIAOYAOJINSCENARIO_H
#define OEXIAOYAOJINSCENARIO_H

#include "scenario.h"
#include "card.h"

class OEXiaoyaojinScenario: public Scenario{
    Q_OBJECT

public:
    explicit OEXiaoyaojinScenario();

    virtual void getRoles(char *roles) const;

    virtual void onTagSet(Room *room, const QString &key) const;
};
#endif // OEXIAOYAOJINSCENARIO_H
