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

ShaobingCard::ShaobingCard(){
    once = true;
}

bool ShaobingCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;

    return to_select != Self;
}

void ShaobingCard::use(Room *room, ServerPlayer *OEhyk, const QList<ServerPlayer *> &targets) const{
    room->loseHp(OEhyk);
    room->setPlayerProperty(targets.first(), "maxhp", targets.first()->getMaxHP()+1);
}

class Shaobing: public ZeroCardViewAsSkill{
public:
    Shaobing():ZeroCardViewAsSkill("shaobing"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("ShaobingCard");
    }

    virtual const Card *viewAs() const{
        return new ShaobingCard;
    }
};

class Maozhua:public OneCardViewAsSkill{
public:
    Maozhua():OneCardViewAsSkill("maozhua"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return Slash::IsAvailable(player);
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern == "slash";
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        const Card *card = to_select->getFilteredCard();

        if(card->getSuit() != Card::Club)
            return false;

        if(card == Self->getWeapon() && card->objectName() == "crossbow")
            return Self->canSlashWithoutCrossbow();
        else
            return true;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getCard();
        Card *slash = new Slash(card->getSuit(), card->getNumber());
        slash->addSubcard(card->getId());
        slash->setSkillName(objectName());
        return slash;
    }
};

class Aiqing: public TriggerSkill{
public:
    Aiqing():TriggerSkill("aiqing"){
        events << Predamaged;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        if(event == Predamaged){
            DamageStruct damage = data.value<DamageStruct>();
            if(damage.from->getGeneral()->isMale() != player->getGeneral()->isMale()){
                if(!player->askForSkillInvoke(objectName()))
                    return false;
                JudgeStruct judge;
                judge.pattern = QRegExp("(.*):(heart):(.*)");
                judge.good = true;
                judge.reason = "aiqing";
                judge.who = player;

                room->judge(judge);

                if(judge.isBad()){
                    return false;
                }
                /*
                LogMessage log;
                log.type = "#AiqingProtect";
                log.to << player;
                log.from = damage.from;
                room->sendLog(log);
                */
                return true;
            }
        }
        return false;
    }
};

class Shuihun: public ViewAsSkill{
public:
    Shuihun():ViewAsSkill("shuihun"){

    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return /* pattern == "slash"
                || pattern == "jink"
                || pattern.contains("peach")
                ||*/ pattern == "nullification";
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return true;//player->isWounded() || Slash::IsAvailable(player);
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        const Card *card = to_select->getFilteredCard();
        int n = qMax(1, Self->getHp());

        if(selected.length() >= n)
            return false;

        if(n > 1 && !selected.isEmpty()){
            Card::Suit suit = selected.first()->getFilteredCard()->getSuit();
            return card->getSuit() == suit;
        }

        switch(ClientInstance->getStatus()){
        case Client::Playing:{
                /*if(Self->isWounded() && card->getSuit() == Card::Heart)
                    return true;
                else if(Slash::IsAvailable(Self) && card->getSuit() == Card::Diamond)
                    return true;
                else
                    return false;*/
                if(card->getSuit() != Card::Club)
                    return true;
            }

        case Client::Responsing:{
                QString pattern = ClientInstance->getPattern();
                /*if(pattern == "jink")
                    return card->getSuit() == Card::Club;
                else */if(pattern == "nullification")
                    return card->getSuit() == Card::Club;/*Spade;
                else if(pattern == "peach" || pattern == "peach+analeptic")
                    return card->getSuit() == Card::Heart;
                else if(pattern == "slash")
                    return card->getSuit() == Card::Diamond;*/
            }

        default:
            break;
        }

        return false;
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        int n = qMax(1, Self->getHp());

        if(cards.length() != n)
            return NULL;

        const Card *card = cards.first()->getFilteredCard();
        Card *new_card = NULL;

        Card::Suit suit = card->getSuit();
        int number = cards.length() > 1 ? 0 : card->getNumber();
        switch(card->getSuit()){
        case Card::Spade:{
                new_card = new SavageAssault(suit, number);
                break;
            }

        case Card::Heart:{
                new_card = new ExNihilo(suit, number);
                break;
            }

        case Card::Club:{
                new_card = new Nullification(suit, number);
                break;
            }

        case Card::Diamond:{
                new_card = new ArcheryAttack(suit, number);
                break;
            }
        default:
            break;
        }

        if(new_card){
            new_card->setSkillName(objectName());
            new_card->addSubcards(cards);
        }

        return new_card;
    }
};

class Huanshen: public TriggerSkill{
public:
    Huanshen():TriggerSkill("huanshen"){
        events << Predamaged;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        //Room *room = player->getRoom();
        if(event == Predamaged){
            DamageStruct damage = data.value<DamageStruct>();
            if(damage.nature != DamageStruct::Normal){
                /*
                LogMessage log;
                log.type = "#HuanshenProtect";
                log.to << player;
                log.from = damage.from;
                room->sendLog(log);
                */
                return true;
            }
        }
        return false;
    }
};

class Jiyu: public TriggerSkill{
public:
    Jiyu():TriggerSkill("jiyu"){
        events << Damage << Damaged;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        /*
        LogMessage log;
        log.type = event == Damage ? "#JiyuDamage" : "#JiyuDamaged";
        log.from = player;
        log.arg = QString::number(damage.damage);
        player->getRoom()->sendLog(log);
        */
        player->gainMark("@lywater", damage.damage);
        //player->getRoom()->playSkillEffect(objectName());

        return false;
    }
};

DuihuanCard::DuihuanCard(){

}

bool DuihuanCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;

    return true;
}

void DuihuanCard::use(Room *room, ServerPlayer *OEsilvlryr, const QList<ServerPlayer *> &targets) const{
    OEsilvlryr->loseMark("@lywater", 2);
    if(targets.first() == OEsilvlryr)
        OEsilvlryr->drawCards(2);
    else{
        DamageStruct damage;
        damage.from = OEsilvlryr;
        damage.to = targets.first();
        damage.card = NULL;

        room->damage(damage);
    }
}

class Duihuan: public ZeroCardViewAsSkill{
public:
    Duihuan():ZeroCardViewAsSkill("duihuan"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getMark("@lywater") >= 2;
    }

    virtual const Card *viewAs() const{
        return new DuihuanCard;
    }
};

class Jiangyou: public TriggerSkill{
public:
    Jiangyou():TriggerSkill("jiangyou"){
        events << Damaged;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        Room *room = player->getRoom();
        ServerPlayer *OEsrhrsr = room->findPlayerBySkillName(objectName());
        if(damage.nature != DamageStruct::Normal){
            RecoverStruct recover;
            recover.who = damage.from;
            recover.recover = damage.damage;
            room->recover(OEsrhrsr, recover, true);
        }
        if(damage.card && damage.card->inherits("Slash"))
            OEsrhrsr->drawCards(damage.damage);
        return false;
    }
};

BaiheCard::BaiheCard(){
}

bool BaiheCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(targets.isEmpty())
        return true;
    if(targets.length() == 1)
        return to_select->getGeneral()->isFemale() && targets.first()->getGeneral()->isFemale();
    return false;
}

void BaiheCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    ServerPlayer *a = targets.at(0);
    ServerPlayer *b = targets.at(1);

    room->throwCard(this);

    RecoverStruct recover;
    recover.card = this;
    recover.who = source;
    room->recover(a, recover, true);
    room->recover(b, recover, true);

}

class Baihe: public ViewAsSkill{
public:
    Baihe():ViewAsSkill("baihe"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("BaiheCard");
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(selected.length() > 1)
            return false;

        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() != 1)
            return NULL;

        BaiheCard *baihe_card = new BaiheCard();
        baihe_card->addSubcards(cards);

        return baihe_card;
    }
};

class Fadai: public TriggerSkill{
public:
    Fadai():TriggerSkill("fadai"){
        events << PhaseChange << Predamaged;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *OEwbolir = room->findPlayerBySkillName(objectName());
        if(OEwbolir->getPhase() == Player::Finish){
            if(room->askForSkillInvoke(OEwbolir, objectName())){
                OEwbolir->gainMark("fadai");
                //room->playSkillEffect(objectName());
                OEwbolir->throwAllCards();
            }
            return false;
        }
        else if(OEwbolir->getPhase() == Player::Start){
            OEwbolir->loseAllMarks("fadai");
            return false;
        }
        else if(event == Predamaged){
            DamageStruct damage = data.value<DamageStruct>();
            if(OEwbolir->getMark("fadai") && damage.to == OEwbolir
               && !damage.card->inherits("SavageAssault") && !damage.card->inherits("ArcheryAttack"))
                return true;
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

    OEubun = new General(this, 3103, "ubun$", "tan", 4, true);
    OEubun->addSkill(new Niubi);
    OEubun->addSkill(new Meikong);
    OEubun->addSkill(new Maimeng);

    OEtenkei = new General(this, 3136, "tenkei", "tan", 3, false);
    OEtenkei->addSkill(new Zhongpao);
    OEtenkei->addSkill(new Guisu);
    OEtenkei->addSkill(new Menghua);

    General *OEhyk, *OEgoldlryr, *OEsilvlryr, *OEsrhrsr, *OEwbolir;
    OEhyk = new General(this, 3118, "hyk3374", "tan", 4, true);
    OEhyk->addSkill(new Shaobing);
    OEhyk->addSkill(new Maozhua);
    OEhyk->addSkill(new Aiqing);

    OEgoldlryr = new General(this, 3119, "goldsiyeliuyue", "tan", 2, false);
    OEgoldlryr->addSkill(new Shuihun);
    OEgoldlryr->addSkill(new Huanshen);

    OEsilvlryr = new General(this, 3120, "silversiyeliuyue", "tan", 3, false);
    OEsilvlryr->addSkill(new Jiyu);
    OEsilvlryr->addSkill(new Duihuan);

    OEsrhrsr = new General(this, 3123, "shihunzhishi", "tan", 2, true);
    OEsrhrsr->addSkill(new Jiangyou);

    OEwbolir = new General(this, 3124, "wocaokongming", "tan", 2, false);
    OEwbolir->addSkill(new Baihe);
    OEwbolir->addSkill(new Fadai);

    addMetaObject<MaimengCard>();
    addMetaObject<ShaobingCard>();
    addMetaObject<DuihuanCard>();
    addMetaObject<BaiheCard>();
    skills << new Mod << new MaimengViewAsSkill;
}

ADD_PACKAGE(OETan)

