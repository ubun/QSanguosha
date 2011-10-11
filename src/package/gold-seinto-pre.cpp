#include "standard.h"
#include "skill.h"
#include "gold-seinto-pre.h"
#include "client.h"
#include "carditem.h"
#include "engine.h"
#include "maneuvering.h"
#include "ai.h"

class Niandong: public TriggerSkill{
public:
    Niandong():TriggerSkill("niandong"){
        events << CardUsed;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        CardUseStruct use = data.value<CardUseStruct>();
        if(player->getRoom()->getCurrent() != player || player->hasFlag("niandong"))
            return false;
        if(use.card->inherits("Slash") || use.card->inherits("Snatch")
            || use.card->inherits("SupplyShortage")){
            Room *room = player->getRoom();
            if(player->askForSkillInvoke(objectName())){
                QList<ServerPlayer *> players;
                foreach(ServerPlayer *tmp, room->getOtherPlayers(player)){
                    if(use.card->inherits("Slash") && tmp->isKongcheng() && tmp->hasSkill("kongcheng"))
                        continue;
                    if(use.card->inherits("Snatch") && tmp->hasSkill("qianxun"))
                        continue;
                    if(use.card->isBlack() && use.card->inherits("TrickCard") && tmp->hasSkill("weimu"))
                        continue;
                    players << tmp;
                }
                if(players.isEmpty()) return false;
                ServerPlayer *target = room->askForPlayerChosen(player, players, objectName());
                use.to.clear();
                use.to << target;
                player->setFlags("niandong");
                data = QVariant::fromValue(use);
            }
        }
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
            foreach(ServerPlayer *p, room->getAlivePlayers()){
                if(p->getMark("@jq") > 0){
                    p->loseAllMarks("@jq");
                    target = p;
                    break;
                }
            }
            if(target){
                foreach(ServerPlayer *p, room->getOtherPlayers(target)){
                    room->setFixedDistance(p, target, -1);
                }
            }
        }
        else if(player->getPhase() == Player::Finish){
            if(!room->askForSkillInvoke(player, objectName()))
                return false;

            target = room->askForPlayerChosen(player, room->getAlivePlayers(), objectName());
            foreach(ServerPlayer *p, room->getOtherPlayers(target)){
                room->setFixedDistance(p, target, target->distanceTo(p)+1);
            }
            target->gainMark("@jq");
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
        QList<ServerPlayer *> players = room->getAllPlayers();
        ServerPlayer *target = NULL;
        foreach(ServerPlayer *p, players){
            if(p->getMark("@jq") > 0){
                p->loseAllMarks("@jq");
                target = p;
                break;
            }
        }
        if(target){
            foreach(ServerPlayer *p, room->getOtherPlayers(target)){
                room->setFixedDistance(p, target, -1);
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

        if(to_select->isEquipped())
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
        source->loseMark("@xm");
        target = room->askForPlayerChosen(source, targets, "xingmie");
        DamageStruct damage;
        damage.from = source;
        damage.to = target;
        damage.damage = target->getHandcardNum();
        room->damage(damage);
    }
    room->throwCard(this);
}

HaojiaoCard::HaojiaoCard(){
    once = true;
}

bool HaojiaoCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(targets.length() < 2)
        return Self->inMyAttackRange(to_select) && !Self->isProhibited(to_select, new Slash(Card::NoSuit, 0));
    else
        return false;
}

void HaojiaoCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    Slash *slash = new Slash(Card::NoSuit, 0);
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
        return pattern == "@haojiao-card";
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

            if(room->askForUseCard(player, "@haojiao-card", "@@haojiao"))
                player->turnOver();
        }
        else if(event == Damaged){
            if(player->faceUp() || !room->askForSkillInvoke(player, objectName()))
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
            if(winner == player)
                player->getRoom()->showAllCards(pindian->to, player);
        }

        return false;
    }
};

class Kongjian:public MasochismSkill{
public:
    Kongjian():MasochismSkill("kongjian"){
    }

    virtual void onDamaged(ServerPlayer *baby, const DamageStruct &damage) const{
        if(!damage.from || damage.to == damage.from)
            return;
        Room *room = baby->getRoom();
        Card::Suit suit = room->askForSuit(baby);
        QString suitpattern = ".";
        switch(suit){
        case Card::Heart : suitpattern = ".H"; break;
        case Card::Diamond : suitpattern = ".D"; break;
        case Card::Spade : suitpattern = ".S"; break;
        case Card::Club : suitpattern = ".C"; break;
        default: break;
        }

        const Card *card = room->askForCard(damage.from, suitpattern, objectName(), NULL, false);
        if(card){
            baby->addToPile("star", card->getId());
        }
        else{
            DamageStruct damage2;
            damage2.from = NULL;
            damage2.to = damage.from;
            room->damage(damage2);
        }
    }
};

class XingbaoStart: public TriggerSkill{
public:
    XingbaoStart():TriggerSkill("#xingbao_start"){
        events << GameStart;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill("xingbao");
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &) const{
        player->getRoom()->acquireSkill(player, "kongjian");
        return false;
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
        foreach(ServerPlayer *p, room->getAllPlayers()){
            if(p->getMark("@shiqi")>0){
                target = p;
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
        if(move->from_place == Player::Hand){
            JudgeStruct judge;
            judge.pattern = QRegExp("(.*):(spade):(.*)");
            judge.good = false;
            judge.reason = objectName();
            judge.who = virgo;

            room->judge(judge);
            if(judge.isGood()){
                virgo->addToPile("guo", judge.card->getId());
            }
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
        return (PhaseChangeSkill::triggerable(target)
                && target->getPhase() == Player::Start
                && target->getMark("liudao") == 0
                && target->getPile("guo").length() >= 6) ||
                (target->getPhase() == Player::Draw
                && target->getMark("liudao") != 0
                && !target->getPile("guo").isEmpty());
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        Room *room = player->getRoom();

        if(player->getMark("liudao") != 0){
            QList<int> guoguo = player->getPile("guo");
            room->fillAG(guoguo, player);
            int card_id = room->askForAG(player, guoguo, true, "guo");
            if(card_id != -1){
                guoguo.removeOne(card_id);
                room->moveCardTo(Sanguosha->getCard(card_id), player, Player::Hand, false);
            }
            player->invoke("clearAG");
            return false;
        }
        LogMessage log;
        log.type = "#LiudaoWake";
        log.from = player;
        room->sendLog(log);

        player->setMark("liudao", 1);
        room->loseMaxHp(player);

        return false;
    }
};

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
            && !damage.to->isAllNude()){
            Room *room = libra->getRoom();
            if(room->askForSkillInvoke(libra, objectName())){
                int to_throw = room->askForCardChosen(libra, damage.to, "he", objectName());
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
        return ;

    int card_id;
    if(card_ids.length() == 1)
        card_id = card_ids.first();
    else{
        room->fillAG(card_ids, player);

        //remove other
        QList<int> numlist,numenable;
        for(int i = 1; i < 14; i++)
            numlist << i;
        foreach(int i, card_ids){
            const Card *c = Sanguosha->getCard(i);
            if(numlist.contains(c->getNumber())){
                numlist.removeOne(c->getNumber());
            }
            else
                numenable << c->getNumber();
        }
        QMutableListIterator<int> itor(card_ids);
        while(itor.hasNext()){
            const Card *c = Sanguosha->getCard(itor.next());
            if(!numenable.contains(c->getNumber())){
                itor.remove();
                room->takeAG(NULL, c->getId());
            }
        }

        card_id = room->askForAG(player, card_ids, true, "hongzhen");
        player->invoke("clearAG");

        if(card_id == -1)
            return;
    }

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

class Hongzhen: public TriggerSkill{
public:
    Hongzhen():TriggerSkill("hongzhen"){
        events << CardFinished << CardResponsed;
        view_as_skill = new HongzhenViewAsSkill;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        CardStar card = NULL;
        if(event == CardFinished){
            CardUseStruct card_use = data.value<CardUseStruct>();
            card = card_use.card;
        }else if(event == CardResponsed)
            card = data.value<CardStar>();

        if((card->isRed() && card->inherits("BasicCard")) || card->isNDTrick()){
            int card_id = card->getSubcards().isEmpty()? card->getId(): card->getSubcards().first();
            if(card_id > -1)
                player->addToPile("needle", card_id);

            if(player->getPile("needle").length() >= 15 && player->askForSkillInvoke(objectName(), data)){
                Room *room = player->getRoom();
                QList<ServerPlayer *> targets;
                foreach(ServerPlayer *tmp, room->getAlivePlayers()){
                    if(player->inMyAttackRange(tmp))
                        targets << tmp;
                }
                if(targets.isEmpty())
                    return false;
                ServerPlayer *target = room->askForPlayerChosen(player, targets, objectName());
                for(int i = player->getPile("needle").length(); i > 0 ;i--){
                    if(!player->getPile("needle").isEmpty())
                        room->throwCard(player->getPile("needle").first());
                }

                room->loseHp(player, 3);
                DamageStruct damage;
                damage.from = player;
                damage.to = target;
                damage.damage = 3;
                room->damage(damage);
            }
        }
        return false;
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

class JiguangClear: public TriggerSkill{
public:
    JiguangClear():TriggerSkill("#jiguang-clear"){
        events << Death;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill("jiguang");
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &) const{
        Room *room = player->getRoom();
        foreach(ServerPlayer *tmp, room->getAlivePlayers()){
            if(room->findPlayerBySkillName("jiguang")  == tmp)
                return false;
        }
        foreach(ServerPlayer *tmp, room->getAlivePlayers()){
            if(tmp->getPile("gas").isEmpty())
                continue;
            foreach(int i, tmp->getPile("gas"))
                room->throwCard(i);
        }
        return false;
    }
};

class Jiguang: public TriggerSkill{
public:
    Jiguang():TriggerSkill("jiguang"){
        events << Damage << Damaged;
        frequency = Frequent;
    }

    virtual int getPriority() const{
        return -1;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        DamageStruct damage = data.value<DamageStruct>();

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
            judge.pattern = QRegExp("(.*):(heart|diamond):(.*)");
            judge.good = true;
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
        if(!pisces || pisces->isKongcheng())
            return false;
        if(use.to.length() == 1 && use.to.contains(pisces))
            return false;
        if(!pisces->askForSkillInvoke(objectName(), data))
            return false;
        const Card *card = room->askForCardShow(pisces, pisces, "mogongshow");
        //const Card *card = room->askForCard(pisces, ".C", "mogongshow", data);
        if(card && card->getSuit() == Card::Club){
            //pisces->obtainCard(card);
            //Show(pisces, use.from, objectName())->getSuit() == Card::Club)
            //room->showCard(pisces, card->getEffectiveId());
            use.to.clear();
            use.to << pisces;

            LogMessage log;
            log.type = "#Mogong";
            log.arg = objectName();
            log.from = pisces;
            room->sendLog(log);
            data = QVariant::fromValue(use);
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

GoldSeintoPrePackage::GoldSeintoPrePackage()
    :Package("goldseintopre")
{
    General *aries, *taurus, *gemini, *cancer, *leo, *virgo,
    *libra, *scorpio, *sagittarius, *capricorn, *aquarius, *pisces;

    aries = new General(this, "aries", "st", 3);
    aries->addSkill(new Niandong);
    aries->addSkill(new Jingqiang);
    aries->addSkill(new JingqiangClear);
    related_skills.insertMulti("jingqiang", "#jingqiang-clear");
    aries->addSkill(new Xingmie);
    aries->addSkill(new MarkAssignSkill("@xm", 1));
    related_skills.insertMulti("xingmie", "#@xm");

    taurus = new General(this, "taurus", "st");
    taurus->addSkill(new Haojiao);

    gemini = new General(this, "gemini", "st", 3);
    gemini->addSkill(new Huanlong);
    gemini->addSkill(new Xingbao);
    gemini->addSkill(new XingbaoStart);
    related_skills.insertMulti("xingbao", "#xingbao_start");

    cancer = new General(this, "cancer", "st");
    cancer->addSkill(new Shiqi);
    cancer->addSkill(new ShiqiEffect);
    related_skills.insertMulti("shiqi", "#shiqi-effect");

    leo = new General(this, "leo", "st");
    leo->addSkill(new Leiguang);

    virgo = new General(this, "virgo", "st");
    virgo->addSkill(new Budong);
    virgo->addSkill(new Liudao);
    virgo->addSkill(new Baolun);

    libra = new General(this, "libra", "st");
    libra->addSkill(new Longba);
    libra->addSkill(new Longfei);

    scorpio = new General(this, "scorpio", "st");
    scorpio->addSkill(new Hongzhen);

    sagittarius = new General(this, "sagittarius", "st");
    sagittarius->addSkill(new Yuanzi);

    capricorn = new General(this, "capricorn", "st");
    capricorn->addSkill(new Shengjian);

    aquarius = new General(this, "aquarius", "st", 3);
    aquarius->addSkill(new Jiguang);
    aquarius->addSkill(new JiguangClear);
    aquarius->addSkill(new Wangqi);
    related_skills.insertMulti("jiguang", "#jiguang-clear");

    pisces = new General(this, "pisces", "st", 3);
    pisces->addSkill(new Mogong);
    pisces->addSkill(new Jingji);

    addMetaObject<XingmieCard>();
    addMetaObject<HaojiaoCard>();
    addMetaObject<HuanlongCard>();
    addMetaObject<ShiqiCard>();
    addMetaObject<BaolunCard>();
    addMetaObject<HongzhenCard>();
    addMetaObject<ShengjianCard>();

    skills << new Kongjian;
}

ADD_PACKAGE(GoldSeintoPre)
