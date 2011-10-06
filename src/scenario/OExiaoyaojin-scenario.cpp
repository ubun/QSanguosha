#include "OExiaoyaojin-scenario.h"
#include "scenario.h"
#include "skill.h"
#include "clientplayer.h"
#include "client.h"
#include "engine.h"
#include "carditem.h"
#include "standard.h"
#include "general.h"
#include "standard-skillcards.h"
#include "ai.h"
#include "settings.h"

//OMEGA ERA XIAOYAOJINZHIZHAN-GOD ZHANGLIAO, GANNING, SUNQUAN

PozhenCard::PozhenCard(){
}

bool PozhenCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(targets.length() >= 3)
        return false;

    if(to_select == Self)
        return false;

    return !to_select->isKongcheng();
}

void PozhenCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    int card_id = room->askForCardChosen(effect.from, effect.to, "h", "pozhen");
    const Card *card = Sanguosha->getCard(card_id);
    room->moveCardTo(card, effect.from, Player::Hand, false);

    room->setEmotion(effect.to, "bad");
    room->setEmotion(effect.from, "good");
}


class PozhenViewAsSkill: public ZeroCardViewAsSkill{
public:
    PozhenViewAsSkill():ZeroCardViewAsSkill("Pozhen"){
    }

    virtual const Card *viewAs() const{
        return new PozhenCard;
    }

protected:
    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern == "@@Pozhen";
    }
};

class Pozhen:public PhaseChangeSkill{
public:
    Pozhen():PhaseChangeSkill("Pozhen"){
        view_as_skill = new PozhenViewAsSkill;
    }

    virtual bool onPhaseChange(ServerPlayer *zhangliao) const{
        if(zhangliao->getPhase() == Player::Draw){
            Room *room = zhangliao->getRoom();
            bool can_invoke = false;
            QList<ServerPlayer *> other_players = room->getOtherPlayers(zhangliao);
            foreach(ServerPlayer *player, other_players){
                if(!player->isKongcheng()){
                    can_invoke = true;
                    break;
                }
            }

            if(can_invoke && room->askForUseCard(zhangliao, "@@Pozhen", "@Pozhen-card"))
                return true;
        }

        return false;
    }
};


class Zhangliao_shenwei:public OneCardViewAsSkill{
public:
    Zhangliao_shenwei():OneCardViewAsSkill("Zhangliao_shenwei"){

    }

    virtual bool viewFilter(const CardItem *to_select) const{
        const Card *card = to_select->getFilteredCard();

        switch(ClientInstance->getStatus()){
        case Client::Playing:{
                // jink as slash
                return card->inherits("Jink");
            }

        case Client::Responsing:{
                QString pattern = ClientInstance->getPattern();
                /*if(pattern == "slash")
                    return card->inherits("Jink");
                else */if(pattern == "jink")
                    return card->inherits("Slash");
            }

        default:
            return false;
        }
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return Slash::IsAvailable(player)&&! player->hasUsed("Zhangliao_shenwei");
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "jink" /*|| pattern == "slash"*/;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getFilteredCard();
        /*if(card->inherits("Slash")){
            Jink *jink = new Jink(card->getSuit(), card->getNumber());
            jink->addSubcard(card);
            jink->setSkillName(objectName());
            return jink;
        }else*/ if(card->inherits("Jink")){
            Slash *slash = new Slash(card->getSuit(), card->getNumber());
            slash->addSubcard(card);
            slash->setSkillName(objectName());
            return slash;
        }else
            return NULL;
    }
};

class xiaoqixi: public OneCardViewAsSkill{
public:
    xiaoqixi():OneCardViewAsSkill("xiaoqixi"){

    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getFilteredCard()->isBlack();
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return (player->usedTimes("Dismantlement")<=1);
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *first = card_item->getCard();
        Dismantlement *dismantlement = new Dismantlement(first->getSuit(), first->getNumber());
        dismantlement->addSubcard(first->getId());
        dismantlement->setSkillName(objectName());
        return dismantlement;
    }
};


xiaozhihengCard::xiaozhihengCard(){
    target_fixed = true;
    once = true;
}

void xiaozhihengCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    room->throwCard(this);
    if(source->isAlive())
        room->drawCards(source, subcards.length());
}

class xiaozhiheng:public ViewAsSkill{
public:
    xiaozhiheng():ViewAsSkill("xiaozhiheng"){

    }
/*
    virtual bool viewFilter(const QList<CardItem *> &, const CardItem *) const{
        return true;
    }
*/
    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        return selected.length() < 3;
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.isEmpty())
            return NULL;

        xiaozhihengCard *xiaozhiheng_card = new xiaozhihengCard;
        xiaozhiheng_card->addSubcards(cards);

        return xiaozhiheng_card;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("xiaozhihengCard");
    }
};


class Qijin: public TriggerSkill{
public:
    Qijin():TriggerSkill("qijin"){
        events << CardLost;

        frequency = Frequent;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *OEgodzhaoyun, QVariant &data) const{
        if(OEgodzhaoyun->getCardCount(false) <7 ){
            CardMoveStar move = data.value<CardMoveStar>();

            if(move->from_place == Player::Hand){
                Room *room = OEgodzhaoyun->getRoom();
                if(room->askForSkillInvoke(OEgodzhaoyun, objectName())){
                    room->playSkillEffect(objectName());

                    OEgodzhaoyun->drawCards(1);
                }
            }
        }

        return false;
    }
};

//OE Generals End

class ganlingdebuhe: public TriggerSkill{

public:
    ganlingdebuhe():TriggerSkill("ganlingdebuhe"){
        events << HpRecover << CardUsed;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();

        if(event == HpRecover){
            RecoverStruct recover = data.value<RecoverStruct>();
            if(recover.who && recover.who->getGeneralName() == "lingtong"){
                room->loseHp(player,recover.recover);
            }
        }
        return false;
    }
};

class linggandebuhe: public TriggerSkill{

public:
    linggandebuhe():TriggerSkill("linggandebuhe"){
        events << HpRecover << CardUsed;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();

        if(event == HpRecover){
            RecoverStruct recover = data.value<RecoverStruct>();
            if(recover.who && recover.who->getGeneralName() == "OEganningXYJ"){
                room->loseHp(player,recover.recover);
            }
        }
        return false;
    }
};




class OEXiaoyaojinRule: public ScenarioRule{
public:
    OEXiaoyaojinRule(Scenario *scenario)
        :ScenarioRule(scenario)
    {
        events << GameStart << Death << TurnStart << Predamaged;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();

        switch(event){
        case GameStart:{
                if(player->isLord()){
                    player->drawCards(1, false);
                    player->gainMark("@qiaoxiao",2);

                    ServerPlayer *OEganningXYJ = room->findPlayer("OEganningXYJ");
                    room->acquireSkill(OEganningXYJ, "ganlingdebuhe");

                    ServerPlayer *lingtong = room->findPlayer("lingtong");
                    room->acquireSkill(lingtong, "linggandebuhe");
                    ServerPlayer *OEsunquanXYJ = room->findPlayer("OEsunquanXYJ");
                    OEsunquanXYJ->gainMark("@Mengjiangdeliqu");
                }
                break;
            }
        case TurnStart:{
            ServerPlayer *OEgodzhangliao = room->findPlayer("OEgodzhangliao");

            if(!player->isLord()){
                OEgodzhangliao->gainMark("@qiaoxiao");
                if(OEgodzhangliao->getMark("@qiaoxiao")>=3){
                    room->setCurrent(OEgodzhangliao);
                    room->getThread()->trigger(TurnStart, OEgodzhangliao);
                    room->setCurrent(player);}
                }
            if(player->isLord()){
                if(OEgodzhangliao->getMark("@qiaoxiao")>=2){
                    OEgodzhangliao->loseMark("@qiaoxiao",2);
                }
                else
                    player->turnOver();
                }
            break;
            }

        case Predamaged:{
                ServerPlayer *zhoutai = room->findPlayer("zhoutai");
                ServerPlayer *OEsunquanXYJ = room->findPlayer("OEsunquanXYJ");
                DamageStruct damage = data.value<DamageStruct>();
                if(player->getGeneralName() == "OEsunquanXYJ" && player->getLostHp() >= 3){
                    RecoverStruct recover;
                    recover.who = OEsunquanXYJ;
                    recover.recover = damage.damage;
                    room->recover(OEsunquanXYJ, recover);
                    room->loseHp(zhoutai, damage.damage+1);
                }
                break;
            }

        case Death:{
                ServerPlayer *OEgodzhangliao = room->findPlayer("OEgodzhangliao");
                ServerPlayer *OEsunquanXYJ = room->findPlayer("OEsunquanXYJ");
                ServerPlayer *OEganningXYJ = room->findPlayer("OEganningXYJ");
                ServerPlayer *lingtong = room->findPlayer("lingtong");
                DamageStar damage = data.value<DamageStar>();                
                if(player->getGeneralName() == "taishici" && damage && damage->from && damage->from->isLord())
                {
                    OEgodzhangliao->drawCards(1, false);
                    if(OEsunquanXYJ)
                    {
                        RecoverStruct recover;
                        recover.who = OEsunquanXYJ;
                        recover.recover = 1;
                        room->recover(OEsunquanXYJ, recover);
                    }
                }
                if(player->getRole() == "rebel")
                    {
                    lingtong->loseSkill("linggandebuhe");
                    OEganningXYJ->loseSkill("ganlingdebuhe");
                }
                if(/*player->getGeneralName() == "sp_pangde" &&
                   */damage && damage->from /*&& damage->from->isLord()*/)
                {
                    damage = NULL;
                    data = QVariant::fromValue(damage);
                }
                break;
            }

        default:
            break;
        }

        return false;
    }
};


OEXiaoyaojinScenario::OEXiaoyaojinScenario()
    :Scenario("OEXiaoyaojin")
{
    lord = "OEgodzhangliao";
    /*loyalists << "huatuo";*/
    rebels << "xusheng" << "lingtong" << "OEganningXYJ" << "zhoutai" << "taishici" << "OEsunquanXYJ";
    /*renegades << "lumeng";*/

    rule = new OEXiaoyaojinRule(this);

    /*skills << new zhoutai_sizhan;*/
    skills << new ganlingdebuhe;
    skills << new linggandebuhe;

    General *OEgodzhangliao = new General(this, 8001, "OEgodzhangliao", "god", 4, true, true);
    OEgodzhangliao->addSkill(new Pozhen);
    OEgodzhangliao->addSkill(new Skill("changdao", Skill::Compulsory));
    OEgodzhangliao->addSkill(new Zhangliao_shenwei);

    General *OEganningXYJ = new General(this, 8002, "OEganningXYJ", "wu", 4, true, true);
    OEganningXYJ->addSkill(new xiaoqixi);

    General *OEsunquanXYJ = new General(this, 8003, "OEsunquanXYJ", "wu", 4, true, true);
    OEsunquanXYJ->addSkill(new xiaozhiheng);

/*
    General *OEgodzhaoyun = new General(this, 8060, "OEgodzhaoyun", "shu", 2, true, true);
    OEgodzhaoyun->addSkill(new Qijin);
    OEgodzhaoyun->addSkill("paoxiao");
    OEgodzhaoyun->addSkill("longhun");
*/
    addMetaObject<PozhenCard>();

}


void OEXiaoyaojinScenario::getRoles(char *roles) const{
    strcpy(roles, "ZFFFFFF");
}

void OEXiaoyaojinScenario::onTagSet(Room *room, const QString &key) const{

}

ADD_SCENARIO(OEXiaoyaojin);

