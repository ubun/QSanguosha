#include "standard.h"
#include "skill.h"
#include "gold-seinto-viva.h"
#include "client.h"
#include "carditem.h"
#include "engine.h"
#include "maneuvering.h"
#include "ai.h"

class XiufuViewAsSkill: public OneCardViewAsSkill{
public:
    XiufuViewAsSkill():OneCardViewAsSkill("xiufu"){
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

class Xiufu: public TriggerSkill{
public:
    Xiufu():TriggerSkill("xiufu"){
        events << Dying;
        view_as_skill = new XiufuViewAsSkill;
    }

    virtual int getPriority() const{
        return 2;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *mu = room->findPlayerBySkillName("xiufu");
        DyingStruct dying = data.value<DyingStruct>();
        if(!mu || dying.who != player || dying.who->tag.value("Dead").toBool())
            return false;
        if(!mu->askForSkillInvoke(objectName(), data))
            return false;
        const Card *cardt = room->askForCard(mu, ".H", "@xiufu", data);
        if(cardt){
            LogMessage log;
            log.type = "#Xiufu";
            log.from = mu;
            log.to << dying.who;
            log.arg = objectName();
            log.arg2 = QString::number(1);
            room->sendLog(log);
            room->setPlayerProperty(dying.who, "hp", 1);
            room->setEmotion(dying.who, "good");
        }
        dying.who->tag["Dead"] = false;

        return false;
    }
};

class Jingqiang: public TriggerSkill{
public:
    Jingqiang(): TriggerSkill("jingqiang"){
        events << PhaseChange;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *target = NULL;
        if(player->getPhase() == Player::Start){
            foreach(target, room->getAlivePlayers()){
                if(target->getMark("@jq") > 0){
                    QList<QVariant> ownerlist = target->tag.value("JQOwner").toList();
                    if(ownerlist.contains(player->objectName())){
                        target->loseMark("@jq", 1);
                        ownerlist.removeOne(player->objectName());
                        target->tag["JQOwner"] = ownerlist;
                        foreach(ServerPlayer *p, room->getOtherPlayers(target)){
                            room->setFixedDistance(p, target, -1);
                        }
                    }
                }
            }
        }
        else if(player->getPhase() == Player::Finish){
            if(!room->askForSkillInvoke(player, objectName(), data))
                return false;

            target = room->askForPlayerChosen(player, room->getAlivePlayers(), objectName());
            foreach(ServerPlayer *p, room->getOtherPlayers(target)){
                room->setFixedDistance(p, target, target->distanceTo(p)+1);
            }
            target->gainMark("@jq");
            QList<QVariant> ownerlist = target->tag.value("JQOwner").toList();
            ownerlist << player->objectName();
            target->tag["JQOwner"] = ownerlist;
        }

        return false;
    }
};

class JingqiangClear: public TriggerSkill{
public:
    JingqiangClear():TriggerSkill("#jingqiang-clear"){
        events << Death;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &) const{
        Room *room = player->getRoom();
        foreach(ServerPlayer *target, room->getAllPlayers()){
            if(target->getMark("@jq") > 0){
                QList<QVariant> ownerlist = target->tag.value("JQOwner").toList();
                if(ownerlist.contains(player->objectName())){
                    target->loseMark("@jq", 1);
                    ownerlist.removeOne(player->objectName());
                    target->tag["JQOwner"] = ownerlist;
                    foreach(ServerPlayer *tmp, room->getOtherPlayers(target)){
                        room->setFixedDistance(tmp, target, -1);
                    }
                }
            }
        }
        return false;
    }
};

class Xingmie: public ViewAsSkill{
public:
    Xingmie():ViewAsSkill("xingmie"){
        frequency = Limited;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getMark("@xm") >= 1;
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(selected.length() >= 3)
            return false;

        return true;
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() != 3)
            return NULL;

        XingmieCard *card = new XingmieCard;
        card->addSubcards(cards);

        return card;
    }
};

XingmieCard::XingmieCard(){
    target_fixed = true;
}
void XingmieCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    room->throwCard(this);
    ServerPlayer *target = source;
    foreach(ServerPlayer *tmp, room->getAllPlayers()){
        if(target->getHandcardNum() > tmp->getHandcardNum())
            target = tmp;
    }
    QList<ServerPlayer *> targets;
    foreach(ServerPlayer *tmp, room->getAllPlayers()){
        if(tmp->getHandcardNum() == target->getHandcardNum())
            targets << tmp;
    }
    if(!targets.isEmpty()){
        target = room->askForPlayerChosen(source, targets, "xingmie");
        source->loseMark("@xm");
        room->broadcastInvoke("animate", "lightbox:$xingmie");
        DamageStruct damage;
        damage.from = source;
        damage.to = target;
        damage.damage = target->getHandcardNum();
        target->tag["Dead"] = true;
        room->damage(damage);
    }
}

HaojiaoCard::HaojiaoCard(){
    once = true;
}

bool HaojiaoCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(targets.length() < 2)
        return Self->canSlash(to_select);
    else
        return false;
}

void HaojiaoCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    Slash *slash = new Slash(Card::NoSuit, 0);
    slash->setSkillName("haojiao");
    CardUseStruct use;
    use.card = slash;
    use.from = source;
    use.to = targets;
    room->useCard(use, false);
}

class HaojiaoViewAsSkill: public ZeroCardViewAsSkill{
public:
    HaojiaoViewAsSkill(): ZeroCardViewAsSkill("haojiao"){

    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "@@haojiao";
    }

    virtual const Card *viewAs() const{
        HaojiaoCard *card = new HaojiaoCard;
        card->setSkillName("haojiao");
        return card;
    }
};

class Haojiao: public TriggerSkill{
public:
    Haojiao(): TriggerSkill("haojiao"){
        events << PhaseChange << Damaged;
        view_as_skill = new HaojiaoViewAsSkill;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();

        if(event == PhaseChange){
            if(player->getPhase() != Player::Finish)
                return false;

            if(room->askForUseCard(player, "@@haojiao", "@haojiao"))
                player->turnOver();
        }
        else if(event == Damaged){
            if(player->faceUp() || !room->askForSkillInvoke(player, objectName(), data))
                return false;

            player->turnOver();
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
    ServerPlayer *target = targets.first();
    source->pindian(target, "huanlong", this);
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
        if(pindian->reason == "huanlong" && pindian->from == player){
            ServerPlayer *winner = pindian->isSuccess() ? pindian->from : pindian->to;
            winner->obtainCard(pindian->to_card);
            winner->obtainCard(pindian->from_card);
            if(winner == player && !pindian->to->isKongcheng()){
                LogMessage log;
                log.from = player;
                log.to << pindian->to;
                log.type = "#Huanlong";
                player->getRoom()->sendLog(log);

                player->getRoom()->showAllCards(pindian->to, player);
            }
        }

        return false;
    }
};

class Kongjian:public MasochismSkill{
public:
    Kongjian():MasochismSkill("kongjian"){
        frequency = Frequent;
    }

    virtual void onDamaged(ServerPlayer *baby, const DamageStruct &damage) const{
        if(baby->getMark("xingbao") > 0)
            return;
        if(!damage.from || damage.to == damage.from || !baby->askForSkillInvoke(objectName()))
            return;
        Room *room = baby->getRoom();

        Card::Suit suit = room->askForSuit(baby);
        QString suit_str = Card::Suit2String(suit);
        QString pattern = QString(".%1").arg(suit_str.at(0).toUpper());

        LogMessage log;
        log.type = "#Kongjian";
        log.from = baby;
        log.to << damage.from;
        log.arg = suit_str;
        room->sendLog(log);
        const Card *card = room->askForCard(damage.from, pattern, "@kongjian:" + suit_str, NULL, false);
        if(card){
            baby->addToPile("star", card->getId());
        }
        else{
            room->setEmotion(damage.from, "bad");
            DamageStruct damage2;
            damage2.from = NULL;
            damage2.to = damage.from;
            room->damage(damage2);
        }
    }
};

class Xingbao: public PhaseChangeSkill{
public:
    Xingbao(): PhaseChangeSkill("xingbao"){
        frequency = Wake;
    }
    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target)
                && target->getMark("xingbao") == 0
                && target->getPhase() == Player::Start
                && target->getPile("star").length() >= 3;
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        Room *room = player->getRoom();

        LogMessage log;
        log.type = "#XingbaoWake";
        log.from = player;
        room->sendLog(log);

        room->setPlayerMark(player, "xingbao", 1);
        room->setPlayerProperty(player, "maxhp", player->getMaxHP() + 1);
        RecoverStruct recover;
        recover.who = player;
        room->recover(player, recover);

        room->detachSkillFromPlayer(player, "kongjian");
        foreach(int i, player->getPile("star")){
            room->obtainCard(player, i);
        }
        room->setEmotion(player, "draw-card");
        return false;
    }
};

ShiqiCard::ShiqiCard(){
    once = true;
}

bool ShiqiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty();
}

void ShiqiCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    room->throwCard(this);

    QString choice = room->askForChoice(source, "shiqi", "start+judge+draw+discard+finish");
    QVariant data = QVariant::fromValue(choice);
    room->setTag(targets.first()->objectName(), data);

    targets.first()->gainMark("@shiqi");
}

class Shiqi: public ViewAsSkill{
public:
    Shiqi(): ViewAsSkill("shiqi"){

    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(selected.length() < 2)
            return true;
        else
            return false;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("ShiqiCard");
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        ShiqiCard *card = new ShiqiCard;
        card->addSubcards(cards);
        card->setSkillName(objectName());
        return card;
    }
};

class ShiqiEffect: public TriggerSkill{
public:
    ShiqiEffect(): TriggerSkill("#shiqi-effect"){
        events << PhaseChange;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        if(player->getPhase() != Player::NotActive)
            return false;

        Room*room = player->getRoom();
        ServerPlayer *target = NULL;
        foreach(ServerPlayer *tmp, room->getAllPlayers()){
            if(tmp->getMark("@shiqi") > 0){
                target = tmp;
                break;
            }
        }

        if(target){
            target->loseAllMarks("@shiqi");
            QString choice = room->getTag(target->objectName()).toString();
            room->removeTag(target->objectName());

            LogMessage log;
            log.type = "#ShiqiEffect";
            log.from = player;
            log.to << target;
            log.arg = "shiqi";
            log.arg2 = choice;
            room->sendLog(log);

            QList<Player::Phase> phases;
            if(choice == "start")
                phases << Player::Start;
            else if(choice == "judge")
                phases << Player::Judge;
            else if(choice == "draw")
                phases << Player::Draw;
            else if(choice == "discard")
                phases << Player::Discard;
            else if(choice == "finish")
                phases << Player::Finish;

            if(!phases.isEmpty()){
                if(phases.contains(Player::Discard))
                    room->setEmotion(target, "bad");
                target->play(phases);
            }
        }

        return false;
    }
};

class Leiguang: public OneCardViewAsSkill{
public:
    Leiguang():OneCardViewAsSkill("leiguang"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return Slash::IsAvailable(player);
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "slash";
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getCard()->isBlack();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *first = card_item->getCard();
        ThunderSlash *thunderslash = new ThunderSlash(first->getSuit(), first->getNumber());
        thunderslash->addSubcard(first->getId());
        thunderslash->setSkillName(objectName());
        return thunderslash;
    }
};

class Budong: public TriggerSkill{
public:
    Budong():TriggerSkill("budong"){
        events << CardLost;
        frequency = Frequent;
    }

    virtual int getPriority() const{
        return -1;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target) && target->getPhase() == Player::NotActive;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *virgo, QVariant &data) const{
        CardMoveStar move = data.value<CardMoveStar>();
        Room *room = virgo->getRoom();
        if(move->from_place == Player::Hand || move->from_place == Player::Equip){
            if(!virgo->askForSkillInvoke(objectName(), data))
                return false;
            JudgeStruct judge;
            judge.pattern = QRegExp("(.*):(spade):(.*)");
            judge.good = false;
            judge.reason = objectName();
            judge.who = virgo;

            room->judge(judge);
            if(judge.isGood()){
                LogMessage log;
                log.type = "#Budong";
                log.from = virgo;
                log.arg = "guo";
                virgo->addToPile("guo", judge.card->getId());
                log.arg2 = QString::number(virgo->getPile("guo").length());
                room->sendLog(log);

                room->setEmotion(virgo, "good");
            }
            else
                room->setEmotion(virgo, "bad");
        }

        return false;
    }
};

class Liudao: public PhaseChangeSkill{
public:
    Liudao():PhaseChangeSkill("liudao"){
        frequency = Wake;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target)
                && target->getMark("liudao") == 0
                && target->getPhase() == Player::Start;
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        Room *room = player->getRoom();
        if(player->getPile("guo").length() < 6)
            return false;
        LogMessage log;
        log.type = "#LiudaoWake";
        log.from = player;
        room->sendLog(log);

        player->setMark("liudao", 1);
        room->loseMaxHp(player);

        return false;
    }
};

class LiudaoWaked: public PhaseChangeSkill{
public:
    LiudaoWaked():PhaseChangeSkill("#liudao_waked"){
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getMark("liudao") > 0
                && target->getPhase() == Player::Draw
                && !target->getPile("guo").isEmpty();
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        Room *room = player->getRoom();

        QList<int> guoguo = player->getPile("guo");
        room->fillAG(guoguo, player);
        int card_id = room->askForAG(player, guoguo, true, "guo");
        if(card_id != -1){
            guoguo.removeOne(card_id);
            room->moveCardTo(Sanguosha->getCard(card_id), player, Player::Hand, false);
        }
        player->invoke("clearAG");
        guoguo.clear();
        return false;
    }
};

BaolunCard::BaolunCard(){
}

bool BaolunCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && !to_select->isKongcheng()
            && Self->getPile("guo").length() >= to_select->getHandcardNum();
}

void BaolunCard::onUse(Room *room, const CardUseStruct &card_use) const{
    if(card_use.from->getMark("liudao") > 0)
        SkillCard::onUse(room, card_use);
}

void BaolunCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    ServerPlayer *target = targets.first();
    int handlog = target->getHandcardNum();
    target->throwAllHandCards();
    QList<int> guoguo = source->getPile("guo");
    if(guoguo.length() == handlog){
        for(int i = 0; i < handlog; i++)
            room->moveCardTo(Sanguosha->getCard(guoguo.at(i)), target, Player::Hand, false);
        room->setEmotion(target, "draw-card");
        return;
    }

    room->fillAG(guoguo);
    while(!guoguo.isEmpty()){
        if(handlog <= 0)
            break;
        int card_id = room->askForAG(source, guoguo, false, "guo");
        guoguo.removeOne(card_id);
        handlog --;
        room->moveCardTo(Sanguosha->getCard(card_id), target, Player::Hand, false);
        room->takeAG(target, card_id);
        room->setEmotion(target, "draw-card");
    }
    room->broadcastInvoke("clearAG");
}

class BaolunDistance: public DistanceSkill{
public:
    BaolunDistance():DistanceSkill("#baolun_distance"){
    }

    virtual int getCorrect(const Player *from, const Player *to) const{
        int p2p = from->getHp() - to->getHp();
        if(to->getMark("liudao") == 0 && to->hasSkill("baolun")
            && p2p > 0)
            return p2p;
        else
            return 0;
    }
};

class Baolun:public ZeroCardViewAsSkill{
public:
    Baolun():ZeroCardViewAsSkill("baolun"){
    }

    virtual const Card *viewAs() const{
        return new BaolunCard;
    }
};

class Longba: public TriggerSkill{
public:
    Longba():TriggerSkill("longba"){
        events << Damage;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *libra, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();

        if(damage.card && damage.card->inherits("Slash") && damage.to->isAlive()
            && !damage.to->isAllNude()){
            Room *room = libra->getRoom();
            if(room->askForSkillInvoke(libra, objectName(), data)){
                int to_throw = room->askForCardChosen(libra, damage.to, "he", objectName());
                LogMessage log;
                log.from = damage.to;
                log.type = "$Longba";
                log.card_str = QString::number(to_throw);
                room->sendLog(log);

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
                && target->getPhase() == Player::NotActive
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
        room->acquireSkill(player, "zhijian");

        return false;
    }
};

class HongzhenViewAsSkill:public ZeroCardViewAsSkill{
public:
    HongzhenViewAsSkill():ZeroCardViewAsSkill("hongzhen"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        QList<int> needle = player->getPile("needle");
        if(needle.length() < 2)
            return false;
        for(int i = 0; i < needle.length(); i++){
            for(int j = 1; j < needle.length(); j++){
                const Card *card1 = Sanguosha->getCard(needle.at(i));
                const Card *card2 = Sanguosha->getCard(needle.at(j));
                if(card1->getNumber() == card2->getNumber())
                    return true;
            }
        }
        return false;
    }

    virtual const Card *viewAs() const{
        return new HongzhenCard;
    }
};

HongzhenCard::HongzhenCard(){
    target_fixed = true;
}

void HongzhenCard::onUse(Room *room, const CardUseStruct &card_use) const{
    ServerPlayer *player = card_use.from;

    QList<int> card_ids = player->getPile("needle");
    if(card_ids.isEmpty())
        return;

    int card_id;
    //remove other
    QList<int> numlist,numenable;
    for(int i = 1; i < 14; i++)
        numlist << i;  //1~13点数集合
    foreach(int i, card_ids){
        const Card *c = Sanguosha->getCard(i);
        if(numlist.contains(c->getNumber())){
            numlist.removeOne(c->getNumber());
            //遍历1~13，如果集合里有，则去掉，如果去掉一次后再没有了，将此点数记下
        }
        else
            numenable << c->getNumber();  //存储满足条件的点数
    }
    QMutableListIterator<int> itor(card_ids);
    while(itor.hasNext()){
        const Card *c = Sanguosha->getCard(itor.next());
        if(!numenable.contains(c->getNumber())){
            itor.remove(); //如果没有满足条件则去掉
            //room->takeAG(NULL, c->getId());
        }
    }
    if(card_ids.isEmpty())
        return;
    room->fillAG(card_ids, player);

    card_id = room->askForAG(player, card_ids, true, "hongzhen");
    player->invoke("clearAG");

    if(card_id == -1)
        return;

    const Card *card = Sanguosha->getCard(card_id);
    Snatch *snatch = new Snatch(card->getSuit(), card->getNumber());
    snatch->setSkillName("hongzhen");

    QList<ServerPlayer *> targets;
    QList<const Player *> empty_list;
    foreach(ServerPlayer *p, room->getAlivePlayers()){
        if(!snatch->targetFilter(empty_list, p, player))
            continue;
        if(player->isProhibited(p, snatch))
            continue;
        targets << p;
    }
    if(targets.isEmpty())
        return;

    ServerPlayer *target = room->askForPlayerChosen(player, targets, "hongzhen");
    room->throwCard(card_id);

    CardUseStruct use;
    use.card = snatch;
    use.from = player;
    use.to << target;

    room->useCard(use);
}

class HongzhenEffect: public TriggerSkill{
public:
    HongzhenEffect():TriggerSkill("hongzhen_effect"){
        events << CardFinished << CardResponsed;
        view_as_skill = new HongzhenViewAsSkill;
    }

    virtual int getPriority() const{
        return -1;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        CardStar card = NULL;
        if(event == CardFinished){
            CardUseStruct card_use = data.value<CardUseStruct>();
            card = card_use.card;
        }else if(event == CardResponsed)
            card = data.value<CardStar>();

        if((card->isRed() && card->inherits("BasicCard")) || card->isNDTrick()){
            if(!player->askForSkillInvoke("hongzhen"))
                return false;
            int card_id = card->getSubcards().isEmpty()? card->getId(): card->getSubcards().first();
            if(card_id > -1){
                LogMessage log;
                log.from = player;
                log.type = "#Hongzhen";
                log.arg = "needle";
                player->addToPile(log.arg, card_id);
                log.arg2 = QString::number(player->getPile(log.arg).length());
                player->getRoom()->sendLog(log);
            }

            if(player->getPile("needle").length() >= 15){
                Room *room = player->getRoom();
                if(room->askForChoice(player, "hongzhen15", "shoot+later") == "later")
                    return false;
                LogMessage log;
                log.from = player;
                log.type = "#Hongzhen15";
                log.arg = "hongzhen15";

                QList<int> needle = player->getPile("needle");
                DummyCard *dummy = new DummyCard;
                ServerPlayer *target = room->askForPlayerChosen(player, room->getOtherPlayers(player), "hongzhen15");
                room->setEmotion(target, "bad");
                for(int i = needle.length(); i > 0 ;i--){
                    if(!player->getPile("needle").isEmpty()){
                        int card_id = player->getPile("needle").first();
                        room->moveCardTo(Sanguosha->getCard(card_id), target, Player::Special, false);
                        room->getThread()->delay(150);
                        dummy->addSubcard(card_id);
                    }
                }
                log.to << target;
                log.arg2 = QString::number(needle.length());
                room->sendLog(log);
                DamageStruct damage;
                damage.from = player;
                damage.to = target;
                damage.damage = int(needle.length() / 5);
                room->damage(damage);

                room->throwCard(dummy);
                delete dummy;
            }
        }
        return false;
    }
};

class Hongzhen: public GameStartSkill{
public:
    Hongzhen():GameStartSkill("hongzhen"){
        frequency = Frequent;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return GameStartSkill::triggerable(target) && target->hasSkill("hongzhen");
    }

    virtual void onGameStart(ServerPlayer *player) const{
        player->getRoom()->acquireSkill(player, "hongzhen_effect");
    }
};

class Yuanzi: public PhaseChangeSkill{
public:
    Yuanzi():PhaseChangeSkill("yuanzi"){
        frequency = Compulsory;
    }

    virtual int getPriority() const{
        return 2;
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        if(player->getPhase() != Player::Start)
            return false;
        Room *room = player->getRoom();
        switch(player->getMark("yuanzi")){
        case 1 : room->detachSkillFromPlayer(player, "yingzi"); break;
        case 2 : room->detachSkillFromPlayer(player, "yinghun"); break;
        case 3 : room->detachSkillFromPlayer(player, "leiguang"); break;
        default: break;
        }
        int atk = player->getAttackRange();
        int hp = player->getHp();
        if(atk > hp && !player->hasSkill("yingzi")){
            room->acquireSkill(player, "yingzi");
            player->setMark("yuanzi", 1);
        }
        else if(atk < hp && !player->hasSkill("leiguang")){
            room->acquireSkill(player, "leiguang");
            player->setMark("yuanzi", 3);
        }
        else if(!player->hasSkill("yinghun")){
            room->acquireSkill(player, "yinghun");
            player->setMark("yuanzi", 2);
        }
        else
            player->setMark("yuanzi", 0);
        return false;
    }
};

ShengjianCard::ShengjianCard(){
    once = true;
}

bool ShengjianCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(subcards.isEmpty())
        return Self->distanceTo(to_select) <= Self->getLostHp();
    else if(subcards.length() == 1)
        return Self->distanceTo(to_select) > Self->getLostHp();
    else
        return false;
}

void ShengjianCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
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

class ShengjianViewAsSkill: public ViewAsSkill{
public:
    ShengjianViewAsSkill(): ViewAsSkill("shengjian"){

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
        ShengjianCard *card = new ShengjianCard;
        card->addSubcards(cards);
        card->setSkillName("shengjian");
        return card;
    }
};

class Shengjian: public TriggerSkill{
public:
    Shengjian(): TriggerSkill("shengjian"){
        events << PhaseChange;
        view_as_skill = new ShengjianViewAsSkill;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        if(player->getPhase() == Player::Start && player->isWounded()){
            player->getRoom()->askForUseCard(player, "@shengjian-card", "@shengjian");
        }

        return false;
    }
};

class Binghuan:public MasochismSkill{
public:
    Binghuan():MasochismSkill("binghuan"){
    }

    virtual void onDamaged(ServerPlayer *player, const DamageStruct &damage) const{
        Room *room = player->getRoom();
        if(!player->askForSkillInvoke(objectName()))
            return;
        for(int i=0; i<damage.damage; i++){
            QList<int> cards;
            cards << room->drawCard();
            LogMessage log;
            log.type = "$BinghuanCard";
            log.from = player;
            log.card_str = QString::number(cards.last());
            room->sendLog(log);
            room->throwCard(cards.last());
            room->getThread()->delay();

            cards << room->drawCard();
            log.card_str = QString::number(cards.last());
            room->sendLog(log);
            room->throwCard(cards.last());

            if(Sanguosha->getCard(cards.first())->getSuit() != Sanguosha->getCard(cards.last())->getSuit()){
                room->fillAG(cards);
                int getit = room->askForAG(player, cards, false, objectName());
                cards.removeOne(getit);
                room->takeAG(player, getit);
                player->addToPile("huan", cards.first());
                room->setEmotion(player, "draw-card");
                room->broadcastInvoke("clearAG");
            }
            else
                room->setEmotion(player, "bad");
            cards.clear();
        }
    }
};

BingjiuCard::BingjiuCard(){
}

bool BingjiuCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty();
}

void BingjiuCard::onEffect(const CardEffectStruct &effect) const{
    ServerPlayer *target = effect.to;
    Room *room = target->getRoom();
    DummyCard *move_cards = target->wholeHandCards();
    if(move_cards){
        room->moveCardTo(move_cards, target, Player::Special);
        foreach(int id, move_cards->getSubcards()){
            target->addToPile("bingjiu", id, false);
        }
        delete move_cards;
    }
    LogMessage log;
    log.from = effect.from;
    log.to << target;
    target->setMark("jiu", 1);
    log.type = "#Bingjiu";
    log.arg = effect.card->getSuitString();
    log.arg2 = "huan";
    room->sendLog(log);
}

class BingjiuViewAsSkill: public OneCardViewAsSkill{
public:
    BingjiuViewAsSkill():OneCardViewAsSkill("bingjiu"){
    }
    virtual bool isEnabledAtPlay(const Player *) const{
        return false;
    }
    virtual bool viewFilter(const CardItem *to_select) const{
        QString str = ClientInstance->getPattern().split(".").last();
        return !to_select->isEquipped() && to_select->getCard()->getSuitString() == str;
    }
    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return pattern.startsWith("@@bingjiu");
    }
    virtual const Card *viewAs(CardItem *card_item) const{
        BingjiuCard *card = new BingjiuCard;
        card->addSubcard(card_item->getCard()->getId());

        return card;
    }
};

class Bingjiu: public PhaseChangeSkill{
public:
    Bingjiu():PhaseChangeSkill("bingjiu"){
        view_as_skill = new BingjiuViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        ServerPlayer *kamiao = target->getRoom()->findPlayerBySkillName(objectName());
        return kamiao;
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        Room *room = player->getRoom();
        if(player->getPhase() == Player::NotActive && player->tag.value("inice").toBool()){
            ServerPlayer *target = player;
            foreach(ServerPlayer *tmp, room->getAllPlayers()){
                if(tmp->getMark("jiu") > 0){
                    target = tmp;
                    break;
                }
            }
            foreach(int card_id, target->getPile("bingjiu")){
                if(!target)
                    room->throwCard(card_id);
                else
                    room->obtainCard(target, card_id);
            }
            LogMessage log;
            log.from = target;
            log.type = "#BJProtectRemove";
            target->setMark("jiu", 0);
            log.arg = objectName();
            room->sendLog(log);

            player->tag["inice"] = false;
            return false;
        }
        if(player->getPhase() != Player::Start)
            return false;
        ServerPlayer *kamiao = room->findPlayerBySkillName(objectName());
        if(!kamiao)
            return false;
        if(!kamiao->getPile("huan").isEmpty() && !kamiao->isKongcheng()){
            QList<int> huan = kamiao->getPile("huan");
            foreach(int tmp1, huan){
                foreach(const Card *tmp2, kamiao->getHandcards()){
                    if(Sanguosha->getCard(tmp1)->getSuit() == tmp2->getSuit())
                        goto s_mark;
                }
            }
            return false;
            s_mark:
            if(!room->askForSkillInvoke(kamiao, objectName()))
                return false;
            int card_id;
            if(huan.length() == 1)
                card_id = huan.first();
            else{
                room->fillAG(huan, kamiao);
                card_id = room->askForAG(kamiao, huan, true, objectName());
                kamiao->invoke("clearAG");
                if(card_id == -1)
                    return false;
            }
            huan.clear();
            QString suit_str = Sanguosha->getCard(card_id)->getSuitString();
            QString pattern = QString("@@bingjiu.%1").arg(suit_str);
            if(room->askForUseCard(kamiao, pattern, "@bingjiu:" + suit_str)){
                room->throwCard(card_id);
                player->tag["inice"] = true;
            }
        }
        return false;
    }
};

class BingjiuEffect: public TriggerSkill{
public:
    BingjiuEffect():TriggerSkill("#bingjiu_effect"){
        events << Predamaged;
    }

    virtual int getPriority() const{
        return 2;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return !target->getPile("bingjiu").isEmpty();
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.nature == DamageStruct::Normal){
            Room *room = player->getRoom();

            LogMessage log;
            log.type = "#BJProtect";
            log.from = player;
            log.arg = QString::number(damage.damage);
            log.arg2 = "bingjiu";
            room->sendLog(log);

            room->setEmotion(player, "good");
            return true;
        }else
            return false;
    }
};

class Meigui: public TriggerSkill{
public:
    Meigui():TriggerSkill("meigui"){
        events << SlashEffect;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *player, QVariant &data) const{
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        Room *room = player->getRoom();

        if(effect.slash->isRed() && !effect.to->isKongcheng() && effect.from->askForSkillInvoke(objectName(), data)){
            room->askForDiscard(effect.to, objectName(), 1);
        }
        if(effect.slash->isBlack() && effect.from->askForSkillInvoke(objectName(), data)){
            if(room->askForDiscard(effect.from, objectName(), 1, true)){
                room->slashResult(effect, NULL);
                return true;
            }
        }

        return false;
    }
};

class Xuexing: public TriggerSkill{
public:
    Xuexing():TriggerSkill("xuexing"){
        events << Death;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        DamageStar damage = data.value<DamageStar>();
        ServerPlayer *killer = damage ? damage->from : NULL;

        if(killer){
            Room *room = player->getRoom();
            if(killer != player && !killer->hasSkill("benghuai")){
                killer->gainMark("@collapse");
                room->acquireSkill(killer, "benghuai");
                room->setEmotion(killer, "bad");
            }
        }
        return false;
    }
};

GoldSeintoViVAPackage::GoldSeintoViVAPackage()
    :Package("goldseintoviva")
{
    General *aries, *taurus, *gemini, *cancer, *leo, *virgo,
    *libra, *scorpio, *sagittarius, *capricorn, *aquarius, *pisces;

    aries = new General(this, "aries", "gold", 3);
    aries->addSkill(new Xiufu);
    aries->addSkill(new Jingqiang);
    aries->addSkill(new JingqiangClear);
    related_skills.insertMulti("jingqiang", "#jingqiang-clear");
    aries->addSkill(new Xingmie);
    aries->addSkill(new MarkAssignSkill("@xm", 1));
    related_skills.insertMulti("xingmie", "#@xm");

    taurus = new General(this, "taurus", "gold");
    taurus->addSkill(new Haojiao);

    gemini = new General(this, "gemini", "gold", 3);
    gemini->addSkill(new Huanlong);
    gemini->addSkill(new Kongjian);
    gemini->addSkill(new Xingbao);

    cancer = new General(this, "cancer", "gold");
    cancer->addSkill(new Shiqi);
    cancer->addSkill(new ShiqiEffect);
    related_skills.insertMulti("shiqi", "#shiqi-effect");

    leo = new General(this, "leo", "gold");
    leo->addSkill(new Leiguang);

    virgo = new General(this, "virgo", "gold");
    virgo->addSkill(new Budong);
    virgo->addSkill(new Liudao);
    virgo->addSkill(new LiudaoWaked);
    related_skills.insertMulti("liudao", "#liudao_waked");
    virgo->addSkill(new Baolun);
    virgo->addSkill(new BaolunDistance);
    related_skills.insertMulti("baolun", "#baolun_distance");

    libra = new General(this, "libra", "gold");
    libra->addSkill(new Longba);
    libra->addSkill(new Longfei);

    scorpio = new General(this, "scorpio", "gold");
    scorpio->addSkill(new Hongzhen);

    sagittarius = new General(this, "sagittarius", "gold");
    sagittarius->addSkill(new Yuanzi);

    capricorn = new General(this, "capricorn", "gold");
    capricorn->addSkill(new Shengjian);

    aquarius = new General(this, "aquarius", "gold", 3);
    aquarius->addSkill(new Binghuan);
    aquarius->addSkill(new Bingjiu);
    aquarius->addSkill(new BingjiuEffect);
    related_skills.insertMulti("bingjiu", "#bingjiu_effect");

    pisces = new General(this, "pisces", "gold", 3);
    pisces->addSkill(new Meigui);
    pisces->addSkill(new Xuexing);

    addMetaObject<XiufuCard>();
    addMetaObject<XingmieCard>();
    addMetaObject<HaojiaoCard>();
    addMetaObject<HuanlongCard>();
    addMetaObject<ShiqiCard>();
    addMetaObject<BaolunCard>();
    addMetaObject<HongzhenCard>();
    addMetaObject<ShengjianCard>();
    addMetaObject<BingjiuCard>();

    skills << new HongzhenEffect;
}

ADD_PACKAGE(GoldSeintoViVA)
