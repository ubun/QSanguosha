#ifndef STANDARDSKILLCARDS_H
#define STANDARDSKILLCARDS_H

#include "skill.h"
#include "card.h"

class KaituoCard:public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE KaituoCard();
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class ShexianCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ShexianCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class CheatCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE CheatCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

//mc
class MpCard:public SkillCard{
    Q_OBJECT
};

class Mp1Card:public MpCard{
    Q_OBJECT

public:
    Q_INVOKABLE Mp1Card();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class Mp2Card: public MpCard{
    Q_OBJECT

public:
    Q_INVOKABLE Mp2Card();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class Mp3Card:public MpCard{
    Q_OBJECT

public:
    Q_INVOKABLE Mp3Card();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class Mp4Card: public MpCard{
    Q_OBJECT

public:
    Q_INVOKABLE Mp4Card();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

#endif // STANDARDSKILLCARDS_H
