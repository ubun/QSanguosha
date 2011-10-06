#ifndef OEHUANGJINSCENARIO_H
#define OEHUANGJINSCENARIO_H

#include "scenario.h"
#include "card.h"
#include "generaloverview.h"

class taoyuanjieyiDialog:public GeneralOverview{
    Q_OBJECT

public:
    taoyuanjieyiDialog();

public slots:
    void popup();
};

class OEhuangjinScenario: public Scenario{
    Q_OBJECT

public:
    explicit OEhuangjinScenario();

    virtual void getRoles(char *roles) const;

    virtual void onTagSet(Room *room, const QString &key) const;
};
#endif // OEHUANGJINSCENARIO_H
