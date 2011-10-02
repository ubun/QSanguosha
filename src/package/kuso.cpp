#include "kuso.h"
#include "skill.h"
#include "client.h"
#include "engine.h"
#include "carditem.h"
#include "room.h"

class Huaxu: public TriggerSkill{
public:
    Huaxu():TriggerSkill("huaxu"){
        events << Damage << TurnStart;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        if(event == TurnStart){
            foreach(ServerPlayer *tmp, player->getRoom()->getAlivePlayers()){
                for(int i = 1; i <= 4; i++){
                    QString mark_name = "@hana" + QString::number(i);
                    if(tmp->getMark(mark_name) > 0)
                        tmp->loseAllMarks(mark_name);
                }
            }
            return false;
        }
        if(player->getPhase() == Player::NotActive)
            return false;
        DamageStruct damage = data.value<DamageStruct>();
        if(!damage.card || damage.card->getSuit() == Card::NoSuit)
            return false;
        Room *room = player->getRoom();
        if(damage.to->isDead())
            return false;
        if(player->askForSkillInvoke(objectName(), data)){
            int i = 0;
            switch(damage.card->getSuit()){
            case Card::Heart : i = 1; break;
            case Card::Diamond : i = 2; break;
            case Card::Spade : i = 3; break;
            case Card::Club : i = 4; break;
            default: break;
            }
            QString mark_name = "@hana" + QString::number(i);
            if(damage.to->getMark(mark_name) < 1)
                damage.to->gainMark(mark_name);
            LogMessage log;
            log.type = "#Gotoshit";
            log.from = player;
            log.to << damage.to;
            log.arg = Card::Suit2String(damage.card->getSuit());
            room->sendLog(log);
        }
        return false;
    }
};

class HuaxuEffect:public TriggerSkill{
public:
    HuaxuEffect():TriggerSkill("#huaxu_eft"){
        events << CardUsed;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        for(int i = 1; i <= 4; i++){
            QString mark_name = "@hana" + QString::number(i);
            if(target->getMark(mark_name) > 0)
                return true;
        }
        return false;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        if(player->getPhase() == Player::NotActive)
            return false;
        CardUseStruct use = data.value<CardUseStruct>();
        CardStar card = use.card;
        Card::Suit suit = card->getSuit();
        Room *room = player->getRoom();
        if(card && suit != Card::NoSuit){
            LogMessage log;
            log.from = player;
            log.card_str = card->getEffectIdString();
            if(suit == Card::Heart && player->getMark("@hana1") > 0){
                log.type = "$HuaxuDamage1";
                room->sendLog(log);
                DamageStruct damage;
                damage.from = damage.to = player;
                damage.card = card;
                //damage.damage = player->getMark("@hana1");
                damage.nature = DamageStruct::Fire;
                room->throwCard(card);
                room->damage(damage);
            }
            else if(suit == Card::Diamond && player->getMark("@hana2") > 0){
                log.type = "$HuaxuDamage2";
                room->sendLog(log);
                DamageStruct damage;
                damage.from = damage.to = player;
                damage.card = card;
                //damage.damage = player->getMark("@hana2");
                room->throwCard(card);
                room->damage(damage);
            }
            else if(suit == Card::Spade && player->getMark("@hana3") > 0){
                log.type = "$HuaxuDamage3";
                room->sendLog(log);
                //room->loseHp(player, player->getMark("@hana3"));
                room->throwCard(card);
                room->loseHp(player);
            }
            else if(suit == Card::Club && player->getMark("@hana4") > 0){
                log.type = "$HuaxuDamage4";
                room->sendLog(log);
                DamageStruct damage;
                damage.from = damage.to = player;
                damage.card = card;
                //damage.damage = player->getMark("@hana4");
                damage.nature = DamageStruct::Thunder;
                room->throwCard(card);
                room->damage(damage);
            }
        }
        return false;
    }
};

class Liaoting:public ViewAsSkill{
public:
    Liaoting():ViewAsSkill("liaoting"){
        frequency = Limited;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getMark("liaot") >= 1;
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(selected.length() > 4)
            return false;
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() != 4)
            return NULL;

        LiaotingCard *card = new LiaotingCard;
        card->addSubcards(cards);

        return card;
    }
};

LiaotingCard::LiaotingCard(){
    target_fixed = true;
}

void LiaotingCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    QList<int> subs = this->getSubcards();
    source->loseMark("liaot");
    foreach(int tmp, subs){
        if(!Sanguosha->getCard(tmp)->inherits("Shit")){
            room->throwCard(this);
            return;
        }
    }
    if(source->isLord() || source->getRole() == "loyalist"){
        foreach(ServerPlayer *tmp, room->getAlivePlayers())
            if(tmp->getRole() == "renegade" || tmp->getRole() == "rebel")
                room->killPlayer(tmp);
    }
    else if(source->getRole() == "renegade"){
        source->drawCards(5, false);
        foreach(ServerPlayer *tmp, room->getAlivePlayers())
            if(tmp->getRole() == "loyalist" || tmp->getRole() == "rebel")
                room->killPlayer(tmp);
    }
    else if(source->getRole() == "rebel")
        room->killPlayer(room->getLord());

    room->throwCard(this);
    if(source->isAlive())
        room->setPlayerProperty(source, "hp", source->getMaxHP());
}

KusoPackage::KusoPackage()
    :Package("kuso")
{
    General *kusoking = new General(this, "kusoking", "god", 4, false);
    kusoking->addSkill(new Huaxu);
    kusoking->addSkill(new HuaxuEffect);
    related_skills.insertMulti("huaxu", "#huaxu_eft");
    kusoking->addSkill(new Liaoting);
    kusoking->addSkill(new MarkAssignSkill("liaot", 1));
    related_skills.insertMulti("liaoting", "#liaot");

    addMetaObject<LiaotingCard>();
}

ADD_PACKAGE(Kuso)
