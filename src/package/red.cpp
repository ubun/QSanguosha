#include "red.h"
#include "skill.h"
#include "client.h"
#include "engine.h"
#include "carditem.h"
#include "settings.h"
#include "room.h"
#include "maneuvering.h"

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

    foreach(int card_id, zhonghui->handCards()){
        zhonghui->addToPile("mycard", card_id, false);
    }
    foreach(int card_id, gay->handCards()){
        room->moveCardTo(Sanguosha->getCard(card_id), zhonghui, Player::Hand, false);
    }
    foreach(int card_id, zhonghui->getPile("mycard")){
        room->moveCardTo(Sanguosha->getCard(card_id), gay, Player::Hand, false);
    }

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
        return player->hasSkill(objectName());/*player->getMark("xoxo") > 0;*/
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
                foreach(int card_id, player->handCards()){
                    player->addToPile("mycard", card_id, false);
                }
                foreach(int card_id, gay->handCards()){
                    room->moveCardTo(Sanguosha->getCard(card_id), player, Player::Hand, false);
                }
                foreach(int card_id, player->getPile("mycard")){
                    room->moveCardTo(Sanguosha->getCard(card_id), gay, Player::Hand, false);
                }
            }
        }
        if(player == zhonghui && player->getPhase() == Player::Play){
            if(!player->askForSkillInvoke(objectName())){
                zhonghui->loseMark("xoxo",1);
                return false;
            }
            QList<ServerPlayer *> players;
            foreach(ServerPlayer *p, room->getOtherPlayers(zhonghui)){
                if(p->hasSkill("lianying") || p->hasSkill("tuntian")
                || p->hasSkill("shangshi") || p->hasSkill("beifa"))
                    continue;
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
            player->loseMark("xoxo", player->getMark("xoxo"));
            zhonghui->loseMark("xoxo", zhonghui->getMark("xoxo"));
            room->detachSkillFromPlayer(player, "tongmouv");
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
            player->loseMark("xoxo", player->getMark("xoxo"));
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

class BaichuEffect: public TriggerSkill{
public:
    BaichuEffect():TriggerSkill("#baichueffect"){
        events << TurnStart << PhaseChange << CardUsed;
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
            const Card *card = room->askForCard(player, ".", "@baichu", false);
            player->addToPile("ji", card->getId());
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

class Baichu:public OneCardViewAsSkill{
public:
    Baichu():OneCardViewAsSkill("baichu"){
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

class TongluSkill: public TriggerSkill{
public:
    TongluSkill():TriggerSkill("#tong_lu"){
        events << SlashMissed << Predamage;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        if(event == SlashMissed){
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

class Tonglu: public ZeroCardViewAsSkill{
public:
    Tonglu():ZeroCardViewAsSkill("tonglu"){}
    virtual const Card *viewAs() const{
        return new TongluCard;
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
        if(tmp->getState() == "robot"){
            if(tmp->getRole() == source->getRole())
                result = "agree";
            else if(source->getRole() == "lord" && tmp->getRole() == "loyalist")
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

class Liehou: public PhaseChangeSkill{
public:
    Liehou():PhaseChangeSkill("liehou"){
        frequency = Compulsory;
    }
    virtual bool onPhaseChange(ServerPlayer *player) const{
        if(player->getPhase() == Player::Finish && !player->hasUsed("TongluCard")){
            LogMessage log;
            log.type = "#Liehou";
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
    XiefangViewAsSkill():ZeroCardViewAsSkill("chuanqi"){

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
        if(!player->askForSkillInvoke(objectName(), data))
            return false;
        const Card *card = room->askForCard(player, "slash", "yanyun-slash");
        if(card && card->getSkillName() != "xiefang"){
            if(player->hasFlag("drank"))
                room->setPlayerFlag(player, "-drank");

            CardUseStruct use;
            use.card = card;
            use.from = player;
            use.to << effect.to;
            room->useCard(use, false);
        }

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
        const Card *card = room->askForCard(qiong, ".black", "xujiu_ask");
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
            player->obtainCard(Sanguosha->getCard(card_id));
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

bool GoulianCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
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
                data = data.toInt() + 2;
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
    redzhonghui->addSkill(new Xianhai);
    related_skills.insertMulti("tongmou", "#tmc");
    skills << new TongmouViewAsSkill;

    General *redxunyou = new General(this, "redxunyou", "wei", 3);
    redxunyou->addSkill(new Baichu);
    redxunyou->addSkill(new BaichuEffect);
    related_skills.insertMulti("baichu", "#baichueffect");

    General *redhejin = new General(this, "redhejin", "qun", 4);
    redhejin->addSkill(new Tonglu);
    redhejin->addSkill(new TongluSkill);
    redhejin->addSkill(new Liehou);
    related_skills.insertMulti("tonglu", "#tong_lu");
/*
何进 群 4体力
【同戮】出牌阶段，你可以令场上武将牌正面朝上的角色选择是否将其武将牌翻面。若如此做，你的下一张【杀】造成的伤害+X。X为选择翻面的武将数。
【列侯】在你的弃牌阶段，你可选择一个背面向上的角色，将需要弃置的牌直接给予此角色，或让该角色摸一张牌。
【自得】锁定技，若你没有发动【同戮】，回合结束时摸一张牌。
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
