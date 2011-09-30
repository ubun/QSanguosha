#include "standard.h"
#include "skill.h"
#include "gold-seinto.h"
#include "client.h"
#include "carditem.h"
#include "engine.h"
#include "maneuvering.h"
#include "ai.h"

class XiufuE: public TriggerSkill{
public:
    XiufuE():TriggerSkill("#xiufu-effect"){
        events << AskForPeaches;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *mu = room->findPlayerBySkillName("xiufu");
        DyingStruct dying = data.value<DyingStruct>();
        if(!mu || dying.who != player)
            return false;

        const Card *cardt = room->askForCard(mu, ".H", "@xiufu");
        if(cardt){
            room->throwCard(cardt->getId());

            LogMessage log;
            log.type = "#Xiufu";
            log.from = player;
            log.to << dying.who;
            log.arg = "xiufu";
            room->sendLog(log);
            room->setPlayerProperty(dying.who, "hp", 1);
        }

        return false;
    }
};

class Xiufu: public OneCardViewAsSkill{
public:
    Xiufu():OneCardViewAsSkill("xiufu"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("XiufuCard") && player->isWounded();
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getCard()->getSuit() == Card::Heart;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        XiufuCard *card = new XiufuCard;
        card->addSubcard(card_item->getCard()->getId());

        return card;
    }
};

XiufuCard::XiufuCard(){
    once = true;
    target_fixed = true;
}
void XiufuCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    room->throwCard(this);
    room->cardEffect(this, source, source);
}
void XiufuCard::onEffect(const CardEffectStruct &effect) const{
    RecoverStruct recover;
    recover.card = this;
    recover.who = effect.from;
    effect.from->getRoom()->recover(effect.to, recover);
}

class Xingmie: public TriggerSkill{
public:
    Xingmie():TriggerSkill("xingmie"){
        events << Damage;
        frequency = Limited;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target) && target->getMark("@xm") > 0;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        DamageStruct damage = data.value<DamageStruct>();
        if(room->askForCard(player, ".S", "xingmie-ask")){
            player->loseMark("@xm");

            LogMessage log;
            log.type = "#InvokeSkill";
            log.from = player;
            log.arg = objectName();
            room->sendLog(log);

            room->broadcastInvoke("animate", "lightbox:$xingmie");
            QList<ServerPlayer *> players = room->getOtherPlayers(player);
            foreach(ServerPlayer *p, players){
                if(!player->inMyAttackRange(p)){
                    DamageStruct damage2;
                    damage2.nature = damage.nature;
                    damage2.from = player;
                    damage2.to = p;
                    room->damage(damage2);
                }
            }
        }
        return false;
    }
};

class Hao2jiao: public TriggerSkill{
public:
    Hao2jiao():TriggerSkill("hao2jiao"){
        events << SlashProceed << Predamage;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        if(event == Predamage){
            DamageStruct damage = data.value<DamageStruct>();
            if(!damage.card->inherits("Slash") || damage.from != player)
                return false;
            CardStar card = player->tag.value("hj", NULL).value<CardStar>();
            if(card && card->getSuit() == Card::Diamond)
                damage.damage ++;
            else if(card && card->getSuit() == Card::Heart)
                damage.from->obtainCard(card);
            player->tag.remove("hj");
            data = QVariant::fromValue(damage);
        }
        else if(event == SlashProceed && player->askForSkillInvoke(objectName(), data)){
            SlashEffectStruct effect = data.value<SlashEffectStruct>();
            JudgeStruct judge;
            judge.pattern = QRegExp("(.*):(heart|diamond):(.*)");
            judge.good = true;
            judge.reason = objectName();
            judge.who = effect.from;
            room->judge(judge);

            effect.from->tag["hj"] = QVariant::fromValue(judge.card);
            if(judge.isGood()){
                room->slashResult(effect, NULL);
                return true;
            }
        }
        return false;
    }
};

HuanlongCard::HuanlongCard(){
    will_throw = false;
    once = true;
}

bool HuanlongCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select != Self && !to_select->isKongcheng();
}

void HuanlongCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    ServerPlayer *sunce = targets.first();
    source->pindian(sunce, "huanlong", this);
}

class HuanlongPindian: public OneCardViewAsSkill{
public:
    HuanlongPindian():OneCardViewAsSkill("huanlong_pindian"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->isKongcheng() && ! player->hasUsed("HuanlongCard");
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return ! to_select->isEquipped();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        HuanlongCard *card = new HuanlongCard;
        card->addSubcard(card_item->getFilteredCard());

        return card;
    }
};

class Huanlong: public TriggerSkill{
public:
    Huanlong():TriggerSkill("huanlong"){
        view_as_skill = new HuanlongPindian;
        events << Pindian;
    }

    virtual int getPriority() const{
        return -1;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        PindianStar pindian = data.value<PindianStar>();
        Room *room = player->getRoom();
        if(pindian->reason == "huanlong" && pindian->from == player){
            if(pindian->isSuccess()){
                QList<ServerPlayer *> players;
                ServerPlayer *tmp = NULL;
                players << pindian->to;
                foreach(tmp, room->getAllPlayers()){
                    if(pindian->to->inMyAttackRange(tmp))
                        players << tmp;
                }
                if(!players.isEmpty()){
                    tmp = room->askForPlayerChosen(player, players, objectName());
                    DamageStruct damage;
                    damage.from = pindian->to;
                    damage.to = tmp;
                    room->damage(damage);
                }
            }
            else{
                DamageStruct damage;
                damage.from = pindian->to;
                damage.to = player;
                room->damage(damage);
            }
        }
        return false;
    }
};

YinheCard::YinheCard(){
}

void YinheCard::use(Room *room, ServerPlayer *p, const QList<ServerPlayer *> &targets) const{
    room->broadcastInvoke("animate", "lightbox:$yinhe");
    p->loseMark("@star", 3);    
    p->loseMark("@yinh");
    Card::use(room, p, targets);
}

void YinheCard::onEffect(const CardEffectStruct &effect) const{
    effect.from->getRoom()->loseMaxHp(effect.to);
}

class Yinhe: public ZeroCardViewAsSkill{
public:
    Yinhe():ZeroCardViewAsSkill("yinhe"){
        frequency = Limited;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getMark("@yinh") >= 1 && player->getMark("@star") > 2;
    }

    virtual const Card *viewAs() const{
        return new YinheCard;
    }
};

MohuaCard::MohuaCard(){
}

bool MohuaCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(targets.isEmpty() && to_select->getKingdom() != "st")
        return false;
    if(to_select == Self)
        return false;
    if(targets.length() == 1 && !Self->canSlash(to_select))
        return false;
    return true;
}

bool MohuaCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    return targets.length() == 2;
}

void MohuaCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    ServerPlayer *knife = targets.first();
    ServerPlayer *guy = targets.last();
    const Card *slash = room->askForCard(knife, "slash", "mohua-ask");
    CardUseStruct card_use;
    card_use.from = source;
    card_use.to << guy;
    while(slash && guy && knife){
        card_use.card = slash;
        source->setFlags("mohua");
        room->useCard(card_use);
        slash = NULL;
        if(!guy->isAlive())
            guy = guy->getNextAlive();
        if(!knife) return;
        slash = room->askForCard(knife, "slash", "mohua-ask");
    }
    source->addMark("mohua");
    knife->addMark("mohua");
}

class MohuaViewAsSkill: public ZeroCardViewAsSkill{
public:
    MohuaViewAsSkill():ZeroCardViewAsSkill("mohua$"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->hasLordSkill("mohua");
    }

    virtual const Card *viewAs() const{
        return new MohuaCard;
    }
};

class Mohua:public TriggerSkill{
public:
    Mohua():TriggerSkill("mohua"){
        events << PhaseChange << Predamage;
        view_as_skill = new MohuaViewAsSkill;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        if(event == PhaseChange){
           if(player->getPhase() != Player::NotActive)
               return false;
           Room *rm = player->getRoom();
           foreach(ServerPlayer *tmp, rm->getAlivePlayers()){
               if(tmp->getMark("mohua") > 0){
                   tmp->turnOver();
                   tmp->setMark("mohua", 0);
               }
           }
           return false;
        }
        if(!player->hasFlag("mohua"))
            return false;
        DamageStruct damage = data.value<DamageStruct>();
        const Card *reason = damage.card;

        if(reason && reason->inherits("Slash")){
            LogMessage log;
            log.type = "#MohuaBuff";
            log.from = player;
            log.to << damage.to;
            log.arg = QString::number(damage.damage);
            log.arg2 = QString::number(damage.damage + 1);
            player->getRoom()->sendLog(log);

            damage.damage ++;
            player->setFlags("-mohua");
            data = QVariant::fromValue(damage);
        }
        return false;
    }
};

class Mingbo: public TriggerSkill{
public:
    Mingbo(): TriggerSkill("mingbo"){
        events << Predamage;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        Room *room = player->getRoom();
        if(damage.card && damage.card->inherits("Slash") && room->askForSkillInvoke(player, objectName(), data)){
            int card_id = room->drawCard();

            LogMessage log;
            log.type = "$Mingbo";
            log.from = player;
            log.card_str = QString::number(card_id);
            room->sendLog(log);

            room->getThread()->delay();
            const Card *card = Sanguosha->getCard(card_id);
            if(card->getSuit() == Card::Spade &&
               card->getNumber() > 1 && card->getNumber() < 10){
                /*
                LogMessage log;
                log.type = "#Mingbo";
                log.from = player;
                log.arg = objectName();
                room->sendLog(log);
                */
                damage.damage += 2;
                damage.nature = DamageStruct::Thunder;
            }
            else if(card->getSuit() == Card::Heart &&
                    card->getNumber() < 11){
                damage.damage ++;
                damage.nature = DamageStruct::Fire;
            }
            room->throwCard(card_id);
            data = QVariant::fromValue(damage);
        }
        return false;
    }
};

class Dian2guang: public TriggerSkill{
public:
    Dian2guang(): TriggerSkill("dian2guang"){
        events << Predamage;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        damage.nature = DamageStruct::Thunder;

        LogMessage log;
        log.type = "#TriggerSkill";
        log.arg = objectName();
        log.from = player;
        player->getRoom()->sendLog(log);

        data = QVariant::fromValue(damage);
        return false;
    }
};

class Lizi: public TriggerSkill{
public:
    Lizi(): TriggerSkill("lizi"){
        events << Predamage;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        Room *room = player->getRoom();
        if(damage.card && damage.card->inherits("Slash") &&
           room->askForCard(player, ".LZ", "@lizi", data)){
            LogMessage log;
            log.type = "#InvokeSkill";
            log.arg = objectName();
            log.from = player;
            room->sendLog(log);

            damage.damage ++;
        }
        data = QVariant::fromValue(damage);
        return false;
    }
};

class LiziPattern: public CardPattern{
public:
    virtual bool match(const Player *player, const Card *card) const{
        return card->inherits("ThunderSlash") ||
                card->inherits("Jink") ||
                card->inherits("Lightning");
    }
};

class Xuechi:public MasochismSkill{
public:
    Xuechi():MasochismSkill("xuechi"){
    }

    virtual void onDamaged(ServerPlayer *player, const DamageStruct &damage) const{
        Room *room = player->getRoom();
        for(int i = damage.damage; i > 0; i--){
            if(room->askForCard(player, "slash", "@xuechi-slash", QVariant::fromValue(damage.card))){
                SavageAssault *card = new SavageAssault(Card::NoSuit, 0);
                card->setSkillName("xuechi");
                CardUseStruct card_use;
                card_use.card = card;
                card_use.from = player;
                room->useCard(card_use);
            }
        }
    }
};

class SixDao:public PhaseChangeSkill{
public:
    SixDao():PhaseChangeSkill("sixd"){
        frequency = Frequent;
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        if(player->getPhase() == Player::Start && player->askForSkillInvoke(objectName())){
            JudgeStruct judge;
            judge.pattern = QRegExp("(.*):(heart):([1-6])");
            judge.good = true;
            judge.reason = "sixd";
            judge.who = player;

            player->getRoom()->judge(judge);
            LogMessage log;
            log.type = judge.isGood() ? "#Sixd1" : "#Sixd2";
            log.from = player;
            player->getRoom()->sendLog(log);
            if(judge.isGood())
                player->setFlags("six");
        }
        return false;
    }
};

class SixDaoBuff: public TriggerSkill{
public:
    SixDaoBuff(): TriggerSkill("#sixdaobf"){
        events << Predamage;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(player->hasFlag("six")){
            damage.damage ++;

            LogMessage log;
            log.type = "#SixdBuff";
            log.arg = "sixd";
            log.from = player;
            player->getRoom()->sendLog(log);
        }
        data = QVariant::fromValue(damage);
        return false;
    }
};

class Jiangmo: public TriggerSkill{
public:
    Jiangmo():TriggerSkill("jiangmo"){
        events << GameStart << PhaseChange;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        if(event == GameStart){
            room->acquireSkill(player, "xuechi");
            room->acquireSkill(player, "sixd");
            player->setMark("stAte", 1);
            return false;
        }
        if(player->getPhase() == Player::Start && player->getMark("stAte") == 1
           && player->isKongcheng() && player->askForSkillInvoke(objectName(), data)){
            room->detachSkillFromPlayer(player, "xuechi");
            room->detachSkillFromPlayer(player, "sixd");
            player->setMark("stAte", 2);
            room->acquireSkill(player, "bao2lun");
            room->acquireSkill(player, "lianluo");
            room->loseMaxHp(player);
        }
        return false;
    }
};

class Bao2lun:public MasochismSkill{
public:
    Bao2lun():MasochismSkill("bao2lun"){
    }

    virtual void onDamaged(ServerPlayer *player, const DamageStruct &damage) const{
        Room *room = player->getRoom();
        if(damage.from && player->askForSkillInvoke(objectName())){
            JudgeStruct judge;
            judge.reason = objectName();
            judge.who = damage.from;
            room->judge(judge);

            LogMessage log;
            log.type = "$Baolun";
            log.from = damage.to;
            log.to << damage.from;
            log.card_str = QString::number(judge.card->getId());
            room->sendLog(log);

            QVariantList card_ids = damage.from->tag["feng"].toList();
            card_ids << judge.card->getId();
            damage.from->addMark(judge.card->getSuitString());
            damage.from->tag["feng"] = card_ids;
            damage.from->addToPile("feng", judge.card->getId());
            room->acquireSkill(damage.from, "bao3lun", false);
            if(damage.from->getMark("heart") > 0 &&
               damage.from->getMark("diamond") > 0 &&
               damage.from->getMark("spade") > 0 &&
               damage.from->getMark("club") >0){
                LogMessage log;
                log.type = "#BaolunDie";
                log.arg = "feng";
                log.from = damage.from;
                room->sendLog(log);

                room->killPlayer(damage.from);
            }
        }
    }
};

class Bao3lun: public TriggerSkill{
public:
    Bao3lun():TriggerSkill("bao3lun"){
        events << CardUsed;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        QVariantList card_ids = player->tag["feng"].toList();
        CardUseStruct use = data.value<CardUseStruct>();
        if(card_ids.isEmpty() || use.from != player)
            return false;
        Room *room = player->getRoom();
        foreach(QVariant n, card_ids){
            const Card *card = Sanguosha->getCard(n.toInt());
            if(card->getSuit() == use.card->getSuit() && card->getType() == use.card->getType()){
                if(use.from->getState() == "robot")
                    room->throwCard(use.card->getId());

                LogMessage log;
                log.type = "#BFeng";
                log.from = use.from;
                log.arg = use.card->getType();
                log.arg2 = use.card->getSuitString();
                room->sendLog(log);
                return true;
            }
        }

        return false;
    }
};

class LianluoEft:public PhaseChangeSkill{
public:
    LianluoEft():PhaseChangeSkill("#lianluo_effect"){
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        if(player->hasFlag("lianluo") && player->getPhase() == Player::Finish){
            int count = player->getRoom()->getAlivePlayers().length();
            player->drawCards(qMin(5, count));
            player->turnOver();
        }
        return false;
    }
};

class Lianluo:public ZeroCardViewAsSkill{
public:
    Lianluo():ZeroCardViewAsSkill("lianluo"){
        frequency = Limited;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getMark("@lianl") > 0;
    }

    virtual const Card *viewAs() const{
        return new LianluoCard;
    }
};

LianluoCard::LianluoCard(){
    target_fixed = true;
}

void LianluoCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    source->setFlags("lianluo");
    source->loseMark("@lianl");
    room->broadcastInvoke("animate", "lightbox:$lianluo");
    foreach(ServerPlayer *tmp, room->getAllPlayers())
        room->cardEffect(this, source, tmp);
}

void LianluoCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    room->loseHp(effect.to);
    room->loseMaxHp(effect.to);
}

class Shengqi: public TriggerSkill{
public:
    Shengqi():TriggerSkill("shengqi"){
        events << PhaseChange << CardLost;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill(objectName()) || target->getMark("@sqv") == 1;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        if(!player->isAlive()) return false;
        ServerPlayer *dohko = room->findPlayerBySkillName(objectName());
        if(dohko && event == PhaseChange && dohko->getPhase() == Player::Start){
            foreach(ServerPlayer *tmp, room->getAlivePlayers()){
                if(tmp->getMark("@sqv") != 0)
                    tmp->loseMark("@sqv", 1);
            }
            if(dohko->isKongcheng() || !dohko->askForSkillInvoke(objectName(), data))
                return false;
            ServerPlayer *target = room->askForPlayerChosen(player, room->getAllPlayers(), objectName());
            room->askForDiscard(dohko, objectName(), 1);
            target->gainMark("@sqv", 1);
        }
        else if(player->getMark("@sqv") == 1 && room->getCurrent() != player){
            if(event == CardLost){
                CardMoveStar move = data.value<CardMoveStar>();
                if(move->from_place == Player::Equip){
                    //player->tag["lostcard"] = QVariant::fromValue(move->card_id);
                    room->moveCardTo(Sanguosha->getCard(move->card_id), player, Player::Equip);

                    LogMessage log;
                    log.type = "$Shengqi";
                    log.from = player;
                    log.card_str = QString::number(move->card_id);
                    room->sendLog(log);
                    return true;
                }
            }
            /*else if(event == CardLostDone){
                int movecard = player->tag.value("lostcard").toInt();
                if(movecard > -1)
                    room->moveCardTo(Sanguosha->getCard(movecard), player, Player::Equip);
            }*/
        }
        return false;
    }
};

class Bailong:public TriggerSkill{
public:
    Bailong():TriggerSkill("bailong"){
        events << SlashEffect;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        Room *room = effect.from->getRoom();
        if(effect.from == player &&
           !player->isKongcheng() && !effect.to->isKongcheng()
           && room->askForDiscard(player, objectName(), 1, true)){
            int to_throw = room->askForCardChosen(effect.from, effect.to, "he", objectName());
            room->throwCard(to_throw);
        }

        return false;
    }
};

class Longxiang: public PhaseChangeSkill{
public:
    Longxiang():PhaseChangeSkill("longxiang"){
        frequency = Wake;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target)
                && target->getMark("longxiang") == 0
                && target->getPhase() == Player::Start;
    }

    virtual bool onPhaseChange(ServerPlayer *l) const{
        Room *room = l->getRoom();

        bool can_invoke = true;
        foreach(ServerPlayer *p, room->getAllPlayers()){
            if(l->getHp() > p->getHp()){
                can_invoke = false;
                break;
            }
        }

        if(can_invoke){
            room->playSkillEffect(objectName());

            LogMessage log;
            log.type = "#LongxiangWake";
            log.from = l;
            log.arg = QString::number(l->getHp());
            log.arg2 = objectName();
            room->sendLog(log);

            if(room->askForChoice(l, objectName(), "recover+draw") == "recover"){
                RecoverStruct recover;
                recover.who = l;
                room->recover(l, recover);
            }else
                room->drawCards(l, 2);

            room->setPlayerMark(l, "longxiang", 1);
            room->loseMaxHp(l);

            room->acquireSkill(l, "lianpo");
        }

        return false;
    }
};

class Duzhen: public TriggerSkill{
public:
    Duzhen():TriggerSkill("duzhen"){
        events << Damage << Damaged;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
/*
        LogMessage log;
        log.type = event == Damage ? "#DuzhenDamage" : "#DuzhenDamaged";
        log.from = player;
        log.arg = QString::number(damage.damage);
        player->getRoom()->sendLog(log);
*/
        ServerPlayer *target = event == Damage ? damage.to : damage.from;
        if(!target)
            return false;
        target->gainMark("@needle", damage.damage);

        int needle = target->getMark("@needle");
        if(needle < 5)
            return false;

        LogMessage log;
        log.type = "#Duzhen";
        log.from = target;
        log.arg = objectName();
        Room *room = target->getRoom();
        room->sendLog(log);

        JudgeStruct judge;
        judge.reason = objectName();
        judge.who = target;
        if(needle >= 5 && needle < 10 && target->getMark("needle5") == 0){
            room->judge(judge);
            if(judge.card->inherits("BasicCard")){
                int frog = (qrand() % 2) + 1;
                room->loseHp(target, frog);
            }
            target->setMark("needle5", 1);
        }
        else if(needle >= 10 && needle < 15 && target->getMark("needle10") == 0){
            room->judge(judge);
            if(judge.card->isRed()){
                int frog = qrand() % 3;
                room->loseMaxHp(target, frog);
            }
            target->setMark("needle10", 1);
        }
        else if(needle >= 15 && target->getMark("needle15") == 0){
            room->judge(judge);
            if(judge.card->inherits("Peach") || judge.card->inherits("Analeptic")){
                target->loseAllMarks("@needle");
                target->throwAllCards();
                target->setMark("needle15", 1);
            }
            else
                target->getRoom()->killPlayer(target);
        }
        return false;
    }
};

class Renma: public DistanceSkill{
public:
    Renma():DistanceSkill("renma"){

    }
    virtual int getCorrect(const Player *from, const Player *to) const{
        if(to->hasSkill(objectName()) && !to->getOffensiveHorse() && !to->getDefensiveHorse())
            return +1;
        else
            return 0;
    }
};

class Shesha: public SlashBuffSkill{
public:
    Shesha():SlashBuffSkill("shesha"){
    }

    virtual bool buff(const SlashEffectStruct &effect) const{
        Room *room = effect.from->getRoom();
        if(effect.nature != DamageStruct::Thunder && effect.nature != DamageStruct::Fire
           && effect.from->askForSkillInvoke(objectName(), QVariant::fromValue(effect))){
            //room->loseHp(effect.from);
            room->slashResult(effect, NULL);
            return true;
        }
        return false;
    }
};

Sheng2jianCard::Sheng2jianCard(){
    once = true;
    mute = true;
}

bool Sheng2jianCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(subcards.isEmpty())
        return Self->distanceTo(to_select) <= Self->getLostHp();
    else if(subcards.length() == 1)
        return Self->distanceTo(to_select) > Self->getLostHp();
    else
        return false;
}

void Sheng2jianCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    if(subcards.isEmpty()){
        Card *card = new Slash(Card::NoSuit, 0);
        CardUseStruct use;
        use.from = source;
        use.to << targets;
        use.card = card;
        room->useCard(use);
    }
    else{
        const Card *card = Sanguosha->getCard(subcards.first());
        CardUseStruct use;
        use.from = source;
        use.to << targets;
        use.card = card;
        room->useCard(use);
    }
}

class Sheng2jianViewAsSkill: public ViewAsSkill{
public:
    Sheng2jianViewAsSkill(): ViewAsSkill("shengjian"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "@shengjian-card";
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(selected.isEmpty())
            return to_select->getFilteredCard()->inherits("Slash");
        else
            return false;
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        Sheng2jianCard *card = new Sheng2jianCard;
        card->addSubcards(cards);
        card->setSkillName("shengjian");
        return card;
    }
};

class Sheng2jian: public TriggerSkill{
public:
    Sheng2jian(): TriggerSkill("sheng2jian"){
        events << PhaseChange;
        view_as_skill = new Sheng2jianViewAsSkill;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        if(player->getPhase() == Player::Start && player->isWounded()){
            player->getRoom()->askForUseCard(player, "@shengjian-card", "@shengjian");
        }

        return false;
    }
};
/*
class Sheng2jian: public OneCardViewAsSkill{
public:
    Sheng2jian():OneCardViewAsSkill("sheng2jian"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("Sheng2jianCard");
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getCard()->inherits("Slash");
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        Sheng2jianCard *card = new Sheng2jianCard;
        card->addSubcard(card_item->getCard()->getId());
        return card;
    }
};

Sheng2jianCard::Sheng2jianCard(){
    target_fixed = true;
    once = true;
}

void Sheng2jianCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    QList<int> see, seee;
    foreach(ServerPlayer *tmp, room->getAlivePlayers()){
        if(tmp->getWeapon())
            see << tmp->getWeapon()->getId();
    }

    int sigema = Sanguosha->getCardCount();
    for(sigema--; sigema > -1; sigema--){
        const Card *weapon = Sanguosha->getCard(sigema);
        if(weapon->getSubtype() == "weapon" && !see.contains(sigema)){
            seee << sigema;
        }
    }

    int frog = qrand() % seee.length();
    source->obtainCard(Sanguosha->getCard(seee.at(frog)));
    room->throwCard(this);
}

class Fengli: public TriggerSkill{
public:
    Fengli():TriggerSkill("fengli"){
        events << Damage;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *shura, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();

        if(damage.card && damage.card->inherits("Slash") && damage.to->isAlive()
            && !damage.to->isNude()){
            Room *room = shura->getRoom();
            if(room->askForSkillInvoke(shura, objectName(), data)){
                int to_throw = room->askForCardChosen(shura, damage.to, "he", objectName());
                room->throwCard(to_throw);
            }
        }

        return false;
    }
};
*/
class Dongqi: public TriggerSkill{
public:
    Dongqi():TriggerSkill("dongqi"){
        events << Predamage;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        Room *room = damage.from->getRoom();

        if(player->getMark("ice") == 4){
            QString result = room->askForChoice(damage.from, objectName(), "damage+discard");
            if(result == "damage")
                return false;
        }
        else{
            LogMessage log;
            log.type = "#TriggerSkill";
            log.from = damage.from;
            log.arg = objectName();
            room->sendLog(log);

            JudgeStruct judge;
            judge.pattern = QRegExp("(.*):(heart|spade):(.*)");
            judge.good = true;
            judge.reason = objectName();
            judge.who = damage.from;
            room->judge(judge);

            if(judge.isBad())
                return false;
        }
        if(!damage.to->isAllNude()){
            int card_id = room->askForCardChosen(player, damage.to, "hej", objectName());
            room->throwCard(card_id);

            if(!damage.to->isNude()){
                card_id = room->askForCardChosen(player, damage.to, "hej", objectName());
                room->throwCard(card_id);
            }
            return true;
        }
        return false;
    }
};

class Binggui: public TriggerSkill{
public:
    Binggui():TriggerSkill("binggui"){
        events << Death;
    }

    virtual bool triggerable(const ServerPlayer *player) const{
        return true;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *camus = room->findPlayerBySkillName(objectName());
        if(!camus || camus->getMark("ice") == 4)
            return false;

        if(room->askForSkillInvoke(camus, objectName(), data)){
            room->loseMaxHp(camus);
            camus->setMark("ice", 4);

            LogMessage log;
            log.type = "#Binggui";
            log.from = camus;
            log.arg = "dongqi";
            room->sendLog(log);
        }
        return false;
    }
};

class Shushu: public TriggerSkill{
public:
    Shushu():TriggerSkill("shushu"){
        events << Death;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        DamageStar damage = data.value<DamageStar>();
        if(damage && damage->from){
            Room *room = player->getRoom();
            QList<const Skill *> skills = damage->from->getVisibleSkillList();
            foreach(const Skill *skill, skills){
                if(skill->parent())
                    room->detachSkillFromPlayer(damage->from, skill->objectName());
            }
            room->acquireSkill(damage->from, "dongqi");
        }

        return false;
    }
};

class Meigui:public MasochismSkill{
public:
    Meigui():MasochismSkill("meigui"){
    }

    virtual void onDamaged(ServerPlayer *aphrodite, const DamageStruct &damage) const{
        //Room *room = aphrodite->getRoom();
        if(!aphrodite->isNude() && aphrodite->askForSkillInvoke(objectName())){
            int count = aphrodite->getCardCount(true);
            aphrodite->throwAllCards();
            aphrodite->drawCards(count);
        }
    }
};

class MogOng: public TriggerSkill{
public:
    MogOng():TriggerSkill("mOgOng"){
        frequency = Compulsory;
        events << CardAsked;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target) && !target->getArmor() && target->getMark("qinggang") == 0;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *aphrodite, QVariant &data) const{
        QString pattern = data.toString();

        if(pattern != "jink")
            return false;

        Room *room = aphrodite->getRoom();
        if(aphrodite->askForSkillInvoke(objectName())){
            JudgeStruct judge;
            judge.pattern = QRegExp("(.*):(heart|diamond):(.*)");
            judge.good = true;
            judge.reason = objectName();
            judge.who = aphrodite;

            room->judge(judge);
            CardUseStruct card_use;
            card_use.from = aphrodite;
            if(judge.isGood()){
                Jink *jink = new Jink(Card::NoSuit, 0);
                jink->setSkillName(objectName());
                room->provide(jink);
                room->setEmotion(aphrodite, "good");
            }else
                room->setEmotion(aphrodite, "bad");

            if(judge.card->getSuit() == Card::Diamond){
                Indulgence *indulgence = new Indulgence(judge.card->getSuit(), judge.card->getNumber());
                indulgence->addSubcard(judge.card);
                indulgence->setSkillName(objectName());
                card_use.card = indulgence;
            }
            else if(judge.card->isBlack() &&
                (judge.card->inherits("BasicCard") || judge.card->inherits("EquipCard"))){
                SupplyShortage *shortage = new SupplyShortage(judge.card->getSuit(), judge.card->getNumber());
                shortage->addSubcard(judge.card);
                shortage->setSkillName(objectName());
                card_use.card = shortage;
            }
            else if(judge.card->isRed())
                return true;
            else
                return false;

            QList<ServerPlayer *> targets;
            foreach(ServerPlayer *tmp, room->getAlivePlayers()){
                if(tmp->containsTrick(card_use.card->objectName()))
                    continue;
                if(room->isProhibited(aphrodite, tmp, card_use.card))
                    continue;
                targets << tmp;
            }
            if(!targets.isEmpty()){
                ServerPlayer *target = room->askForPlayerChosen(aphrodite, targets, objectName());
                card_use.to << target;
                room->useCard(card_use);
            }
            if(judge.isGood())
                return true;
        }

        return false;
    }
};


GoldSeintoPackage::GoldSeintoPackage()
    :Package("goldseinto")
{
    General *mu, *aldebaran, *saga, *deathmask,
    *aiolia, *shaka, *dohko, *milo,
    *aiolos, *shura, *camus, *aphrodite;

    mu = new General(this, "mu", "st", 3);
    mu->addSkill("jingqiang");
    mu->addSkill(new Xiufu);
    mu->addSkill(new XiufuE);
    mu->addSkill(new Xingmie);
    mu->addSkill(new MarkAssignSkill("@xm", 1));
    related_skills.insertMulti("xiufu", "#xiufu-effect");
    related_skills.insertMulti("xingmie", "#@xm");

    aldebaran = new General(this, "aldebaran", "st");
    aldebaran->addSkill(new Hao2jiao);

    saga = new General(this, "saga$", "st");
    saga->addSkill(new Huanlong);
    saga->addSkill("ciyuan");
    saga->addSkill(new Yinhe);
    saga->addSkill(new MarkAssignSkill("@yinh", 1));
    saga->addSkill(new Mohua);
    related_skills.insertMulti("yinhe", "#@yinh");

    deathmask = new General(this, "deathmask", "st");
    deathmask->addSkill(new Mingbo);

    aiolia = new General(this, "aiolia", "st");
    aiolia->addSkill(new Dian2guang);
    aiolia->addSkill(new Lizi);

    shaka = new General(this, "shaka", "st");
    //shaka->addSkill(new Xuechi);
    //shaka->addSkill(new SixDao);
    //shaka->addSkill(new SixDaoBuff);
    shaka->addSkill(new Jiangmo);
    shaka->addSkill(new MarkAssignSkill("@lianl", 1));
    skills << new Xuechi << new SixDao << new SixDaoBuff
            << new Bao2lun << new Bao3lun << new Lianluo << new LianluoEft;
    related_skills.insertMulti("sixd", "#sixdaobf");
    related_skills.insertMulti("lianluo", "#lianluo_effect");
    related_skills.insertMulti("lianluo", "#@lianl");

    dohko = new General(this, "dohko", "st");
    dohko->addSkill(new Shengqi);
    dohko->addSkill(new Bailong);
    dohko->addSkill(new Longxiang);

    milo = new General(this, "milo", "st");
    milo->addSkill(new Duzhen);

    aiolos = new General(this, "aiolos", "st", 3);
    aiolos->addSkill("jingong");
    aiolos->addSkill(new Renma);
    aiolos->addSkill(new Shesha);

    shura = new General(this, "shura", "st");
    shura->addSkill(new Sheng2jian);
    //shura->addSkill(new Fengli);

    camus = new General(this, "camus", "st");
    camus->addSkill(new Dongqi);
    camus->addSkill(new Binggui);
    camus->addSkill(new Shushu);

    aphrodite = new General(this, "aphrodite", "st", 3);
    aphrodite->addSkill(new Meigui);
    aphrodite->addSkill(new MogOng);

    addMetaObject<XiufuCard>();
    addMetaObject<HuanlongCard>();
    addMetaObject<YinheCard>();
    addMetaObject<MohuaCard>();
    addMetaObject<LianluoCard>();
    addMetaObject<Sheng2jianCard>();

    patterns[".LZ"] = new LiziPattern;
}

ADD_PACKAGE(GoldSeinto)
