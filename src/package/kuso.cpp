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
            if(player->isDead())
                return true;
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

//cards

class KawaiiDressSkill: public ArmorSkill{
public:
    KawaiiDressSkill():ArmorSkill("kawaii_dress"){
        events << Predamaged;
    }

    virtual int getPriority() const{
        return 2;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(player && player == damage.to && player->getHp() == 1){
            LogMessage log;
            log.type = "#KawaiiDressProtect";
            log.from = player;
            log.arg = QString::number(damage.damage);
            if(damage.nature == DamageStruct::Normal)
                log.arg2 = "normal_nature";
            else if(damage.nature == DamageStruct::Fire)
                log.arg2 = "fire_nature";
            else
                log.arg2 = "thunder_nature";
            player->getRoom()->sendLog(log);

            return true;
        }
        return false;
    }
};

KawaiiDress::KawaiiDress(Suit suit, int number) :Armor(suit, number){
    setObjectName("kawaii_dress");
    skill = new KawaiiDressSkill;
}

void KawaiiDress::onUninstall(ServerPlayer *player) const{
    if(player->isAlive() && player->getMark("qinggang") == 0){
        player->drawCards(2);
    }
}

class FivelineSkill: public ArmorSkill{
public:
    FivelineSkill():ArmorSkill("fiveline"){
        events << HpChanged;
    }

    virtual int getPriority() const{
        return -1;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &) const{
        int hp = player->getHp();
        if(player->isDead() || hp < 1)
            return false;
        Room *room = player->getRoom();
        QStringList skills;
        skills << "rende" << "jizhi" << "jieyin" << "guose" << "kurou";
        QVariantList has_skills = player->tag["fiveline"].toList();
        foreach(QString str, skills){
            if(has_skills.contains(str))
                continue;
            else{
                room->detachSkillFromPlayer(player, str);
                player->loseSkill(str);
            }
        }
        if(hp <= 5)
            room->acquireSkill(player, skills.at(hp - 1));

        return false;
    }
};

Fiveline::Fiveline(Suit suit, int number) :Armor(suit, number){
    setObjectName("fiveline");
    skill = new FivelineSkill;
}

void Fiveline::onInstall(ServerPlayer *player) const{
    EquipCard::onInstall(player);
    QVariantList skills;
    if(player->hasSkill("rende"))
        skills << "rende";
    else if(player->hasSkill("jizhi"))
        skills << "jizhi";
    else if(player->hasSkill("jieyin"))
        skills << "jieyin";
    else if(player->hasSkill("guose"))
        skills << "guose";
    else if(player->hasSkill("kurou"))
        skills << "kurou";
    player->tag["fiveline"] = skills;
    player->getRoom()->setPlayerProperty(player, "hp", player->getHp());
}

void Fiveline::onUninstall(ServerPlayer *player) const{
    if(player->isDead())
        return;
    QStringList skills;
    skills << "rende" << "jizhi" << "jieyin" << "guose" << "kurou";
    QVariantList has_skills = player->tag["fiveline"].toList();
    foreach(QString str, skills){
        if(has_skills.contains(str))
            continue;
        else{
            player->getRoom()->detachSkillFromPlayer(player, str);
            player->loseSkill(str);
        }
    }
}

KusoCardPackage::KusoCardPackage()
    :Package("kuso_cards")
{
    (new KawaiiDress(Card::Spade, 2))->setParent(this);
    (new Fiveline(Card::Heart, 5))->setParent(this);

    type = CardPack;
}

ADD_PACKAGE(Kuso)
ADD_PACKAGE(KusoCard)
