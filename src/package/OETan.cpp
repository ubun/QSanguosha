#include "OETan.h"
#include "standard-skillcards.h"
#include "general.h"
#include "skill.h"
#include "engine.h"
#include "standard.h"
#include "carditem.h"
#include "generaloverview.h"
#include "clientplayer.h"
#include "client.h"
#include "maneuvering.h"
#include "room.h"
#include "ai.h"
#include "mountainpackage.h"

class Qianshui: public TriggerSkill{
public:
    Qianshui():TriggerSkill("qianshui"){
        events << CardEffect << CardEffected;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        CardEffectStruct effect = data.value<CardEffectStruct>();
        if(effect.card->inherits("Duel") || (effect.card->inherits("Slash") && effect.card->isRed())){
            //Room *room = player->getRoom();
            if(effect.to->hasSkill(objectName()) && effect.from){
                if(effect.to->getMark("shangfu")){return false;
                }/*
                LogMessage log;
                log.type = "#SkillNullify";
                log.from = effect.to;
                log.to << effect.from;
                log.arg = effect.card->objectName();

                room->sendLog(log);*/
                return true;
            }
        }
        return false;
    }
};

class Shangfu: public PhaseChangeSkill{
public:
    Shangfu():PhaseChangeSkill("shangfu"){
        frequency = Wake;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target)
                && target->getMark("shangfu") == 0
                && target->getPhase() == Player::Start
                && target->getEquips().length() >= 2;
    }

    virtual bool onPhaseChange(ServerPlayer *ibicdlcod) const{
        Room *room = ibicdlcod->getRoom();

        LogMessage log;
        log.type = "#shangfuWake";
        log.from = ibicdlcod;
        room->sendLog(log);

        room->loseMaxHp(ibicdlcod);

        room->acquireSkill(ibicdlcod, "mod");
        room->setPlayerMark(ibicdlcod, "shangfu", 1);
        ibicdlcod->gainMark("shangfu");
        return false;
    }
};

class Mod: public PhaseChangeSkill{
public:
    Mod():PhaseChangeSkill("mod"){
        frequency = Compulsory;
    }

    virtual int getPriority() const{
        return 3;
    }

    virtual bool onPhaseChange(ServerPlayer *ibicdlcod) const{
        if((ibicdlcod->getPhase() != Player::NotActive) && (ibicdlcod->getPhase() != Player::Start))
            return false;

        Room *room = ibicdlcod->getRoom();int i;
        if(!room->askForSkillInvoke(ibicdlcod, objectName()))
            return false;
        {
            ibicdlcod->loseAllSkills();
            room->acquireSkill(ibicdlcod, "mod");
            //QStringList lords = Sanguosha->getLords();
            QList<ServerPlayer *> players = room->getAlivePlayers();//getOtherPlayers(ibicdlcod);
            //foreach(ServerPlayer *player, players){
                //lords.removeOne(player->getGeneralName());
            //}
            for(i=0;i<2;i++){
            QStringList lord_skills;

            foreach(ServerPlayer *pp, players){
                QString lord = pp->getGeneralName();
                const General *general = Sanguosha->getGeneral(lord);
                QList<const Skill *> skills = general->findChildren<const Skill *>();
                foreach(const Skill *skill, skills){
                    if(!ibicdlcod->hasSkill(skill->objectName()) && skill->isVisible())
                        lord_skills << skill->objectName();
                }
            }

            if(!lord_skills.isEmpty()){
                QString skill_name = room->askForChoice(ibicdlcod, objectName(), lord_skills.join("+"));

                const Skill *skill = Sanguosha->getSkill(skill_name);
                room->acquireSkill(ibicdlcod, skill);

                if(skill->inherits("GameStartSkill")){
                    const GameStartSkill *game_start_skill = qobject_cast<const GameStartSkill *>(skill);
                    game_start_skill->onGameStart(ibicdlcod);
                }
            }
            }//for
        }
        return false;
    }
};

class Niubi: public PhaseChangeSkill{
public:
    Niubi():PhaseChangeSkill("niubi"){

    }

    virtual int getPriority() const{
        return 3;
    }

    virtual bool onPhaseChange(ServerPlayer *ubun) const{
        switch(ubun->getPhase()){
        case Player::Play: {
                Room *room = ubun->getRoom();
                bool invoked = ubun->askForSkillInvoke(objectName());
                if(invoked){
                    room->askForDiscard(ubun, "niubi", (room->getAlivePlayers().length()+1)/2, false, true);
                    ubun->gainMark("niubi");
                }//if
                return false;
            }//case
        case Player::Finish: {
                if(ubun->getMark("niubi")){
                    Room *room = ubun->getRoom();
                    while(ubun->askForSkillInvoke(objectName())){
                        ServerPlayer *player1 = room->askForPlayerChosen(ubun, room->getOtherPlayers(ubun), objectName());
                        ServerPlayer *player2 = room->askForPlayerChosen(ubun, room->getOtherPlayers(ubun), objectName());
                        room->swapSeat(player1,player2);
                        ubun->loseAllMarks("niubi");
                    }
                }//if
                break;
            }//case
        default:
            break;
        }
        return false;
    }
};

class Meikong: public TriggerSkill{
public:
    Meikong():TriggerSkill("meikong"){
        events << Predamage;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.to->getGeneralName() == "tenkei"){
            LogMessage log;
            log.type = "#MeikongSolace";
            log.from = player;
            log.to << damage.to;
            log.arg = QString::number(damage.damage);
            player->getRoom()->sendLog(log);

            RecoverStruct recover;
            recover.who = player;
            recover.recover = damage.damage;
            player->getRoom()->recover(damage.to, recover);
            return true;
        }
        return false;
    }
};

MaimengCard::MaimengCard(){
    once = true;
}

void MaimengCard::use(Room *room, ServerPlayer *player, const QList<ServerPlayer *> &targets) const{
    ServerPlayer *ubun = targets.first();
    if(ubun->hasSkill("maimeng")){
        ubun->obtainCard(this);
        room->setEmotion(ubun, "good");
        RecoverStruct recover;
        recover.who = player;
        player->getRoom()->recover(ubun, recover);
    }
}

bool MaimengCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select->hasLordSkill("maimeng") && to_select != Self;
}

class MaimengViewAsSkill: public ViewAsSkill{
public:
    MaimengViewAsSkill():ViewAsSkill("maimengv"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("MaimengCard") && player->getKingdom() == "tan";
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(selected.isEmpty())
            return !to_select->isEquipped();
        else if(selected.length() == 1){
            const Card *card = selected.first()->getFilteredCard();
            return !to_select->isEquipped() && to_select->getFilteredCard()->getSuit() == card->getSuit();
        }else
            return false;
    }

    virtual const Card *viewAs(const QList<CardItem*> &cards) const{
        MaimengCard *card = new MaimengCard;
        card->addSubcards(cards);
        card->setSkillName(objectName());
        return card;
    }
};

class Maimeng: public GameStartSkill{
public:
    Maimeng():GameStartSkill("maimeng$"){

    }

    virtual void onGameStart(ServerPlayer *ubun) const{
        Room *room = ubun->getRoom();
        QList<ServerPlayer *> players = room->getAlivePlayers();
        foreach(ServerPlayer *player, players){
            room->attachSkillToPlayer(player, "maimengv");
        }
    }
};

class Zhongpao: public TriggerSkill{
public:
    Zhongpao():TriggerSkill("zhongpao"){
        events << Predamage;
        frequency = Compulsory;
    }

    virtual int getPriority() const{
        return -2;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *xuchu, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();

        const Card *reason = damage.card;
        if(reason == NULL)
            return false;

        if(reason->inherits("Slash")){

            LogMessage log;
            log.type = "#zhongpao";
            log.from = xuchu;
            log.to << damage.to;
            log.arg = QString::number(damage.damage);
            log.arg2 = QString::number(damage.damage + 1);
            xuchu->getRoom()->sendLog(log);

            damage.damage ++;
            data = QVariant::fromValue(damage);
        }
        return false;
    }
};

class Guisu: public TriggerSkill{
public:
    Guisu():TriggerSkill("guisu"){
        events << CardEffect;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        CardEffectStruct effect = data.value<CardEffectStruct>();

        if(effect.card->inherits("Slash")){
            Room *room = player->getRoom();
            /*
            room->playSkillEffect(objectName());

            LogMessage log;
            log.type = "#Guisu";
            log.from = effect.from;
            log.to << effect.to;
            room->sendLog(log);
            */
            if(player->distanceTo(effect.to) > 1)
                return !room->askForDiscard(player, "guisu", player->distanceTo(effect.to)-1,true,true);
            else return false;
        }
        return false;
    }
};

class Menghua: public TriggerSkill{
public:
    Menghua():TriggerSkill("menghua"){
        events << Predamaged << PhaseChange;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        if(event == Predamaged){
            if(!player->getMark("menghua")) return false;
            DamageStruct damage = data.value<DamageStruct>();
            if((player->askForSkillInvoke(objectName(),data)) && (damage.from == player || damage.to == player))
            {
            LogMessage log;
            log.type = "#menghuaPrevent";
            log.from = player;
            log.arg = QString::number(damage.damage);
            player->getRoom()->sendLog(log);
                player->loseMark("menghua");
                return true;
            }
            return false;
        }else if(player->hasSkill(objectName()) && event == PhaseChange && player->getPhase() == Player::Start){
            player->loseAllMarks("menghua");
            player->gainMark("menghua");
        }
        return false;
    }
};

OETanPackage::OETanPackage()
    :Package("OEtan")
{
    General *OEibicdlcod, *OEubun, *OEtenkei;
    OEibicdlcod = new General(this, 3112, "ibicdlcod", "tan", 4, true);
    OEibicdlcod->addSkill(new Qianshui);
    OEibicdlcod->addSkill(new Shangfu);

    OEubun = new General(this, 3105, "ubun$", "tan", 4, true);
    OEubun->addSkill(new Niubi);
    OEubun->addSkill(new Meikong);
    OEubun->addSkill(new Maimeng);

    OEtenkei = new General(this, 3136, "tenkei", "tan", 3, false);
    OEtenkei->addSkill(new Zhongpao);
    OEtenkei->addSkill(new Guisu);
    OEtenkei->addSkill(new Menghua);

    addMetaObject<MaimengCard>();
    skills << new Mod << new MaimengViewAsSkill;
}

ADD_PACKAGE(OETan)

