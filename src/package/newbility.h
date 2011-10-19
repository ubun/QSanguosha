#ifndef NEWBILITY_H
#define NEWBILITY_H
#include "package.h"
#include "standard.h"

class NewbilityPackage: public Package{
    Q_OBJECT

public:
    NewbilityPackage();
};

class NewbilityGeneralPackage: public Package{
    Q_OBJECT

public:
    NewbilityGeneralPackage();
};

class JuicePeach: public Peach{
    Q_OBJECT

public:
    Q_INVOKABLE JuicePeach(Card::Suit suit, int number);
    virtual QString getSubtype() const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool isAvailable(const Player *player) const;
};

class Stink: public BasicCard{
    Q_OBJECT

public:
    Q_INVOKABLE Stink(Card::Suit suit, int number);
    virtual QString getSubtype() const;
    virtual QString getEffectPath(bool is_male) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class Wall: public BasicCard{
    Q_OBJECT

public:
    Q_INVOKABLE Wall(Card::Suit suit, int number);
    virtual QString getSubtype() const;
    virtual QString getEffectPath(bool is_male) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class Poison: public BasicCard{
    Q_OBJECT

public:
    Q_INVOKABLE Poison(Card::Suit suit, int number);
    virtual QString getSubtype() const;
    virtual QString getEffectPath(bool is_male) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
};

class Sacrifice: public SingleTargetTrick{
    Q_OBJECT

public:
    Q_INVOKABLE Sacrifice(Card::Suit suit, int number);

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class ClearShirt:public Armor{
    Q_OBJECT

public:
    Q_INVOKABLE ClearShirt(Card::Suit suit, int number);

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const;
};

class Niubi:public Armor{
    Q_OBJECT

public:
    Q_INVOKABLE Niubi(Card::Suit suit, int number);
    virtual void onUninstall(ServerPlayer *player) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const;
};

class RollingpinCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE RollingpinCard();
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class WookonCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE WookonCard();
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class DiezhiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE DiezhiCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class WutianCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE WutianCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class YuluCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE YuluCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class ViewMyWordsCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ViewMyWordsCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class BaichuCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE BaichuCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

#endif // NEWBILITY_H
