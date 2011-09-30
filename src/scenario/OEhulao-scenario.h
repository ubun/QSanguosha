#ifndef OEHULAOSCENARIO_H
#define OEHULAOSCENARIO_H

#include "scenario.h"
#include "card.h"

class OEhulaoScenario: public Scenario{
    Q_OBJECT

public:
    explicit OEhulaoScenario();

    virtual void getRoles(char *roles) const;

    virtual void onTagSet(Room *room, const QString &key) const;
};
#endif // OEHULAOSCENARIO_H
