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

RedPackage::RedPackage()
    :Package("red")
{
    General *redzhonghui = new General(this, "redzhonghui", "wei");
    redzhonghui->addSkill(new Tongmou);
    redzhonghui->addSkill(new TongmouClear);
    redzhonghui->addSkill(new Xianhai);
    related_skills.insertMulti("tongmou", "#tmc");

    General *redxunyou = new General(this, "redxunyou", "wei", 3);
    redxunyou->addSkill(new Baichu);
    redxunyou->addSkill(new BaichuEffect);
    related_skills.insertMulti("baichu", "#baichueffect");

    General *redhejin = new General(this, "redhejin", "qun", 4);
    redhejin->addSkill(new Tonglu);
    redhejin->addSkill(new TongluSkill);
    redhejin->addSkill(new Liehou);
    related_skills.insertMulti("tonglu", "#tong_lu");

    skills << new TongmouViewAsSkill;;

    addMetaObject<TongmouCard>();
    addMetaObject<XianhaiCard>();
    addMetaObject<BaichuCard>();
    addMetaObject<TongluCard>();
}

ADD_PACKAGE(Red)
