#ifndef OEXIAPISCENARIO_H
#define OEXIAPISCENARIO_H

#include "scenario.h"
#include "card.h"

class OExiapiScenario: public Scenario{
    Q_OBJECT

public:
    explicit OExiapiScenario();

    virtual void getRoles(char *roles) const;

    virtual void onTagSet(Room *room, const QString &key) const;
};

#endif // OEXIAPISCENARIO_H
