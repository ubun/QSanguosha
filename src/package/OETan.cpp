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

class Qianshui: public TriggerSkill{
public:
    Qianshui():TriggerSkill("Qianshui"){
        events << CardEffect << CardEffected;;
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
        /*
        LogMessage log;
        log.type = "#shangfuWake";
        log.from = ibicdlcod;
        room->sendLog(log);
        */
        room->loseMaxHp(ibicdlcod);

        //room->acquireSkill(ibicdlcod, "mod");
        room->setPlayerMark(ibicdlcod, "shangfu", 1);
        ibicdlcod->gainMark("shangfu");
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

    OEtenkei = new General(this, 3136, "tenkei", "tan", 3, true);
}

ADD_PACKAGE(OETan)

