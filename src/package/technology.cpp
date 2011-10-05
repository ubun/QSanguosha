#include "technology.h"
#include "general.h"
#include "skill.h"
#include "standard-skillcards.h"
#include "carditem.h"
#include "engine.h"
#include "standard.h"

class Tuiyan:public TriggerSkill{
public:
    Tuiyan():TriggerSkill("tuiyan"){
        events << CardUsed << TurnStart;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *lu, QVariant &data) const{
        if(event == TurnStart){
            lu->tag["bo"] = 0;
            lu->setMark("tuiyan", 0);
            lu->setMark("tuiyanfalse", 0);
            return false;
        }
        Room *room = lu->getRoom();
        if(room->getCurrent() != lu)
            return false;
        CardUseStruct use = data.value<CardUseStruct>();
        const Card *usecard = use.card;

        if(use.card->getId() < 0){
            if(use.card->getSubtype() == "skill_card"
               || use.card->getSubcards().length() > 1
               || use.card->getSubcards().isEmpty())
                return false;
            usecard = Sanguosha->getCard(use.card->getSubcards().first());
        }
        int precardnum = lu->tag.value("bo").toInt();
        if(lu->getMark("tuiyanfalse") == 0 && usecard->getNumber() > precardnum){
            if(precardnum == 0 || lu->askForSkillInvoke(objectName(), data)){
                ServerPlayer *target = room->askForPlayerChosen(lu, room->getAlivePlayers(), objectName());
                use.to.clear();
                use.to << target;
                /*if(usecard->inherits("Collateral")){
                    target = room->askForPlayerChosen(lu, room->getAlivePlayers(), objectName());
                    use.to << target;
                }*/
                LogMessage log;
                log.type = "$Tuiyan";
                log.from = lu;
                log.to = use.to;
                log.card_str = QString::number(usecard->getId());
                room->sendLog(log);
            }
            lu->addMark("tuiyan");
        }
        else
            lu->setMark("tuiyanfalse", 1);

        lu->tag["bo"] = usecard->getNumber();
        data = QVariant::fromValue(use);
        return false;
    }
};

class Tianji: public PhaseChangeSkill{
public:
    Tianji():PhaseChangeSkill("tianji"){
        frequency = Frequent;
    }

    virtual int getPriority() const{
        return -1;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target)
                && target->getPhase() == Player::NotActive
                && target->getMark("tuiyanfalse") == 0;
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        Room *room = target->getRoom();
        target->setMark("mingfa", 0);
        if(target->getMark("tuiyan") >= 3 && target->askForSkillInvoke(objectName())){
            target->setMark("tuiyan", 0);
            target->setMark("mingfa", 1);
            room->getThread()->trigger(TurnStart, target);
        }
        return false;
    }
};

class Mingfa:public TriggerSkill{
public:
    Mingfa():TriggerSkill("mingfa"){
        events << TurnStart;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getMark("mingfa") > 0;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *lu, QVariant &data) const{
        Room *room = lu->getRoom();
        int card_id = room->drawCard();

        LogMessage log;
        log.type = "$Mingfa";
        log.from = lu;
        log.card_str = QString::number(card_id);
        room->sendLog(log);

        room->getThread()->delay();
        if(Sanguosha->getCard(card_id)->getSuit() == Card::Spade){
            LogMessage log;
            log.type = "#Mingfa";
            log.from = lu;
            log.arg = objectName();
            room->sendLog(log);

            room->loseHp(lu, 2);
        }
        room->throwCard(card_id);
        lu->setMark("mingfa", 0);
        return false;
    }
};


TechnologyPackage::TechnologyPackage()
    :Package("technology")
{

    General *guanlu = new General(this, "guanlu", "god", 3);
    guanlu->addSkill(new Tuiyan);
    guanlu->addSkill(new Tianji);
    guanlu->addSkill(new Mingfa);

}

ADD_PACKAGE(Technology);
