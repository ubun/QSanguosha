#include "standard.h"
#include "skill.h"
#include "gold-seinto.h"
#include "client.h"
#include "carditem.h"
#include "engine.h"
#include "maneuvering.h"
#include "ai.h"

class Xiufu: public OneCardViewAsSkill{
public:
    Xiufu():OneCardViewAsSkill("xiufu"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("XiufuCard");
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
    RecoverStruct recover;
    recover.card = this;
    recover.who = source;
    room->recover(source, recover);
}

class Xingmie: public TriggerSkill{
public:
    Xingmie():TriggerSkill("xingmie"){
        events << Damage;
        frequency = Limited;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        if(player->getMark("xm") > 0)
            return false;
        Room *room = player->getRoom();
        DamageStruct damage = data.value<DamageStruct>();
        if(player->askForSkillInvoke(objectName(), data) &&
           room->askForCard(player, ".S", "xingmie-ask")){
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
            player->addMark("xm");
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
            CardStar card = player->tag.value("hj").value<CardStar>();
            if(card->getSuit() == Card::Diamond)
                damage.damage ++;
            else if(card->getSuit() == Card::Heart)
                damage.from->obtainCard(card);
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

            if(judge.isGood()){
                effect.from->tag["hj"] = QVariant::fromValue(judge.card);
                room->slashResult(effect, NULL);
                return true;
            }
        }
        return false;
    }
};

Huan2longCard::Huan2longCard(){
    will_throw = false;
    once = true;
}

bool Huan2longCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select != Self && !to_select->isKongcheng();
}

void Huan2longCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    ServerPlayer *sunce = targets.first();
    source->pindian(sunce, "huan2long", this);
}

class Huan2longPindian: public OneCardViewAsSkill{
public:
    Huan2longPindian():OneCardViewAsSkill("huan2long_pindian"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->isKongcheng() && ! player->hasUsed("Huan2longCard");
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return ! to_select->isEquipped();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        Huan2longCard *card = new Huan2longCard;
        card->addSubcard(card_item->getFilteredCard());

        return card;
    }
};

class Huan2long: public TriggerSkill{
public:
    Huan2long():TriggerSkill("huan2long"){
        view_as_skill = new Huan2longPindian;
        events << Pindian;
    }

    virtual int getPriority() const{
        return -1;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        PindianStar pindian = data.value<PindianStar>();
        Room *room = player->getRoom();
        if(pindian->reason == "huan2long" && pindian->from == player){
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
    if(p->getMark("yinhe") > 0)
        return;
    room->broadcastInvoke("animate", "lightbox:$yinhe");
    p->loseMark("@star", 3);

    Card::use(room, p, targets);
}

void YinheCard::onEffect(const CardEffectStruct &effect) const{
    effect.from->getRoom()->loseMaxHp(effect.to);
    effect.from->addMark("yinhe");
}

class Yinhe: public ZeroCardViewAsSkill{
public:
    Yinhe():ZeroCardViewAsSkill("yinhe"){
        frequency = Limited;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getMark("yinhe") == 0 && player->getMark("@star") > 2;
    }

    virtual const Card *viewAs() const{
        return new YinheCard;
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
        log.type = "#Dian2guang";
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
            log.type = "#Lizi";
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
            judge.pattern = QRegExp("(.*):(heart|diamond):([1-6])");
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
            return false;
        }
        if(player->getPhase() == Player::Start && player->isKongcheng() && player->askForSkillInvoke(objectName(), data)){
            room->detachSkillFromPlayer(player, "xuechi");
            room->detachSkillFromPlayer(player, "sixd");

            room->acquireSkill(player, "bao2lun");
            room->acquireSkill(player, "yiji");
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
        if(player->askForSkillInvoke(objectName())){
            JudgeStruct judge;
            judge.good = true;
            judge.reason = objectName();
            judge.who = damage.from;
            room->judge(judge);

            damage.from->tag["feng"] = QVariant::fromValue(judge.card);
            damage.from->addToPile("feng", judge.card->getId());
        }
    }
};
/*
class Baolun:public ZeroCardViewAsSkill{
public:
    Baolun():ZeroCardViewAsSkill("baolun"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getMark("liudao") == 0;
    }

    virtual const Card *viewAs() const{
        return new BaolunCard;
    }
};

BaolunCard::BaolunCard(){
}

bool BaolunCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && !to_select->isKongcheng()
            && Self->getPile("guo").length() >= to_select->getHandcardNum();
}

void BaolunCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    ServerPlayer *target = targets.first();
    int handlog = target->getHandcardNum();
    target->throwAllHandCards();

    QList<int> guoguo = source->getPile("guo");
    room->fillAG(guoguo, source);
    while(!guoguo.isEmpty()){
        if(handlog <= 0)
            break;
        int card_id = room->askForAG(source, guoguo, true, "guo");
        if(card_id == -1)
            break;
        guoguo.removeOne(card_id);
        handlog --;
        room->moveCardTo(Sanguosha->getCard(card_id), target, Player::Hand, false);
    }
    source->invoke("clearAG");
}

class Longba: public TriggerSkill{
public:
    Longba():TriggerSkill("longba"){
        events << Damage;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *libra, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();

        if(damage.card && damage.card->inherits("Slash") && damage.to->isAlive()
            && damage.card->isRed()
            && !damage.to->isAllNude()){
            Room *room = libra->getRoom();
            if(room->askForSkillInvoke(libra, objectName())){
                int to_throw = room->askForCardChosen(libra, damage.to, "hej", objectName());
                room->throwCard(to_throw);
            }
        }
        return false;
    }
};

class Longfei: public PhaseChangeSkill{
public:
    Longfei():PhaseChangeSkill("longfei"){
        frequency = Wake;
        default_choice = "draw";
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target)
                && target->getPhase() == Player::Start
                && target->getMark("longfei") == 0
                && target->getEquips().length() > 2;
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        Room *room = player->getRoom();

        LogMessage log;
        log.type = "#LongfeiWake";
        log.from = player;
        room->sendLog(log);

        QString result = room->askForChoice(player ,objectName(), "recover+draw");
        if(result == "recover"){
            RecoverStruct recover;
            recover.card = NULL;
            recover.who = NULL;
            room->recover(player, recover);
        }
        else if(result == "draw")
            player->drawCards(2);

        player->setMark("longfei", 1);

        room->loseMaxHp(player);
        room->acquireSkill(player, "longdan");
        room->acquireSkill(player, "mashu");

        return false;
    }
};

class Zhiyan:public ZeroCardViewAsSkill{
public:
    Zhiyan():ZeroCardViewAsSkill("zhiyan"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->isWounded();
    }

    virtual const Card *viewAs() const{
        return new ZhiyanCard;
    }
};

ZhiyanCard::ZhiyanCard(){
}

bool ZhiyanCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty();
}

void ZhiyanCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    ServerPlayer *target = targets.first();
    room->loseMaxHp(source);
    DamageStruct damage;
    damage.nature = DamageStruct::Fire;
    damage.from = source;
    damage.to = target;
    room->damage(damage);
}

class Jingong: public TriggerSkill{
public:
    Jingong():TriggerSkill("jingong"){
        events << GameStart << CardLost << HpChanged;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        if(event == GameStart)
            room->setPlayerProperty(player, "atk", player->getAttackRange() + 4);
        else if(event == CardLost){
            CardMoveStar move = data.value<CardMoveStar>();
            if(move->from_place == Player::Equip){
                const Card *card = Sanguosha->getCard(move->card_id);
                const EquipCard *equip = qobject_cast<const EquipCard *>(card);
                if(equip && equip->location() == EquipCard::WeaponLocation)
                    room->setPlayerProperty(player, "atk", player->getAttackRange() + 4);
            }
        }
        return false;
    }
};

class Shanyao: public PhaseChangeSkill{
public:
    Shanyao():PhaseChangeSkill("shanyao"){
        frequency = Wake;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target)
                && target->getPhase() == Player::Start
                && target->getMark("shanyao") == 0
                && target->getHp() == 1;
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        Room *room = player->getRoom();

        LogMessage log;
        log.type = "#ShanyaoWake";
        log.from = player;
        room->sendLog(log);

        player->setMark("shanyao", 1);

        room->loseMaxHp(player);
        room->acquireSkill(player, "liegong");
        room->acquireSkill(player, "buqu");

        return false;
    }
};

class Shengjian: public OneCardViewAsSkill{
public:
    Shengjian():OneCardViewAsSkill("shengjian"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->isKongcheng() && !player->hasUsed("ShengjianCard");
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped() && to_select->getCard()->inherits("TrickCard");
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        ShengjianCard *card = new ShengjianCard;
        card->addSubcard(card_item->getCard()->getId());

        return card;
    }
};

ShengjianCard::ShengjianCard(){
    target_fixed = true;
    once = true;
}

void ShengjianCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    int sigema = Sanguosha->getCardCount();
    QStringList weapons;
    switch(Sanguosha->getCard(this->getSubcards().first())->getSuit()){
        case Spade: weapons << "blade" << "spear"; break;
        case Heart: weapons << "kylin_bow" << "ice_sword"; break;
        case Club: weapons << "qinggang_sword" << "halberd"; break;
        case Diamond: weapons << "axe" << "double_sword"; break;
        default: return;
    }
    foreach(QString w, weapons){
        if(source->getMark(w) > 0)
            weapons.removeOne(w);
    }
    source->addMark("stswd");
    if(weapons.length() < 1)
        return;
    QString wpch = room->askForChoice(source, "shengjian", weapons.join("+"));
    for(sigema--; sigema > -1; sigema--){
        if(Sanguosha->getCard(sigema)->objectName() == wpch){
            room->obtainCard(source, sigema);
            break;
        }
    }
    room->throwCard(this);
    if(source->getWeapon())
        room->throwCard(source->getWeapon());
    room->moveCardTo(Sanguosha->getCard(sigema), source, Player::Equip, true);

    LogMessage log;
    log.type = "$Shengjian";
    log.from = source;
    log.card_str = Sanguosha->getCard(sigema)->toString();
    room->sendLog(log);

    source->addMark(wpch);
}

class Duanbi: public PhaseChangeSkill{
public:
    Duanbi():PhaseChangeSkill("duanbi"){
        frequency = Wake;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target)
                && target->getPhase() == Player::Judge
                && target->getMark("duanbi") == 0
                && target->getMark("stswd") >= 8;
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        Room *room = player->getRoom();

        LogMessage log;
        log.type = "#DuanbiWake";
        log.from = player;
        room->sendLog(log);

        player->setMark("duanbi", 1);

        player->throwAllCards();
        room->acquireSkill(player, "paoxiao");

        return false;
    }
};

class Duanbipre: public PhaseChangeSkill{
public:
    Duanbipre():PhaseChangeSkill("#duanbip"){
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getPhase() == Player::Finish && target->getMark("stswd") == 7;
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        Room *room = player->getRoom();

        LogMessage log;
        log.type = "#DuanbiHide";
        log.from = player;
        room->sendLog(log);

        room->acquireSkill(player, "duanbi");

        return false;
    }
};

class Jiguang: public TriggerSkill{
public:
    Jiguang():TriggerSkill("jiguang"){
        events << Damage << Damaged;
        frequency = Frequent;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        Room *room = player->getRoom();

        ServerPlayer *target = player == damage.from ? damage.to : damage.from;
        if(target->isAlive() && !target->isKongcheng() && player->askForSkillInvoke(objectName())){
            for(int i = damage.damage; i > 0; i--){
                if(target && !target->isKongcheng()){
                    int card_id = room->askForCardChosen(player, target, "h", "jiguang");
                    target->addToPile("gas", card_id);
                }
            }
        }
        return false;
    }
};

class Wangqi: public PhaseChangeSkill{
public:
    Wangqi():PhaseChangeSkill("wangqi"){
        frequency = Frequent;
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        Room *room = player->getRoom();

        if(player->getPhase() == Player::Judge && player->askForSkillInvoke(objectName())){
            JudgeStruct judge;
            judge.pattern = QRegExp("(.*):(spade):(.*)");
            judge.good = false;
            judge.reason = "wangqi";
            judge.who = player;

            room->judge(judge);

            int count = 0;
            foreach(ServerPlayer *p, room->getAllPlayers()){
                foreach(int i, p->getPile("gas")){
                    room->moveCardTo(Sanguosha->getCard(i), judge.isGood() ? player : p, Player::Hand);
                    count++;
                }
            }
            if(judge.isGood()){
                QList<int> yiji_cards = player->handCards().mid(player->getHandcardNum() - count);
                while(room->askForYiji(player, yiji_cards));
            }
        }
        return false;
    }
};

class Mogong: public TriggerSkill{
public:
    Mogong():TriggerSkill("mogong"){
        events << CardUsed;
    }

    virtual bool triggerable(const ServerPlayer *player) const{
        return !player->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        CardUseStruct use = data.value<CardUseStruct>();
        if(!use.card->inherits("Slash"))
            return false;
        Room *room = player->getRoom();
        ServerPlayer *pisces = room->findPlayerBySkillName(objectName());
        if(use.to.length() == 1 && use.to.contains(pisces))
            return false;
        if(room->askForCard(pisces, ".C", "mogongshow", false))
            use.to.clear();
            use.to << pisces;
            room->useCard(use);
            return true;
        }

        return false;
    }
};

class Jingji:public MasochismSkill{
public:
    Jingji():MasochismSkill("jingji"){
        default_choice = "draw";
    }

    virtual void onDamaged(ServerPlayer *pisces, const DamageStruct &damage) const{
        Room *room = pisces->getRoom();

        QString choice = room->askForChoice(pisces, objectName(), "draw+throw+cancel");
        if(choice == "draw"){
            int x = damage.damage, i;
            for(i=0; i<x; i++)
                pisces->drawCards(2);
        }
        else if(choice == "throw" && !damage.from->isNude()){
            room->askForDiscard(damage.from, objectName(), qMin(2, damage.from->getCardCount(true)), false, true);
        }
    }
};
*/

GoldSeintoPackage::GoldSeintoPackage()
    :Package("goldseinto")
{
    General *mu, *aldebaran, *saga, *deathmask,
    *aiolia, *shaka, *dohko, *milo,
    *aiolos, *shura, *camus, *aphrodite;

    mu = new General(this, "mu$", "st", 3);
    mu->addSkill("jingqiang");
    mu->addSkill(new Xiufu);
    mu->addSkill(new Xingmie);

    aldebaran = new General(this, "aldebaran", "st");
    aldebaran->addSkill(new Hao2jiao);

    saga = new General(this, "saga", "st");
    saga->addSkill(new Huan2long);
    saga->addSkill("ciyuan");
    saga->addSkill(new Yinhe);

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
    skills << new Xuechi << new SixDao << new SixDaoBuff;
    related_skills.insertMulti("sixd", "#sixdaobf");

    dohko = new General(this, "dohko", "st");
    milo = new General(this, "milo", "st");
    aiolos = new General(this, "aiolos", "st", 3);
    shura = new General(this, "shura", "st");
    camus = new General(this, "camus", "st");
    aphrodite = new General(this, "aphrodite", "st", 3);

    addMetaObject<XiufuCard>();
    addMetaObject<Huan2longCard>();
    addMetaObject<YinheCard>();

    patterns[".LZ"] = new LiziPattern;
}

ADD_PACKAGE(GoldSeinto)
