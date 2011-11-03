#ifndef GREEN_H
#define GREEN_H

#include "package.h"
#include "card.h"
#include "standard.h"

struct YuanlvStruct{
    YuanlvStruct();
    QString kingdom;
    QString generalA;
    QString generalB;
    int maxhp;
    QStringList skills;
};

Q_DECLARE_METATYPE(YuanlvStruct);

class YuanlvCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE YuanlvCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class GreenPackage: public Package{
    Q_OBJECT

public:
    GreenPackage();
};

#endif // GREEN_H
