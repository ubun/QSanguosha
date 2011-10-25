#include "OETanB.h"
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

GuayanCard::GuayanCard(){
    will_throw = false;
}

bool GuayanCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;

    if(to_select == Self)
        return false;

    return !to_select->isKongcheng();
}

void GuayanCard::use(Room *room, ServerPlayer *OEedado, const QList<ServerPlayer *> &targets) const{
    ServerPlayer *tiger = targets.first();
    if(tiger->isKongcheng())
        return;

    //room->playSkillEffect("guayan", 1);

    bool success = OEedado->pindian(tiger, "guayan");
    if(success){
        //room->playSkillEffect("guayan", 2);
        int wolfnum = tiger->getHandcardNum();int i;
        for(i = 0; i < (wolfnum+1)/2; i++){
            room->moveCardTo(Sanguosha->getCard(room->askForCardChosen(OEedado, tiger, "h", "tuxi"))
                         , OEedado, Player::Hand, false);
        }
    }
}

class GuayanViewAsSkill: public ZeroCardViewAsSkill{
public:
    GuayanViewAsSkill():ZeroCardViewAsSkill("guayan"){
    }

    virtual const Card *viewAs() const{
        return new GuayanCard;
    }

//protected:
    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern == "@@guayan";
    }
};

class Guayan:public TriggerSkill{
public:
    Guayan():TriggerSkill("guayan"){
        view_as_skill = new GuayanViewAsSkill;

        events << PhaseChange << FinishJudge;
        frequency = Frequent;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *OEedado, QVariant &data) const{
        Room *room = OEedado->getRoom();
        if(event == PhaseChange && OEedado->getPhase() == Player::Judge){
            if(OEedado->askForSkillInvoke("guayan")){
                //room->playSkillEffect(objectName());

                JudgeStruct judge;
                judge.pattern = QRegExp("(.*):(.*):(.*)");
                judge.good = true;
                judge.reason = objectName();
                judge.who = OEedado;

                room->judge(judge);

                switch(judge.card->getSuit()){
                case Card::Heart:{
                        RecoverStruct recover;
                        recover.who = OEedado;
                        room->recover(OEedado, recover);
                        break;
                    }

                case Card::Diamond:{
                        if(OEedado->isKongcheng())
                            return false;
                        room->askForUseCard(OEedado, "@@guayan", "@guayan");
                        break;
                    }

                case Card::Club:{
                        OEedado->setFlags("luoyi");
                        break;
                    }

                case Card::Spade:{
                        room->setPlayerFlag(OEedado, "tianyi_success");
                        break;
                    }

                default:
                    break;
                }
                return false;
            }
        }else if(event == PhaseChange && OEedado->getPhase() == Player::Finish){
            if(OEedado->hasFlag("tianyi_success")){
                room->setPlayerFlag(OEedado, "-tianyi_success");
            }
        }else if(event == FinishJudge){
            JudgeStar judge = data.value<JudgeStar>();
            if(judge->reason == "guayan" && judge->isGood() && !(OEedado->hasSkill("tiandu")) ){
                OEedado->addToPile("guayanpile", judge->card->getEffectiveId());
                return true;
            }
        }
        return false;
    }
};

class Nitai: public PhaseChangeSkill{
public:
    Nitai():PhaseChangeSkill("nitai"){
        frequency = Wake;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return (PhaseChangeSkill::triggerable(target)
            && target->getMark("nitai") == 0
            && target->getPhase() == Player::Start);
            //target->getPile("guayanp").length() >= 3;
    }

    virtual bool onPhaseChange(ServerPlayer *OEedado) const{
        int m = 0;
        Room *room = OEedado->getRoom();
        //const QList<int> &guayanpile = OEedado->getPile("guanyanile");
        QList<int> hearts, clubs, spades, diamonds;
        foreach(int card_id, OEedado->getPile("guayanpile")){
            const Card *card = Sanguosha->getCard(card_id);
            Card::Suit suit = card->getSuit();

            switch(suit){
            case Card::Heart:   hearts << card_id;  break;
            case Card::Diamond: diamonds << card_id;break;
            case Card::Spade:   spades << card_id;  break;
            case Card::Club:    clubs << card_id;   break;
            default: break;
            }
        }
        if(hearts.length() > 2 || diamonds.length() > 2 || spades.length() > 2 || clubs.length() > 2)
            m++;

        if(!m) return false;
        /*
        LogMessage log;
        log.type = "#NitaiWake";
        log.from = OEedado;
        room->sendLog(log);

        room->playSkillEffect("nitai");

        room->broadcastInvoke("animate", "lightbox:$nitai:5000");
        room->getThread()->delay(5000);*/

        room->setPlayerMark(OEedado, "nitai", 1);
        room->acquireSkill(OEedado, "tiandu");
        room->acquireSkill(OEedado, "mod");

        room->loseMaxHp(OEedado);

        return false;
    }
};

class Skydao:public MasochismSkill{
public:
    Skydao():MasochismSkill("skydao"){
        frequency = Compulsory;
    }

    virtual void onDamaged(ServerPlayer *player, const DamageStruct &damage) const{
        Room *room = player->getRoom();
        if(damage.to && damage.to == player && player->getPhase() == Player::NotActive){
            LogMessage log;
            log.from = player;
            log.type = "#SkydaoMAXHP";
            log.arg2 = objectName();
            room->setPlayerProperty(player, "maxhp", player->getMaxHP() + 1);
            log.arg = QString::number(player->getMaxHP());
            room->sendLog(log);
        }
    }
};

class Noqing:public MasochismSkill{
public:
    Noqing():MasochismSkill("noqing"){
        frequency = Compulsory;
    }

    virtual int getPriority() const{
        return -1;
    }

    virtual QString getDefaultChoice(ServerPlayer *player) const{
        if(player->getMaxHP() >= player->getHp() + 2)
            return "maxhp";
        else
            return "hp";
    }

    virtual void onDamaged(ServerPlayer *player, const DamageStruct &damage) const{
        Room *room = player->getRoom();
        if(damage.to && damage.to == player){
            foreach(ServerPlayer *tmp, room->getOtherPlayers(player))
                if(tmp->getHp() < player->getHp())
                    return;
            foreach(ServerPlayer *tmp, room->getAllPlayers()){
                QString choice = room->askForChoice(tmp, objectName(), "hp+max_hp");
                LogMessage log;
                log.from = player;
                log.to << tmp;
                log.arg = objectName();
                if(choice == "hp"){
                    log.type = "#NoqingLoseHp";
                    room->sendLog(log);
                    room->loseHp(tmp);
                }else{
                    log.type = "#NoqingLoseMaxHp";
                    room->sendLog(log);
                    room->loseMaxHp(tmp);
                }
            }
        }
    }
};

OETanBPackage::OETanBPackage()
    :Package("OEtanb")
{
    General *OEedado = new General(this, 3126, "edado", "tan", 3, false);
    OEedado->addSkill(new Guayan);
    OEedado->addSkill("#luoyi");
    OEedado->addSkill(new Nitai);
    related_skills.insertMulti("guayan", "#luoyi");

    General *OEtianyin = new General(this, 3144, "tianyin", "tan", 4, false);
    OEtianyin->addSkill(new Skydao);
    OEtianyin->addSkill(new Noqing);

    addMetaObject<GuayanCard>();
}

ADD_PACKAGE(OETanB)


