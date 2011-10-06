#include "red.h"
#include "skill.h"
#include "client.h"
#include "engine.h"
#include "carditem.h"
#include "room.h"
//#include "maneuvering.h"

TongmouCard::TongmouCard(){
    target_fixed = true;
}

void TongmouCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    ServerPlayer *tmp = NULL;
    foreach(tmp, room->getOtherPlayers(source)){
        if(tmp->getMark("xoxo") > 0)
            break;
    }
    ServerPlayer *zhonghui, *gay;
    if(source->hasSkill("tongmou")){
        zhonghui = source;
        gay = tmp;
    }
    else{
        zhonghui = tmp;
        gay = source;
    }
    if(!gay || !zhonghui) return;
    QList<ServerPlayer *> gays;
    gays << zhonghui << gay;
    QStringList forbid_skills;
    forbid_skills << "lianying" << "tuntian" << "shangshi" << "beifa";
    foreach(ServerPlayer *tmp, gays){
        if(tmp->hasSkill("tuntian") && tmp == source)
            continue;
        if(tmp->hasSkill("juxiang") && tmp != source)
            source->setFlags("sa_forbidden");
        foreach(QString tmp2, forbid_skills){
            if(tmp->hasSkill(tmp2)){
                if(source->hasFlag("forct")){
                    LogMessage log;
                    log.type = "#Tongmou_forbidden";
                    log.from = tmp;
                    log.arg = "tongmou";
                    log.arg2 = tmp2;
                    room->sendLog(log);
                    return;
                }
                source->setFlags("forct");
            }
        }
    }

    DummyCard *card1 = zhonghui->wholeHandCards();
    DummyCard *card2 = gay->wholeHandCards();

    if(card1){
        room->moveCardTo(card1, gay, Player::Hand, false);
        delete card1;
    }
    //room->getThread()->delay();
    if(card2){
        room->moveCardTo(card2, zhonghui, Player::Hand, false);
        delete card2;
    }
/*
    foreach(int card_id, zhonghui->handCards()){
        zhonghui->addToPile("mycard", card_id, false);
    }
    foreach(int card_id, gay->handCards()){
        room->moveCardTo(Sanguosha->getCard(card_id), zhonghui, Player::Hand, false);
    }
    foreach(int card_id, zhonghui->getPile("mycard")){
        room->moveCardTo(Sanguosha->getCard(card_id), gay, Player::Hand, false);
    }
*/
    source->tag["flag"] = !source->tag.value("flag", true).toBool();
}

class TongmouViewAsSkill: public ZeroCardViewAsSkill{
public:
    TongmouViewAsSkill():ZeroCardViewAsSkill("tongmouv"){
    }
    virtual const Card *viewAs() const{
        return new TongmouCard;
    }
};

class TongmouAsSkill: public ZeroCardViewAsSkill{
public:
    TongmouAsSkill():ZeroCardViewAsSkill("tongmou"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->hasSkill(objectName());
    }

    virtual const Card *viewAs() const{
        return new TongmouCard;
    }
};

class Tongmou: public PhaseChangeSkill{
public:
    Tongmou():PhaseChangeSkill("tongmou"){
        view_as_skill = new TongmouAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        Room *room = player->getRoom();
        ServerPlayer *zhonghui = room->findPlayerBySkillName(objectName());
        if(!zhonghui) return false;
        if(player != zhonghui && player->getMark("xoxo") < 1)
            return false;

        if(player->getMark("xoxo") > 0 && player->getPhase() == Player::Discard){
            ServerPlayer *gay = NULL;
            foreach(gay, room->getOtherPlayers(player)){
                if(gay->getMark("xoxo") > 0)
                    break;
            }
            if(gay && !player->tag.value("flag", true).toBool()){
                DummyCard *card1 = player->wholeHandCards();
                DummyCard *card2 = gay->wholeHandCards();

                if(card1){
                    room->moveCardTo(card1, gay, Player::Hand, false);
                    delete card1;
                }
                //room->getThread()->delay();
                if(card2){
                    room->moveCardTo(card2, player, Player::Hand, false);
                    delete card2;
                }
                /*
                foreach(int card_id, player->handCards()){
                    player->addToPile("mycard", card_id, false);
                }
                foreach(int card_id, gay->handCards()){
                    room->moveCardTo(Sanguosha->getCard(card_id), player, Player::Hand, false);
                }
                foreach(int card_id, player->getPile("mycard")){
                    room->moveCardTo(Sanguosha->getCard(card_id), gay, Player::Hand, false);
                }*/
            }
        }
        if(player == zhonghui && player->getPhase() == Player::Play){
            if(!player->askForSkillInvoke(objectName())){
                zhonghui->setMark("xoxo",0);
                return false;
            }
            QList<ServerPlayer *> players;
            foreach(ServerPlayer *p, room->getOtherPlayers(zhonghui)){
                /*if(p->hasSkill("lianying") || p->hasSkill("tuntian")
                || p->hasSkill("shangshi") || p->hasSkill("beifa"))
                    continue;*/
                players << p;
            }
            ServerPlayer *gay = room->askForPlayerChosen(zhonghui, players, "tongmou_tie");
            if(!gay) return false;
            player->tag["flag"] = true;
            gay->addMark("xoxo");
            zhonghui->addMark("xoxo");

            room->attachSkillToPlayer(gay, "tongmouv");
        }
        else if(player != zhonghui && player->getMark("xoxo") > 0 && player->getPhase() == Player::Play){
            player->tag["flag"] = true;
        }
        else if(player != zhonghui && player->getMark("xoxo") > 0 && player->getPhase() == Player::Finish){
            player->setMark("xoxo", 0);
            zhonghui->setMark("xoxo", 0);
            room->detachSkillFromPlayer(player, "tongmouv");
        }
        return false;
    }
};

class TongmouForbidden: public TriggerSkill{
public:
    TongmouForbidden():TriggerSkill("#tmf"){
        events << CardUsed;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getMark("xoxo") > 0;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *zhonghui, QVariant &dat) const{
        Room *room = zhonghui->getRoom();
        CardUseStruct use = dat.value<CardUseStruct>();
        if(use.from && use.from == zhonghui && use.from->hasFlag("sa_forbidden")
            && use.card->inherits("SavageAssault")){
            room->throwCard(use.card);
            return true;
        }
        return false;
    }
};

class TongmouClear: public TriggerSkill{
public:
    TongmouClear():TriggerSkill("#tmc"){
        events << Death;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill("tongmou");
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *zhonghui, QVariant &) const{
        Room *room = zhonghui->getRoom();
        foreach(ServerPlayer *player, room->getAllPlayers()){
            player->setMark("xoxo", 0);
            room->detachSkillFromPlayer(player, "tongmouv");
        }
        return false;
    }
};

class Xianhai: public OneCardViewAsSkill{
public:
    Xianhai():OneCardViewAsSkill("xianhai"){

    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getCard()->inherits("Disaster");
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        XianhaiCard *card = new XianhaiCard;
        card->addSubcard(card_item->getCard()->getId());

        return card;
    }
};

XianhaiCard::XianhaiCard(){
}

bool XianhaiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(to_select->getJudgingArea().contains(Sanguosha->getCard(this->getSubcards().first())))
        return false;
    return targets.isEmpty();
}

void XianhaiCard::onEffect(const CardEffectStruct &effect) const{
    effect.from->getRoom()->moveCardTo(Sanguosha->getCard(this->getSubcards().first()), effect.to, Player::Judging);
}

class BaichuViewAsSkill:public OneCardViewAsSkill{
public:
    BaichuViewAsSkill():OneCardViewAsSkill("baichu"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("BaichuCard");
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        switch(ClientInstance->getStatus()){
        case Client::Playing:{
                return !to_select->isEquipped();
            }

        case Client::Responsing:{
                const Card *card = to_select->getFilteredCard();
                QString pattern = ClientInstance->getPattern();
                if(pattern == "jink")
                    return card->getNumber() >= Sanguosha->getCard(Self->getPile("ji").first())->getNumber();
            }

        default:
            return false;
        }
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "jink";
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        bool isactive = Self->getPhase() != Player::NotActive
                        ? true :false;
        if(isactive){
            BaichuCard *card = new BaichuCard;
            card->addSubcard(card_item->getCard()->getId());
            return card;
        }
        else{
            const Card *card = card_item->getFilteredCard();
            Jink *jink = new Jink(card->getSuit(), card->getNumber());
            jink->addSubcard(card);
            jink->setSkillName(objectName());
            return jink;
        }
        return NULL;
    }
};

BaichuCard::BaichuCard(){
    once = true;
    target_fixed = true;
    will_throw = false;
}

void BaichuCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    room->obtainCard(source, source->getPile("ji").first());
    source->addToPile("ji", this->getSubcards().first(), false);
}

class Baichu: public TriggerSkill{
public:
    Baichu():TriggerSkill("baichu"){
        events << TurnStart << PhaseChange << CardUsed;
        view_as_skill = new BaichuViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *pp, QVariant &data) const{
        Room *room = pp->getRoom();
        ServerPlayer *player = room->findPlayerBySkillName("baichu");
        if(!player)
            return false;
        if(event == PhaseChange && pp->getPhase() == Player::Finish){
            player->addMark("begin");
            player->setMark("bc_count", 0);
        }
        if(player->getMark("begin") == 0 && event == TurnStart){
            player->drawCards(1);
            const Card *card = room->askForCard(player, ".", "@baichu", false, false);
            if(card)
                player->addToPile("ji", card->getId());
            else
                player->addToPile("ji", player->getHandcards().last()->getId());
        }
        if(pp != player)
            return false;
        const Card *card = NULL;
        if(player->getPhase() != Player::NotActive){
            if(event == CardUsed){
                CardUseStruct use = data.value<CardUseStruct>();
                if(use.card->getSubtype() != "skill_card")
                    card = use.card;
            }
            if(player->getMark("bc_count") < 5 &&
               card && card->getNumber() < Sanguosha->getCard(player->getPile("ji").first())->getNumber()
                && room->askForSkillInvoke(player, "baichu", data)){
                    player->drawCards(1);
                    player->addMark("bc_count");
                }
        }
        return false;
    }
};

class TongluViewAsSkill: public ZeroCardViewAsSkill{
public:
    TongluViewAsSkill():ZeroCardViewAsSkill("tonglu"){}
    virtual const Card *viewAs() const{
        return new TongluCard;
    }
};

class Tonglu: public TriggerSkill{
public:
    Tonglu():TriggerSkill("tonglu"){
        events << CardFinished << Predamage;
        view_as_skill = new TongluViewAsSkill;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        if(event == CardFinished){
            CardUseStruct card_use = data.value<CardUseStruct>();
            if(!card_use.card->inherits("Slash"))
                return false;
            player->loseAllMarks("@wocao");
            return false;
        }
        DamageStruct damage = data.value<DamageStruct>();
        int x = player->getMark("@wocao");
        if(damage.card && damage.card->inherits("Slash") && x > 0){
            damage.damage += x;
            player->loseAllMarks("@wocao");
            data = QVariant::fromValue(damage);

            LogMessage log;
            log.type = "#TongluBuff";
            log.from = player;
            log.arg = QString::number(x);
            log.arg2 = QString::number(damage.damage);
            player->getRoom()->sendLog(log);
        }

        return false;
    }
};

TongluCard::TongluCard(){
    target_fixed = true;
}

void TongluCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    QList<ServerPlayer *> players;
    foreach(ServerPlayer *tmp, room->getOtherPlayers(source)){
        if(tmp->faceUp())
            players << tmp;
    }
    if(players.isEmpty())
        return;
    source->setMark("@wocao", 0);
    foreach(ServerPlayer *tmp, players){
        QString result = room->askForChoice(tmp, "tonglu", "agree+deny");
        if(tmp->getState() == "robot" || tmp->getState() == "offline" || tmp->getState() == "trust"){
            if(tmp->getRole() == source->getRole())
                result = "agree";
            else if(source->isLord() && tmp->getRole() == "loyalist")
                result == "agree";
            else
                result = "deny";
        }
        if(result != "deny"){
            tmp->turnOver();
            source->gainMark("@wocao", 1);
        }
    }
}

class Liehou: public TriggerSkill{
public:
    Liehou():TriggerSkill("liehou"){
        events << CardDiscarded << PhaseChange;
        default_choice = "draw";
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        QList<ServerPlayer *> sbmen;
        foreach(ServerPlayer *tmp, room->getAllPlayers()){
            if(!tmp->faceUp())
                sbmen << tmp;
        }
        if(event == PhaseChange){
            if(player->getPhase() == Player::Finish && !player->hasFlag("liehou_on") && !sbmen.isEmpty()){
                QString result = room->askForChoice(player, objectName(), "draw+cancel");
                if(result == "cancel")
                    return false;
                LogMessage log;
                log.type = "#InvokeSkill";
                log.arg = objectName();
                log.from = player;
                room->sendLog(log);
                ServerPlayer *target = room->askForPlayerChosen(player, sbmen, "liehou");
                target->drawCards(1);
            }
            return false;
        }
        if(player->getPhase() == Player::Discard && !sbmen.isEmpty()){

            CardStar card = data.value<CardStar>();
            DummyCard *dummy = new DummyCard;
            foreach(int card_id, card->getSubcards()){
                dummy->addSubcard(Sanguosha->getCard(card_id));
            }

            QString result = room->askForChoice(player, objectName(), "get+draw+cancel");
            player->setFlags("liehou_on");
            if(result == "cancel")
                return false;
            LogMessage log;
            log.type = "#InvokeSkill";
            log.arg = objectName();
            log.from = player;
            room->sendLog(log);

            ServerPlayer *target = room->askForPlayerChosen(player, sbmen, "liehou");
            if(result == "draw")
                target->drawCards(1);
            else{
                log.type = "#Liehou";
                log.arg = QString::number(dummy->subcardsLength());
                log.to << target;
                room->sendLog(log);

                target->obtainCard(dummy);
                delete dummy;
            }
        }
        return false;
    }
};

class Zide: public PhaseChangeSkill{
public:
    Zide():PhaseChangeSkill("zide"){
        frequency = Compulsory;
    }
    virtual bool onPhaseChange(ServerPlayer *player) const{
        if(player->getPhase() == Player::Finish && !player->hasUsed("TongluCard")){
            LogMessage log;
            log.type = "#TriggerSkill";
            log.arg = objectName();
            log.from = player;
            player->getRoom()->sendLog(log);

            player->drawCards(1);
        }
        return false;
    }
};

class XiefangViewAsSkill:public ZeroCardViewAsSkill{
public:
    XiefangViewAsSkill():ZeroCardViewAsSkill("xiefang"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return Slash::IsAvailable(player);
    }

    virtual const Card *viewAs() const{
        return new XiefangCard;
    }
};

XiefangCard::XiefangCard(){
}

bool XiefangCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    return targets.length() == 2 || targets.length() == 1;
}

bool XiefangCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(targets.isEmpty()){
        return to_select->getWeapon() && to_select != Self;
    }else if(targets.length() == 1){
        const Player *first = targets.first();
        return first != Self && first->getWeapon() && Self->canSlash(to_select);
    }else
        return false;
}

void XiefangCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    const Card *weapon = targets.at(0)->getWeapon();
    ServerPlayer *target;
    if(targets.length() == 1){
        if(!Self->canSlash(targets.first()))
            return;
        else
            target = targets.first();
    }
    else
        target = targets.at(1);

    if(weapon){
        Slash *slash = new Slash(weapon->getSuit(), weapon->getNumber());
        slash->setSkillName("xiefang");
        slash->addSubcard(weapon);
        CardUseStruct use;
        //room->throwCard(slash->getId());
        use.card = slash;
        use.from = source;
        use.to << target;
        room->useCard(use);
    }
}

class Xiefang: public TriggerSkill{
public:
    Xiefang():TriggerSkill("xiefang"){
        events << CardAsked;
        view_as_skill = new XiefangViewAsSkill;
    }

    virtual int getPriority() const{
        return 1;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *player, QVariant &data) const{
        QString asked = data.toString();
        if(asked == "slash" && player->getMark("yany") > 0)
            return false;
        Room *room = player->getRoom();
        QList<ServerPlayer *> players;
        foreach(ServerPlayer *tmp, room->getOtherPlayers(player)){
            if(asked == "slash" && tmp->getWeapon())
                players << tmp;
            else if(asked == "jink" &&
                    (tmp->getArmor() || tmp->getOffensiveHorse() || tmp->getDefensiveHorse()))
                players << tmp;
        }
        if(players.isEmpty())
            return false;
        if(room->askForSkillInvoke(player, objectName())){
            ServerPlayer *target = room->askForPlayerChosen(player, players, objectName());
            int card_id = asked == "slash" ?
                          target->getWeapon()->getId() :
                          room->askForCardChosen(player, target, "e", objectName());
            if(asked == "jink" && target->getWeapon() && target->getWeapon()->getId() == card_id)
                return false;
            const Card *card = Sanguosha->getCard(card_id);
            if(asked == "slash"){
                Slash *slash = new Slash(card->getSuit(), card->getNumber());
                slash->setSkillName(objectName());
                slash->addSubcard(card);
                room->provide(slash);
            }
            else if(asked == "jink"){
                Jink *jink = new Jink(card->getSuit(), card->getNumber());
                jink->setSkillName(objectName());
                jink->addSubcard(card);
                room->provide(jink);
            }
        }
        return false;
    }
};

class Yanyun: public TriggerSkill{
public:
    Yanyun():TriggerSkill("yanyun"){
        events << SlashMissed;
        frequency = Frequent;
    }

    virtual int getPriority() const{
        return -1;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *player, QVariant &data) const{
        SlashEffectStruct effect = data.value<SlashEffectStruct>();

        if(effect.slash->getSkillName() != "xiefang")
            return false;
        if(effect.to->hasSkill("kongcheng") && effect.to->isKongcheng())
            return false;

        Room *room = player->getRoom();
        player->setMark("yany", 1);
        if(room->askForSkillInvoke(player, objectName(), data)){
            const Card *card = room->askForCard(player, "slash", "yanyun-slash");
            player->setMark("yany", 0);
            if(card && card->getSkillName() != "xiefang"){
                if(player->hasFlag("drank"))
                    room->setPlayerFlag(player, "-drank");

                CardUseStruct use;
                use.card = card;
                use.from = player;
                use.to << effect.to;
                room->useCard(use, false);
            }
        }
        player->setMark("yany", 0);
        return false;
    }
};

class Jielue: public TriggerSkill{
public:
    Jielue():TriggerSkill("jielue"){
        events << SlashEffect << Pindian;
        frequency = Frequent;
    }

    virtual int getPriority() const{
        return -1;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        if(event == Pindian){
            PindianStar pindian = data.value<PindianStar>();
            if(pindian->reason == objectName() && pindian->isSuccess())
                pindian->from->obtainCard(pindian->to_card);
            return false;
        }
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        if(effect.slash->getNumber() == 0)
            return false;
        if(effect.slash && !effect.to->isKongcheng() && effect.from->askForSkillInvoke(objectName(), data))
            effect.from->pindian(effect.to, objectName(), effect.slash);
        return false;
    }
};

class Chuzhen: public PhaseChangeSkill{
public:
    Chuzhen():PhaseChangeSkill("chuzhen"){
        frequency = Wake;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target)
                && target->getMark("chuzhen") == 0
                && target->getPhase() == Player::Start
                && target->isWounded();
    }

    virtual bool onPhaseChange(ServerPlayer *pai) const{
        Room *room = pai->getRoom();

        LogMessage log;
        log.type = "#ChuzhenWake";
        log.from = pai;
        room->sendLog(log);

        room->setPlayerProperty(pai, "maxhp", pai->getMaxHP() + 1);
        room->acquireSkill(pai, "linjia");
        room->acquireSkill(pai, "zhubing");
        room->setPlayerMark(pai, "chuzhen", 1);
        return false;
    }
};

class Linjia: public TriggerSkill{
public:
    Linjia():TriggerSkill("linjia"){
        events << Predamaged << SlashEffected << CardEffected;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        if(event == SlashEffected){
            SlashEffectStruct effect = data.value<SlashEffectStruct>();
            if(effect.nature == DamageStruct::Normal){
                LogMessage log;
                log.from = player;
                log.type = "#LinjiaNullify";
                log.arg = objectName();
                log.arg2 = effect.slash->objectName();
                player->getRoom()->sendLog(log);

                return true;
            }
        }else if(event == CardEffected){
            CardEffectStruct effect = data.value<CardEffectStruct>();
            if(effect.card->inherits("SavageAssault")){
                LogMessage log;
                log.from = player;
                log.type = "#LinjiaNullify";
                log.arg = objectName();
                log.arg2 = effect.card->objectName();
                player->getRoom()->sendLog(log);

                return true;
            }
        }else if(event == Predamaged){
            DamageStruct damage = data.value<DamageStruct>();
            if(damage.nature == DamageStruct::Fire){
                LogMessage log;
                log.type = "#LinjiaDamage";
                log.from = player;
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

class Zhubing: public PhaseChangeSkill{
public:
    Zhubing():PhaseChangeSkill("zhubing"){
    }
    virtual bool onPhaseChange(ServerPlayer *pai) const{
        if(pai->getPhase() == Player::Draw){
            Room *room = pai->getRoom();
            if(room->askForSkillInvoke(pai, objectName())){
                int x = pai->getLostHp(), i;
                room->playSkillEffect(objectName(), 1);
                bool has_spade = false;

                for(i=0; i<x; i++){
                    int card_id = room->drawCard();
                    room->moveCardTo(Sanguosha->getCard(card_id), NULL, Player::Special, true);

                    room->getThread()->delay();
                    const Card *card = Sanguosha->getCard(card_id);
                    if(card->getSuit() == Card::Spade){
                        SavageAssault *nanmam = new SavageAssault(Card::Spade, card->getNumber());
                        nanmam->setSkillName("zhubing");
                        nanmam->addSubcard(card);
                        CardUseStruct card_use;
                        card_use.card = nanmam;
                        card_use.from = pai;
                        room->useCard(card_use);
                        has_spade = true;
                    }else
                        room->obtainCard(pai, card_id);
                }
/*
                if(has_spade)
                    room->playSkillEffect(objectName(), 2);
                else
                    room->playSkillEffect(objectName(), 3);
*/
                return true;
            }
        }
        return false;
    }
};

class Xujiu: public PhaseChangeSkill{
public:
    Xujiu():PhaseChangeSkill("xujiu"){
    }
    virtual bool onPhaseChange(ServerPlayer *qiong) const{
        if(qiong->getPhase() != Player::Start)
            return false;
        Room *room = qiong->getRoom();
        const Card *card = room->askForCard(qiong, ".black", "xujiu_ask", false, false);
        if(card){
            qiong->addToPile("niangA", card->getId());
        }
        return false;
    }
};

class BlackPattern: public CardPattern{
public:
    virtual bool match(const Player *player, const Card *card) const{
        return card->isBlack();
    }
};

class XujiuSlash: public TriggerSkill{
public:
    XujiuSlash():TriggerSkill("#xujiu_slash"){
        events << Predamage;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(!damage.card->inherits("Slash") || damage.from != player)
            return false;
        Room *room = player->getRoom();
        if(!player->getPile("niangA").isEmpty() && room->askForSkillInvoke(player, "xujiu", data)){
            int card_id = player->getPile("niangA").first();
            room->obtainCard(player, card_id);
            player->addToPile("niangB", card_id);

            LogMessage log;
            log.type = "#XujiuBuff";
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

class Xuebi: public PhaseChangeSkill{
public:
    Xuebi():PhaseChangeSkill("xuebi"){
        frequency = Compulsory;
    }
    virtual bool onPhaseChange(ServerPlayer *qiong) const{
        if(qiong->getPhase() != Player::Finish)
            return false;
        int x = qiong->getPile("niangA").length();
        int y = qiong->getPile("niangB").length();
        if(x + y > 3){
            Room *room = qiong->getRoom();
            for(int i = qMax(x, y); i > 0 ;i--){
                if(!qiong->getPile("niangA").isEmpty())
                    room->throwCard(qiong->getPile("niangA").first());
                if(!qiong->getPile("niangB").isEmpty())
                    room->throwCard(qiong->getPile("niangB").first());
            }
            qiong->loseSkill("niangA");
            qiong->loseSkill("niangB");
            LogMessage log;
            log.from = qiong;
            log.type = "#Xuebi";
            log.arg = objectName();
            log.arg2 = "niangA";
            room->sendLog(log);
            room->loseMaxHp(qiong);
        }
        return false;
    }
};

class Jiaochong: public PhaseChangeSkill{
public:
    Jiaochong():PhaseChangeSkill("jiaochong"){

    }
    virtual bool onPhaseChange(ServerPlayer *player) const{
        if(player->getPhase() == Player::Draw)
            player->tag["card_num"] = player->getHandcardNum();
        else if(player->getPhase() == Player::Play){
            Room *room = player->getRoom();
            int num = player->getHandcardNum() - player->tag.value("card_num").toInt();
            if(num > 0 && player->askForSkillInvoke(objectName())){
                QList<int> cards = player->handCards().mid(player->tag.value("card_num").toInt());
                for(int i = cards.length(); i >0; i--)
                    room->throwCard(cards.at(i-1));
                player->drawCards(num);
            }
        }
        return false;
    }
};

class GoulianViewAsSkill: public OneCardViewAsSkill{
public:
    GoulianViewAsSkill():OneCardViewAsSkill("goulian"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("GoulianCard");
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        GoulianCard *goulian_card = new GoulianCard;
        goulian_card->addSubcard(card_item->getCard()->getId());

        return goulian_card;
    }
};

GoulianCard::GoulianCard(){
    once = true;
}

bool GoulianCard::targetFilter(const QList<const Player *> &, const Player *to_select, const Player *Self) const{
    if(to_select->getMark("@goulian") > 0 || to_select->getMark("goulianA") > 0)
        return false;
    return to_select->getGeneral()->isMale();
}

void GoulianCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    QString result = room->askForChoice(effect.to, "goulian", "a+b");
    LogMessage log;
    log.from = effect.to;
    log.to << effect.from;
    if(result == "a"){
        log.type = "#GoulianA";
        room->sendLog(log);
        RecoverStruct recover;
        recover.who = effect.from;
        room->recover(effect.to, recover);
        effect.to->setMark("goulianA", 1);
    }
    else{
        log.type = "#GoulianB";
        room->sendLog(log);
        effect.to->drawCards(2);
        effect.to->gainMark("@goulian");
        effect.from->gainMark("@goulian");
    }
}

class Goulian: public TriggerSkill{
public:
    Goulian():TriggerSkill("goulian"){
        events << Predamaged << PhaseChange << DrawNCards;
        view_as_skill = new GoulianViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill("goulian") || target->getMark("@goulian") > 0;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        if(event == Predamaged && player->hasSkill("goulian")){
            DamageStruct damage = data.value<DamageStruct>();
            if(damage.to != player)
                return false;
            foreach(ServerPlayer *tmp, room->getOtherPlayers(player)){
                if(tmp->getMark("goulianA") > 0){
                    damage.to = tmp;
                    tmp->setMark("goulianA", 0);
                    //data = QVariant::fromValue();
                    LogMessage log;
                    log.type = "#GoulianAdamage";
                    log.from = tmp;
                    log.to << player;
                    room->sendLog(log);

                    DamageStruct damage2 = damage;
                    room->damage(damage2);
                    return true;
                }
            }
        }
        else if(event == PhaseChange){
            if(player->getPhase() == Player::Draw && !player->hasSkill("goulian") && player->getMark("@goulian") > 0){
                player->loseMark("@goulian");
                return true;
            }
            else if(player->getPhase() == Player::Start && player->hasSkill("goulian")){
                foreach(ServerPlayer *tmp, room->getOtherPlayers(player)){
                    if(tmp->getMark("goulianA") > 0)
                        tmp->setMark("goulianA", 0);
                    if(tmp->getMark("@goulian") > 0)
                        tmp->loseMark("@goulian");
                }
            }
        }
        else if(event == DrawNCards){
            if(player->hasSkill("goulian") && player->getMark("@goulian") > 0){
                LogMessage log;
                log.type = "#GoulianBdraw";
                log.arg = objectName();
                log.from = player;
                room->sendLog(log);

                player->loseMark("@goulian");
                data = data.toInt() + 1;
            }
        }
        return false;
    }
};

RedPackage::RedPackage()
    :Package("red")
{
    General *redzhonghui = new General(this, "redzhonghui", "wei");
    redzhonghui->addSkill(new Tongmou);
    redzhonghui->addSkill(new TongmouClear);
    redzhonghui->addSkill(new TongmouForbidden);
    redzhonghui->addSkill(new Xianhai);
    related_skills.insertMulti("tongmou", "#tmc");
    related_skills.insertMulti("tongmou", "#tmf");
    skills << new TongmouViewAsSkill;

    General *redxunyou = new General(this, "redxunyou", "wei", 3);
    redxunyou->addSkill(new Baichu);

    General *redhejin = new General(this, "redhejin", "qun", 4);
    redhejin->addSkill(new Tonglu);
    redhejin->addSkill(new Liehou);
    redhejin->addSkill(new Zide);
/*
何进 群 4体力
【同戮】出牌阶段，你可以令场上武将牌正面朝上的角色依次选择是否愿意将自己的武将牌翻面。若如此做，你的下一张【杀】造成的伤害+X。X为愿意翻面的武将数量
【列侯】弃牌阶段弃牌后，你可选择一个背面向上的角色，令其获得你弃掉的牌或摸一张牌
【自得】锁定技，回合结束阶段，若你在本回合内没有发动“同戮”，立即摸一张牌
*/

    General *redguansuo = new General(this, "redguansuo", "shu", 3);
    redguansuo->addSkill(new Xiefang);
    redguansuo->addSkill(new Yanyun);

    General *redyanbaihu = new General(this, "redyanbaihu", "wu");
    redyanbaihu->addSkill(new Jielue);

    General *redwutugu = new General(this, "redwutugu", "shu", 3);
    redwutugu->addSkill(new Chuzhen);
    skills << new Linjia << new Zhubing;

    General *redchunyuqiong = new General(this, "redchunyuqiong", "qun");
    redchunyuqiong->addSkill(new Xujiu);
    redchunyuqiong->addSkill(new XujiuSlash);
    redchunyuqiong->addSkill(new Xuebi);
    related_skills.insertMulti("xujiu", "#xujiu_slash");
    patterns[".black"] = new BlackPattern;

    General *redsunluban = new General(this, "redsunluban", "wu", 3, false);
    redsunluban->addSkill(new Jiaochong);
    redsunluban->addSkill(new Goulian);

    addMetaObject<TongmouCard>();
    addMetaObject<XianhaiCard>();
    addMetaObject<BaichuCard>();
    addMetaObject<TongluCard>();
    addMetaObject<XiefangCard>();
    addMetaObject<GoulianCard>();
}

ADD_PACKAGE(Red)
