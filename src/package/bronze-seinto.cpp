#include "standard.h"
#include "skill.h"
#include "bronze-seinto.h"
#include "client.h"
#include "carditem.h"
#include "engine.h"
#include "maneuvering.h"
#include "ai.h"

class Liuxing: public TriggerSkill{
public:
    Liuxing():TriggerSkill("liuxing"){
        events << Predamage;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();

        const Card *reason = damage.card;
        if(!reason->inherits("Slash"))
            return false;

        if(damage.to->getEquips().length() != 0){
            Room *room = player->getRoom();
            if(!room->askForSkillInvoke(player, objectName(), data))
                return false;
            room->playSkillEffect("liuxing");
            LogMessage log;
            log.type = "#LiuxingBuff";
            log.from = player;
            log.to << damage.to;
            log.arg = QString::number(damage.damage);
            log.arg2 = QString::number(damage.damage + 1);
            room->sendLog(log);

            damage.damage ++;
            data = QVariant::fromValue(damage);
        }

        return false;
    }
};

class Huixuan: public PhaseChangeSkill{
public:
    Huixuan():PhaseChangeSkill("huixuan"){
        frequency = Wake;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target)
                && target->getMark("huixuan") == 0
                && target->getPhase() == Player::Start
                && target->getHandcardNum() > target->getMaxHP();
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        Room *room = player->getRoom();

        LogMessage log;
        log.type = "#HuixuanWake";
        log.from = player;
        room->sendLog(log);

        if(room->askForChoice(player, objectName(), "recover+draw") == "recover"){
            RecoverStruct recover;
            recover.who = player;
            room->recover(player, recover);
        }else
            room->drawCards(player, 2);

        room->playSkillEffect("huixuan");
        room->setPlayerMark(player, "huixuan", 1);
        room->acquireSkill(player, "renma");
        room->acquireSkill(player, "liegong");

        room->loseMaxHp(player);

        return false;
    }
};

class Shenzui: public PhaseChangeSkill{
public:
    Shenzui():PhaseChangeSkill("shenzui$"){
        frequency = Wake;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target)
                && target->hasLordSkill("shenzui")
                && target->getMark("shenzui") == 0
                && target->getPhase() == Player::Start
                && target->getHp() == 1;
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        Room *room = player->getRoom();

        LogMessage log;
        log.type = "#ShenzuiWake";
        log.from = player;
        room->sendLog(log);

        room->setPlayerMark(player, "shenzui", 1);
        room->acquireSkill(player, "buqu");
        room->acquireSkill(player, "keji");

        room->loseMaxHp(player);

        return false;
    }
};

ShenglongCard::ShenglongCard(){
    will_throw = false;
    once = true;
}

bool ShenglongCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select != Self && !to_select->isKongcheng();
}

void ShenglongCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    ServerPlayer *s = targets.first();
    source->pindian(s, "shenglong", this);
}

class ShenglongPindian: public OneCardViewAsSkill{
public:
    ShenglongPindian():OneCardViewAsSkill("shenglong_pindian"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->isKongcheng() && ! player->hasUsed("ShenglongCard");
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return ! to_select->isEquipped();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        ShenglongCard *card = new ShenglongCard;
        card->addSubcard(card_item->getFilteredCard());

        return card;
    }
};

class Shenglong: public TriggerSkill{
public:
    Shenglong():TriggerSkill("shenglong"){
        view_as_skill = new ShenglongPindian;
        events << Pindian << PhaseChange << GameStart;
    }

    virtual int getPriority() const{
        return -1;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        if(event == GameStart){
            if(player->hasSkill("tieji"))
                player->setMark("ismachao", 1);
            return false;
        }
        else if(event == PhaseChange){
            if(player->getPhase() == Player::NotActive && player->hasSkill("tieji") && player->getMark("ismachao") == 0)
                player->getRoom()->detachSkillFromPlayer(player, "tieji");
            return false;
        }
        PindianStar pindian = data.value<PindianStar>();
        Room *room = player->getRoom();
        if(pindian->reason == "shenglong" && pindian->from == player){
            if(pindian->isSuccess()){
                player->obtainCard(pindian->to_card);
                room->acquireSkill(player, "tieji");
                pindian->to->obtainCard(pindian->from_card);
            }
        }
        return false;
    }
};

class Showhu: public PhaseChangeSkill{
public:
    Showhu():PhaseChangeSkill("showhu$"){
        frequency = Wake;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target)
                && target->hasLordSkill("showhu")
                && target->getMark("showhu") == 0
                && target->getPhase() == Player::Start
                && target->isKongcheng();
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        Room *room = player->getRoom();

        LogMessage log;
        log.type = "#ShowhuWake";
        log.from = player;
        room->sendLog(log);

        room->setPlayerMark(player, "showhu", 1);
        room->acquireSkill(player, "kurou");
        room->acquireSkill(player, "benghuai");
        room->acquireSkill(player, "yizhong");

        return false;
    }
};

class Zuanxing: public TriggerSkill{
public:
    Zuanxing():TriggerSkill("zuanxing"){
        events << SlashHit;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        SlashEffectStruct effect = data.value<SlashEffectStruct>();

        Room *room = player->getRoom();

        if(!effect.to->isNude() && player->askForSkillInvoke(objectName(), data)){
            room->playSkillEffect("zuanxing");
            int card_id = room->askForCardChosen(player, effect.to, "he", objectName());
            room->throwCard(card_id);

            if(!effect.to->isNude()){
                card_id = room->askForCardChosen(player, effect.to, "he", objectName());
                room->throwCard(card_id);
            }

            return true;
        }

        return false;
    }
};

class Jinguang: public PhaseChangeSkill{
public:
    Jinguang():PhaseChangeSkill("jinguang"){
        frequency = Wake;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target)
                && target->getMark("jinguang") == 0
                && target->getPhase() == Player::Start
                && target->getHp() == 1;
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        Room *room = player->getRoom();

        LogMessage log;
        log.type = "#JinguangWake";
        log.from = player;
        room->sendLog(log);

        room->playSkillEffect("jinguang");
        room->setPlayerMark(player, "jinguang", 1);
        room->acquireSkill(player, "shushu");
        room->acquireSkill(player, "ganglie");

        room->loseMaxHp(player);

        return false;
    }
};

class Suolian:public OneCardViewAsSkill{
public:
    Suolian():OneCardViewAsSkill("suolian"){

    }

    virtual bool viewFilter(const CardItem *to_select) const{
        const Card *card = to_select->getFilteredCard();

        switch(ClientInstance->getStatus()){
        case Client::Playing:{
                return card->inherits("Jink") || card->inherits("Slash");
            }

        case Client::Responsing:{
                QString pattern = ClientInstance->getPattern();
                if(pattern == "slash" || pattern == "jink")
                    return card->inherits("IronChain");
            }

        default:
            return false;
        }
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "jink" || pattern == "slash";
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getFilteredCard();
        if(card->inherits("Slash") || card->inherits("Jink")){
            IronChain *chain = new IronChain(card->getSuit(), card->getNumber());
            chain->addSubcard(card);
            chain->setSkillName(objectName());
            return chain;
        }
        else if(card->inherits("IronChain")){
            if(ClientInstance->getPattern() == "slash"){
                Slash *slash = new Slash(card->getSuit(), card->getNumber());
                slash->addSubcard(card);
                slash->setSkillName(objectName());
                return slash;
            }
            else {
                Jink *jink = new Jink(card->getSuit(), card->getNumber());
                jink->addSubcard(card);
                jink->setSkillName(objectName());
                return jink;
            }
        }
        else
            return NULL;
    }
};

XingyunCard::XingyunCard(){
    once = true;
}

bool XingyunCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty();
}

void XingyunCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    QList<int> card_ids = room->getNCards(5);
    room->fillAG(card_ids);

    int n = 0;
    while(n < (5 - effect.from->getLostHp())){
        int card_id = room->askForAG(effect.from, card_ids, false, "xingyun");
        //effect.to->obtainCard(Sanguosha->getCard(card_id));
        card_ids.removeOne(card_id);
        room->takeAG(effect.to, card_id);
        n ++;
    }

    room->broadcastInvoke("clearAG");
    effect.to->turnOver();
}

class Xingyun: public ZeroCardViewAsSkill{
public:
    Xingyun():ZeroCardViewAsSkill("xingyun"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("XingyunCard");
    }

    virtual const Card *viewAs() const{
        return new XingyunCard;
    }
};

class Qiliu: public TriggerSkill{
public:
    Qiliu():TriggerSkill("qiliu"){
        events << Predamage;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *q, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();

        const Card *reason = damage.card;
        if(reason == NULL || damage.to->faceUp())
            return false;

        if(reason->inherits("Slash")){
            q->getRoom()->playSkillEffect("qiliu");
            LogMessage log;
            log.type = "#QiliuBuff";
            log.from = q;
            log.to << damage.to;
            log.arg = QString::number(damage.damage);
            log.arg2 = QString::number(damage.damage + 1);
            q->getRoom()->sendLog(log);

            damage.damage ++;
            data = QVariant::fromValue(damage);
        }

        return false;
    }
};

HuanmoCard::HuanmoCard(){
    once = true;
}

bool HuanmoCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(targets.isEmpty())
        return true;
    else if(targets.length() == 1)
        return targets.first()->canSlash(to_select);
    else
        return false;
}

bool HuanmoCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    return targets.length() == 2;
}

void HuanmoCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    room->throwCard(this);

    QString prompt = QString("huanmo-slash:%1:%2")
                     .arg(source->objectName()).arg(targets.at(1)->objectName());
    const Card *card = room->askForCard(targets.at(0), "slash", prompt);
    if(card){
        CardUseStruct use;
        use.card = card;
        use.from = targets.at(0);
        use.to << targets.at(1);
        room->useCard(use);
    }
    else{
        DamageStruct damage;
        damage.from = damage.to = targets.at(0);
        room->damage(damage);
    }
}

class Huanmo: public OneCardViewAsSkill{
public:
    Huanmo():OneCardViewAsSkill("huanmo"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("HuanmoCard");
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getCard()->isRed();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        HuanmoCard *Huanmo_card = new HuanmoCard;
        Huanmo_card->addSubcard(card_item->getCard()->getId());

        return Huanmo_card;
    }
};

class Fengyi: public TriggerSkill{
public:
    Fengyi():TriggerSkill("fengyi"){
        events << SlashEffect << Predamage;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        if(event == SlashEffect){
            SlashEffectStruct effect = data.value<SlashEffectStruct>();
            effect.nature = DamageStruct::Fire;
            data = QVariant::fromValue(effect);
        }
        else if(event == Predamage){
            DamageStruct damage = data.value<DamageStruct>();

            const Card *reason = damage.card;
            if(reason == NULL)
                return false;

            if(reason->inherits("Slash") && damage.nature == DamageStruct::Fire){
                player->getRoom()->playSkillEffect("fengyi");
                LogMessage log;
                log.type = "#FengyiBuff";
                log.from = player;
                log.to << damage.to;
                log.arg = QString::number(damage.damage);
                log.arg2 = QString::number(damage.damage + 1);
                player->getRoom()->sendLog(log);

                damage.damage ++;
                data = QVariant::fromValue(damage);
            }
        }

        return false;
    }
};

BronzeSeintoPackage::BronzeSeintoPackage()
    :Package("bronzeseinto")
{
    General *seiya, *shiryu, *ikki, *shum, *hyoga;

    seiya = new General(this, "seiya$", "ao");
    seiya->addSkill(new Liuxing);
    seiya->addSkill(new Huixuan);
    seiya->addSkill(new Shenzui);

    shiryu = new General(this, "shiryu$", "ao");
    shiryu->addSkill(new Shenglong);
    shiryu->addSkill(new Showhu);

    hyoga = new General(this, "hyoga", "ao");
    hyoga->addSkill(new Zuanxing);
    hyoga->addSkill(new Jinguang);

    shum = new General(this, "shum", "ao", 3);
    shum->addSkill(new Suolian);
    shum->addSkill(new Xingyun);
    shum->addSkill(new Qiliu);

    ikki = new General(this, "ikki", "ao");
    ikki->addSkill(new Huanmo);
    ikki->addSkill(new Fengyi);

    addMetaObject<ShenglongCard>();
    addMetaObject<XingyunCard>();
    addMetaObject<HuanmoCard>();
}

ADD_PACKAGE(BronzeSeinto)
