#ifndef PASSMODESCENARIO_H
#define PASSMODESCENARIO_H

#include "gamerule.h"
#include "scenario.h"
#include "maneuvering.h"

class PDuanyanCard:public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE PDuanyanCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class RendePassCard:public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE RendePassCard();
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class JijiangPassCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE JijiangPassCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class LuoyiPassCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE LuoyiPassCard();
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class TuxiPassCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE TuxiPassCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};


class ZhihengPassCard:public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ZhihengPassCard();
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class FanjianPassCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE FanjianPassCard();
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class KurouPassCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE KurouPassCard();
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class PZhaxiangCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE PZhaxiangCard();
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};


class JieyinPassCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE JieyinPassCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class PYuyueCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE PYuyueCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class LijianPassCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE LijianPassCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class QingnangPassCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE QingnangPassCard();
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class PYaoshuCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE PYaoshuCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class PassModeScenario : public Scenario{
    Q_OBJECT

public:
    explicit PassModeScenario();

    virtual bool exposeRoles() const;
    virtual void assign(QStringList &generals, QStringList &roles) const;
    virtual int getPlayerCount() const;
    virtual void getRoles(char *roles) const;
    virtual void onTagSet(Room *room, const QString &key) const;
    virtual bool generalSelection() const;
};

class PassMode: public GameRule{
    Q_OBJECT

public:
    PassMode(QObject *parent);

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const;

private:
    mutable jmp_buf env;
};
#endif // PASSMODESCENARIO_H
