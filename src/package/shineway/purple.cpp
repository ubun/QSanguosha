#include "purple.h"
#include "skill.h"
#include "client.h"
#include "engine.h"
#include "carditem.h"
#include "room.h"

class Zhuosu:public MasochismSkill{
public:
    Zhuosu():MasochismSkill("zhuosu"){
    }

    virtual void onDamaged(ServerPlayer *yelai, const DamageStruct &damage) const{
        Room *room = yelai->getRoom();
        if(yelai->askForSkillInvoke(objectName())){
            ServerPlayer *player = room->askForPlayerChosen(yelai, room->getAllPlayers(), objectName());
            if(yelai->faceUp())
                player->gainAnExtraTurn();
            else
                player->turnOver();
            yelai->turnOver();
        }
    }
};

class Difu: public TriggerSkill{
public:
    Difu():TriggerSkill("difu"){
        events << Dying;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *pp, QVariant &data) const{
        if(pp->isNude())
            return false;
        Room *room = pp->getRoom();
        if(pp->askForSkillInvoke(objectName())){
            ServerPlayer *taregt = room->askForPlayerChosen(pp, room->getOtherPlayers(pp), objectName());
            taregt->drawCards(pp->getCardCount(true));
            DyingStruct dying_data = data.value<DyingStruct>();
            room->killPlayer(pp, dying_data.damage);
            return true;
        }
        return false;
    }
};

class Bui: public TriggerSkill{
public:
    Bui():TriggerSkill("bui"){
        events << Death;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        DamageStar damage = data.value<DamageStar>();
        ServerPlayer *killer = damage ? damage->from : NULL;

        if(killer && killer != player)
            killer->gainMark("@noequip");
        return false;
    }
};

class Wuxian: public TriggerSkill{
public:
    Wuxian():TriggerSkill("wuxian"){
        events << CardGotDone;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *nichang = room->findPlayerBySkillName(objectName());
        if(nichang == player){
            if(nichang->getHandcardNum() > 5){
                int n = nichang->getHandcardNum() - nichang->getMaxHP();
                if(n > 0)
                    room->askForDiscard(nichang, objectName(), n);
            }
            return false;
        }
        if(room->getCurrent() == player)
            return false;
        if(nichang && nichang->askForSkillInvoke(objectName()))
            nichang->drawCards(1);
        return false;
    }
};

class Xianming: public TriggerSkill{
public:
    Xianming():TriggerSkill("xianming"){
        events << CardEffected << CardEffect;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *miug, QVariant &data) const{
        //Room *room = miug->getRoom();
        CardEffectStruct effect = data.value<CardEffectStruct>();
        if(!effect.card->inherits("Snatch") && !effect.card->inherits("Dismantlement"))
            return false;
        if(miug->askForSkillInvoke(objectName())){
            Card::Suit suit = effect.card->getSuit();
            int num = effect.card->getNumber();
            if(effect.card->inherits("Snatch")){
                Dismantlement *d = new Dismantlement(suit, num);
                d->setSkillName(objectName());
                d->addSubcard(effect.card);
                effect.card = d;
            }
            else{
                Snatch *s = new Snatch(suit, num);
                s->setSkillName(objectName());
                s->addSubcard(effect.card);
                effect.card = s;
            }
            data = QVariant::fromValue(effect);
        }
        return false;
    }
};

class Xiannei: public TriggerSkill{
public:
    Xiannei():TriggerSkill("xiannei"){
        events << CardLost;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getHandcardNum() < target->getMaxCards();
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        CardMoveStar move = data.value<CardMoveStar>();
        Room *room = player->getRoom();
        ServerPlayer *current = room->getCurrent();
        ServerPlayer *xing = room->findPlayerBySkillName(objectName());
        if(!xing || move->to == player || player->hasFlag("nie"))
            return false;
        if(move->from_place == Player::Hand || move->from_place == Player::Equip){
            if(!current->hasFlag("nei") && room->askForSkillInvoke(xing, objectName())){
                room->playSkillEffect(objectName());
                xing->setFlags("nie");
                if(room->askForDiscard(xing, objectName(), 1, true, true)){
                    QString ch = !player->isWounded()? "draw":
                                 room->askForChoice(xing, objectName(), "draw+hp");
                    if(ch == "draw")
                        player->drawCards(2);
                    else{
                        RecoverStruct r;
                        r.who = xing;
                        room->recover(player, r);
                    }
                    room->setPlayerFlag(current, "nei");
                }
                xing->setFlags("-nie");
            }
        }
        return false;
    }
};

class Shangjue: public TriggerSkill{
public:
    Shangjue():TriggerSkill("shangjue"){
        events << CardLost;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *zous, QVariant &data) const{
        if(zous->getPhase() == Player::NotActive && zous->isKongcheng()){
            CardMoveStar move = data.value<CardMoveStar>();
            if(move->from_place == Player::Hand){
                Room *room = zous->getRoom();
                if(room->askForSkillInvoke(zous, objectName())){
                    room->playSkillEffect(objectName());
                    zous->drawCards(2);
                    DamageStruct damage;
                    damage.from = zous;
                    damage.to = room->askForPlayerChosen(zous, room->getAllPlayers(), objectName());
                    room->damage(damage);
                }
            }
        }
        return false;
    }
};

class Quling: public PhaseChangeSkill{
public:
    Quling():PhaseChangeSkill("quling"){
        frequency = Wake;
    }

    static QList<ServerPlayer *> getFriends(PlayerStar me, QString kingdom){
        Room *room = me->getRoom();
        QList<ServerPlayer *> targets;
        foreach(ServerPlayer *tmp, room->getOtherPlayers(me)){
            if(tmp->getKingdom() == kingdom)
                targets << tmp;
        }
        return targets;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target)
                && target->getMark("quling") == 0
                && target->getPhase() == Player::Start
                && getFriends((PlayerStar)target, "qun").isEmpty();
    }

    virtual bool onPhaseChange(ServerPlayer *zou) const{
        Room *room = zou->getRoom();

        LogMessage log;
        log.type = "#QulingWake";
        log.from = zou;
        room->sendLog(log);

        room->loseMaxHp(zou);
        room->setPlayerProperty(zou, "kingdom", "wei");
        room->acquireSkill(zou, "huoshui");
        room->setPlayerMark(zou, "quling", 1);

        return false;
    }
};

class Huoshui: public TriggerSkill{
public:
    Huoshui():TriggerSkill("huoshui"){
        events << Dying;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *pp, QVariant &data) const{
        Room *room = pp->getRoom();
        if(pp->askForSkillInvoke(objectName())){
            foreach(ServerPlayer *tmp, room->getAllPlayers())
                if(tmp->getGeneral()->isMale())
                    room->loseHp(tmp);
            DyingStruct dying_data = data.value<DyingStruct>();
            room->killPlayer(pp, dying_data.damage);
            return true;
        }
        return false;
    }
};

class PoxieBuff: public TriggerSkill{
public:
    PoxieBuff():TriggerSkill("#poxie"){
        events << CardEffect;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasFlag("poxie") && target->isAlive();
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *duanmt, QVariant &data) const{
        CardEffectStruct effect = data.value<CardEffectStruct>();
        if(!effect.card->inherits("BasicCard") && !effect.card->inherits("TrickCard"))
            return false;
        PlayerStar target = duanmt->tag["PoxieTarget"].value<PlayerStar>();
        if(!target || duanmt->hasFlag("Poxie"))
            return false;
        Room *room = duanmt->getRoom();
        QString suit_str = effect.card->getSuitString();
        QString pattern = QString(".%1").arg(suit_str.at(0).toUpper());
        QString prompt = QString("@poxie:%1::%2").arg(duanmt->getGeneralName()).arg(suit_str);
        if(!room->askForCard(target, pattern, prompt)){
            DamageStruct dmg;
            dmg.from = duanmt;
            dmg.to = target;
            room->damage(dmg);
        }
        duanmt->setFlags("Poxie");
        duanmt->tag.remove("PoxieTarget");
        return false;
    }
};

class Poxie: public DrawCardsSkill{
public:
    Poxie():DrawCardsSkill("poxie"){
    }

    virtual int getDrawNum(ServerPlayer *duanmt, int n) const{
        Room *room = duanmt->getRoom();
        if(room->askForSkillInvoke(duanmt, objectName())){
            room->playSkillEffect(objectName());
            PlayerStar target = room->askForPlayerChosen(duanmt, room->getAllPlayers(), objectName());
            duanmt->tag["PoxieTarget"] = QVariant::fromValue(target);
            duanmt->setFlags(objectName());
            return n - 1;
        }else
            return n;
    }
};

class Xiangfeng: public TriggerSkill{
public:
    Xiangfeng():TriggerSkill("xiangfeng"){
        events << SlashMissed;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *dm = room->findPlayerBySkillName(objectName());
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        if(dm == effect.to && dm->askForSkillInvoke(objectName())){
            dm->obtainCard(effect.slash);
        }
        return false;
    }
};

/*
class Guolie: public TriggerSkill{
public:
    Guolie():TriggerSkill("guolie"){
        events << Predamage;
    }

    virtual int getPriority() const{
        return 2;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        DamageStruct damage = data.value<DamageStruct>();
        if(player->askForSkillInvoke(objectName(), data)){
            DummyCard *card1 = damage.to->wholeHandCards();
            DummyCard *card2 = damage.from->wholeHandCards();

            if(card1){
                room->moveCardTo(card1, damage.from, Player::Hand, false);
                delete card1;
            }
            if(card2){
                room->moveCardTo(card2, damage.to, Player::Hand, false);
                delete card2;
            }

            LogMessage log;
            log.type = "#Guolie";
            log.from = damage.from;
            log.to << damage.to;
            room->sendLog(log);
            return true;
        }
        return false;
    }
};

class Qianpan: public TriggerSkill{
public:
    Qianpan():TriggerSkill("qianpan"){
        events << CardEffected << Damaged << HpRecover;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        if(event == Damaged || event == HpRecover){
            QString kingdom;
            if(event == HpRecover){
                RecoverStruct recover = data.value<RecoverStruct>();
                if(!recover.who || recover.who == player)
                    return false;
                kingdom = recover.who->getKingdom();
            }
            else
                kingdom = player->getKingdom() == "shu" ? "wu" : "shu";
            LogMessage log;
            log.type = "#Qianpan";
            log.from = player;
            log.arg = kingdom;
            log.arg2 = objectName();
            room->sendLog(log);

            room->setPlayerProperty(player, "kingdom", kingdom);
            return false;
        }
        CardEffectStruct effect = data.value<CardEffectStruct>();
        if(effect.card->isNDTrick() && effect.from->getKingdom() != effect.to->getKingdom()){

            LogMessage log;
            log.type = "#QianpanProtect";
            log.from = effect.to;
            log.to << effect.from;
            log.arg = effect.card->objectName();
            log.arg2 = objectName();
            room->sendLog(log);

            return true;
        }
        return false;
    }
};

class Anshi: public TriggerSkill{
public:
    Anshi():TriggerSkill("anshi"){
        events << Dying << PhaseChange;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *fanzhang = room->findPlayerBySkillName(objectName());
        if(!fanzhang)
            return false;
        if(event == PhaseChange && fanzhang->hasFlag("Anshi") && fanzhang->getPhase() == Player::NotActive){
            room->detachSkillFromPlayer(fanzhang, "wansha");
            LogMessage log;
            log.type = "#AnshiSuicide";
            log.from = fanzhang;
            log.arg = objectName();
            room->sendLog(log);

            room->loseHp(fanzhang, fanzhang->getHp());
            return false;
        }
        if(room->getCurrent() != fanzhang)
            return false;
        if(event == Dying && fanzhang->getPhase() == Player::Play){
            DyingStruct dying = data.value<DyingStruct>();
            if(dying.who != fanzhang && fanzhang->askForSkillInvoke(objectName(), data)){
                fanzhang->acquireSkill("wansha");
                room->setPlayerFlag(fanzhang, "Anshi");
            }
        }
        return false;
    }
};

class Jiaozei: public TriggerSkill{
public:
    Jiaozei():TriggerSkill("jiaozei"){
        events << SlashMissed;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *player, QVariant &data) const{
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        if(!effect.to->isKongcheng() && !player->isKongcheng() && player->askForSkillInvoke(objectName(), data)){
            Room *room = player->getRoom();
            const Card *from_card = NULL;
            if(player->hasLordSkill("zhaobing") && player->askForSkillInvoke("zhaobing", data)){
                foreach(ServerPlayer *tmp, room->getOtherPlayers(player)){
                    if(tmp->getKingdom() != "qun")
                        continue;
                    from_card = room->askForCard(tmp, ".", "@zhaobing-pindian:" + player->objectName(), QVariant::fromValue(player));
                    if(from_card){
                        LogMessage log;
                        log.type = "$Zhaobing";
                        log.from = tmp;
                        log.to << player;
                        log.card_str = from_card->getEffectIdString();
                        room->sendLog(log);
                        break;
                    }
                }
            }
            if(player->pindian(effect.to, objectName(), from_card)){
                LogMessage log;
                log.type = "#Jiaozei";
                log.from = player;
                log.to << effect.to;
                log.arg = objectName();
                room->sendLog(log);

                room->slashResult(effect, NULL);
                return true;
            }
        }

        return false;
    }
};
*/
PurplePackage::PurplePackage()
    :Package("purple")
{
    General *purpleyelai = new General(this, "purpleyelai", "wei", 3, false);
    purpleyelai->addSkill(new Zhuosu);
    purpleyelai->addSkill(new Difu);
    purpleyelai->addSkill(new Bui);

    General *purplesunnichang = new General(this, "purplesunnichang", "wu", 3, false);
    purplesunnichang->addSkill(new Wuxian);
    purplesunnichang->addSkill(new Xianming);

    General *purplexingcai = new General(this, "purplexingcai", "shu", 4, false);
    purplexingcai->addSkill(new Xiannei);

    General *purpleduanmeng = new General(this, "purpleduanmeng", "shu", 3, false);
    purpleduanmeng->addSkill(new Poxie);
    purpleduanmeng->addSkill(new Xiangfeng);
    purpleduanmeng->addSkill(new Skill("fuchou", Skill::Compulsory));

    General *purplezoushi = new General(this, "purplezoushi", "qun", 4, false);
    purplezoushi->addSkill(new Shangjue);
    purplezoushi->addSkill(new Quling);
    skills << new Huoshui;
    //addMetaObject<JunlingCard>();
}

ADD_PACKAGE(Purple)
