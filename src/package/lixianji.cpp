#include "lixianji.h"
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
#include "guandu-scenario.h"

class XJzhenggong: public TriggerSkill{
public:
    XJzhenggong():TriggerSkill("XJzhenggong"){
        events << Damaged;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return ! target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{

        Room *room = player->getRoom();
        ServerPlayer *XJzhonghui = room->findPlayerBySkillName(objectName());

        DamageStruct damage = data.value<DamageStruct>();
        if(damage.card == NULL || !damage.card->inherits("Slash") || damage.to->isDead())
            return false;

        if(XJzhonghui && !XJzhonghui->isNude() && XJzhonghui->askForSkillInvoke(objectName(), data)){
            room->askForDiscard(XJzhonghui, "XJzhenggong", 1, false, true);

            JudgeStruct judge;
            judge.pattern = QRegExp("(.*):(.*):(.*)");
            judge.good = true;
            judge.who = player;
            judge.reason = objectName();

            room->judge(judge);

            switch(judge.card->getSuit()){
            case Card::Heart:
            case Card::Diamond:{
                    XJzhonghui->drawCards(1);
                    break;
                }

            case Card::Club:
            case Card::Spade:{
                    room->askForUseCard(XJzhonghui, "@@smalltuxi", "@tuxi-card");
                    /*if(damage.from && damage.from->isAlive())
                        damage.from->turnOver();*/

                    break;
                }

            default:
                break;
            }
        }

        return false;
    }
};


class XJwencai: public TriggerSkill{
public:
    XJwencai():TriggerSkill("XJwencai"){
        events << FinishJudge;
        frequency = Frequent;
    }

    virtual int getPriority() const{
        return -1;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    QList<const Card *> getRed(const Card *card) const{
        QList<const Card *> reds;

        if(!card->isVirtualCard()){
            if(card->isRed())
                reds << card;

            return reds;
        }

        foreach(int card_id, card->getSubcards()){
            const Card *c = Sanguosha->getCard(card_id);
            if(c->isRed())
                reds << c;
        }

        return reds;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        QList<const Card *> reds;

        if(event == FinishJudge){
            JudgeStar judge = data.value<JudgeStar>();
            if(room->getCardPlace(judge->card->getEffectiveId()) == Player::DiscardedPile
               && judge->card->isRed())
               reds << judge->card;
        }

        if(reds.isEmpty())
            return false;

        ServerPlayer *XJcaozhi = room->findPlayerBySkillName(objectName());
        if(XJcaozhi && XJcaozhi->askForSkillInvoke(objectName(), data)){
            if(player->getGeneralName() == "zhenji")
                room->playSkillEffect("XJwencai", 2);
            else
                room->playSkillEffect("XJwencai", 1);

            foreach(const Card *club, reds)
                XJcaozhi->obtainCard(club);
        }

        return false;
    }
};


XJfengliuCard::XJfengliuCard(){
    mute = true;
}

bool XJfengliuCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(targets.length() >= 1)
        return false;
    return true;
}

void XJfengliuCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    effect.to->drawCards(2);
    room->setEmotion(effect.to, "good");
}

class XJfengliuViewAsSkill: public ZeroCardViewAsSkill{
public:
    XJfengliuViewAsSkill():ZeroCardViewAsSkill("xjfengliu"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "@@xjfengliu";
    }

    virtual const Card *viewAs() const{
        return new XJfengliuCard;
    }
};

class XJfengliu:public MasochismSkill{
public:
    XJfengliu():MasochismSkill("xjfengliu"){
        frequency = Frequent;
        view_as_skill = new XJfengliuViewAsSkill;
    }
    virtual void onDamaged(ServerPlayer *XJcaozhi, const DamageStruct &damage) const{
        Room *room = XJcaozhi->getRoom();

        if(!room->askForSkillInvoke(XJcaozhi, objectName()))
            return;

        room->playSkillEffect(objectName());

        /*int x = damage.damage, i;*/
        /*for(i=0; i<x; i++){*/

            JudgeStruct judge;
            judge.pattern = QRegExp("(.*):(.*):(.*)");
            judge.good = true;
            judge.who = XJcaozhi;
            judge.reason = objectName();

            room->judge(judge);

            switch(judge.card->getSuit()){
            case Card::Club:
            case Card::Spade:{
                    if(!room->askForUseCard(XJcaozhi, "@@xjfengliu", "@xjfengliu"))
                        break;
                }
            default:
                break;
            }
        /*}*/
    }
};

class XJchenjing:public MasochismSkill{
    XJchenjing():MasochismSkill("XJchenjing"){
        frequency = Frequent;
    }

    virtual void onDamaged(ServerPlayer *XJlidian, const DamageStruct &damage) const{
        Room *room = XJlidian->getRoom();
    }
};

class XJzhaoling: public SlashBuffSkill{
public:
    XJzhaoling():SlashBuffSkill("XJzhaoling"){
        frequency = Compulsory;
    }

    virtual bool buff(const SlashEffectStruct &effect) const{
        ServerPlayer *XJliuxie = effect.from;
        Room *room = XJliuxie->getRoom();
        if(effect.nature == 2 || effect.nature == 1)/*1:Fire 2:Thunder*/{
            if(1){
                room->playSkillEffect(objectName());
                room->slashResult(effect, NULL);
                if(!effect.to->isNude()){
                    int card_id = room->askForCardChosen(XJliuxie, effect.to, "h", objectName());
                    if(room->getCardPlace(card_id) == Player::Hand)
                        room->moveCardTo(Sanguosha->getCard(card_id), XJliuxie, Player::Hand, false);
                    else
                        room->obtainCard(XJliuxie, card_id);
                }
                return true;
            }
        }

        return false;
    }
};

class SavageAssaultAvoid: public TriggerSkill{
public:
    SavageAssaultAvoid(const QString &avoid_skill)
        :TriggerSkill("#sa_avoid_" + avoid_skill), avoid_skill(avoid_skill)
    {
        events << CardEffected;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        CardEffectStruct effect = data.value<CardEffectStruct>();
        if(effect.card->inherits("SavageAssault")){
            LogMessage log;
            log.type = "#SkillNullify";
            log.from = player;
            log.arg = avoid_skill;
            log.arg2 = "savage_assault";
            player->getRoom()->sendLog(log);

            return true;
        }else
            return false;
    }

private:
    QString avoid_skill;
};

class XJhanwei: public TriggerSkill{
public:
    XJhanwei():TriggerSkill("XJhanwei"){
        events << SlashEffected;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target);/* && target->getArmor() == NULL;*/
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        SlashEffectStruct effect = data.value<SlashEffectStruct>();

        if(effect.slash->isBlack() && effect.nature == 0){
            player->getRoom()->playSkillEffect(objectName());

            LogMessage log;
            log.type = "#SkillNullify";
            log.from = player;
            log.arg = objectName();
            log.arg2 = effect.slash->objectName();

            player->getRoom()->sendLog(log);

            return true;
        }

        return false;
    }
};

XJjielveCard::XJjielveCard(){
}

bool XJjielveCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(targets.length() >= 1)
        return false;
    return true;
}

void XJjielveCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();

    DamageStruct damage;
    damage.card = NULL;
    damage.from = effect.from;
    damage.to = effect.to;

    room->damage(damage);
}

class XJjielveViewAsSkill: public ZeroCardViewAsSkill{
public:
    XJjielveViewAsSkill():ZeroCardViewAsSkill("XJjielve"){

    }

    virtual bool isEnabledAtPlay(const Player *) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        if(player->hasUsed("XJjielveCard"))
            return false;
        return pattern == "@@XJjielve";
    }

    virtual const Card *viewAs() const{
        return new XJjielveCard;
    }
};

class XJjielve: public TriggerSkill{
public:
    XJjielve():TriggerSkill("XJjielve"){
        events << CardUsed;
        view_as_skill = new XJjielveViewAsSkill;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        if(player->hasUsed("XJjielveCard"))
            return false;
        return true;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *XJpanzhang, QVariant &data) const{
        const Card *card = NULL;
        if(event == CardUsed){
            CardUseStruct use = data.value<CardUseStruct>();
            card = use.card;
        }
        if(card == NULL)
            return false;

        if(XJpanzhang->hasUsed("XJjielveCard")) return false;
        if(card->inherits("Dismantlement") || (card->inherits("Snatch"))){
            if(XJpanzhang->askForSkillInvoke(objectName(), data)){
                Room *room = XJpanzhang->getRoom();
                room->askForUseCard(XJpanzhang, "@@XJjielve", "@XJjielve");
            }
        }
        return false;
    }
};

class XJyifen: public TriggerSkill{
public:
    XJyifen():TriggerSkill("XJyifen"){
        events << CardLost << CardLostDone;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *XJlingtong, QVariant &data) const{
        if(event == CardLost){
            CardMoveStar move = data.value<CardMoveStar>();
            if(move->from_place == Player::Equip || move->from_place == Player::Judging)
                XJlingtong->tag["InvokeXJyifen"] = true;
        }else if(event == CardLostDone && XJlingtong->tag.value("InvokeXJyifen", false).toBool()){
            XJlingtong->tag.remove("InvokeXJyifen");

            if(XJlingtong->getPhase() != Player::NotActive)
                return false;
            Room *room = XJlingtong->getRoom();
            room->playSkillEffect(objectName());
            QList<ServerPlayer *> players = room->getOtherPlayers(XJlingtong), targets;
            foreach(ServerPlayer *p, players){
                targets << p;
            }
            ServerPlayer *target = room->askForPlayerChosen(XJlingtong, targets, "XJyifen-damage");
            DamageStruct damage;
            damage.from = XJlingtong;
            damage.to = target;
            room->damage(damage);
        }
        return false;
    }
};

XJduwuCard::XJduwuCard(){
}

bool XJduwuCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    int a = Self->getMark("@duwu");
    if(targets.length() >= a)
        return false;
    if(to_select == Self)
        return false;
    if(to_select->isKongcheng() && to_select->hasSkill("kongcheng"))
        return false;
    return true;
}

void XJduwuCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();

    CardEffectStruct effect2;
    Slash *slash = new Slash(Card::NoSuit, 0);
    slash->setSkillName("XJduwu");
    effect2.card = slash;
    effect2.from = effect.from;
    effect2.to = effect.to;

    room->cardEffect(effect2);

    room->setEmotion(effect.to, "bad");
    room->setEmotion(effect.from, "good");
}

class XJduwuViewAsSkill: public ZeroCardViewAsSkill{
public:
    XJduwuViewAsSkill():ZeroCardViewAsSkill("XJduwu"){
    }

    virtual const Card *viewAs() const{
        return new XJduwuCard;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern == "@@XJduwu";
    }
};

class XJduwu: public PhaseChangeSkill{
public:
    XJduwu():PhaseChangeSkill("XJduwu"){
        view_as_skill = new XJduwuViewAsSkill;
    }

    virtual bool onPhaseChange(ServerPlayer *XJjiangwei) const{
        if(XJjiangwei->getPhase() == Player::Draw && XJjiangwei->isWounded()){
            Room *room = XJjiangwei->getRoom();
            if(room->askForSkillInvoke(XJjiangwei, objectName())){
                int x = XJjiangwei->getLostHp(), i;

                for(i=0; i<x; i++){
                    int card_id = room->drawCard();
                    //int card_num=0;
                    room->moveCardTo(Sanguosha->getCard(card_id), NULL, Player::Special, true);

                    room->getThread()->delay();

                    const Card *card = Sanguosha->getCard(card_id);
                    if(card->isBlack()){
                        //card_num++;
                        XJjiangwei->gainMark("@duwu");
                    }else
                        room->obtainCard(XJjiangwei, card_id);
                }                
                if(XJjiangwei->getMark("@duwu"))
                    room->askForUseCard(XJjiangwei, "@@XJduwu", "@XJduwuask");
                XJjiangwei->loseAllMarks("@duwu");
                return true;
            }
        }
        return false;
    }
};


class XJweiwu:public OneCardViewAsSkill{
public:
    XJweiwu():OneCardViewAsSkill("XJweiwu"){

    }

    virtual bool viewFilter(const CardItem *to_select) const{
        const Card *card = to_select->getFilteredCard();
        switch(ClientInstance->getStatus()){
        case Client::Responsing:{
                QString pattern = ClientInstance->getPattern();
                if(pattern == "jink" || pattern == "peach")
                    return card->inherits("Slash");
            }

        default:
            return false;
        }
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return (pattern.contains("peach") || pattern == "jink") && player->getPhase() == Player::NotActive;
    }

    virtual const Card *viewAs(CardItem *card_item) const{

        const Card *card = card_item->getCard();
        switch(ClientInstance->getStatus()){
        case Client::Responsing:{
                QString pattern = ClientInstance->getPattern();
                if(pattern == "jink"){
                    Jink *jink = new Jink(card->getSuit(), card->getNumber());
                    jink->setSkillName(objectName());
                    jink->addSubcard(card->getId());
                    return jink;
                }
                else if(pattern == "peach"){
                    const Card *first = card_item->getCard();
                    Peach *peach = new Peach(first->getSuit(), first->getNumber());
                    peach->addSubcard(first->getId());
                    peach->setSkillName(objectName());
                    return peach;
                }
            }
        default:
            return false;
        }
    }
};
class XJanju:public OneCardViewAsSkill{
public:
    XJanju():OneCardViewAsSkill("XJanju"){

    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getCard()->isRed();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getCard();
        switch(ClientInstance->getStatus()){
        case Client::Responsing:{
                QString pattern = ClientInstance->getPattern();
                if(pattern == "jink"){
                    Jink *jink = new Jink(card->getSuit(), card->getNumber());
                    jink->setSkillName(objectName());
                    jink->addSubcard(card->getId());
                    return jink;
                }
                else if(pattern == "nullification"){
                    const Card *first = card_item->getFilteredCard();
                    Card *ncard = new Nullification(first->getSuit(), first->getNumber());
                    ncard->addSubcard(first);
                    ncard->setSkillName("XJanju");
                    return ncard;
                }
            }
        default:
            return false;
        }
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern == "jink" || pattern == "nullification";
    }
};


class XJleye: public OneCardViewAsSkill{
public:
    XJleye():OneCardViewAsSkill("XJleye"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("ExNihilo");//(player->usedTimes("Dismantlement")<=1);
    }


    virtual bool viewFilter(const CardItem *to_select) const{
        const Card *card = to_select->getFilteredCard();
        return card->inherits("EquipCard");
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getFilteredCard();

        ExNihilo *shortage = new ExNihilo(card->getSuit(), card->getNumber());
        shortage->setSkillName(objectName());
        shortage->addSubcard(card);

        return shortage;
    }
};

class XJshuishou:public TriggerSkill{
public:
    XJshuishou():TriggerSkill("XJshuishou$"){
        //events << CardUsed << CardResponsed;
        //frequency = Frequent;
        events << PhaseChange;
        frequency = Frequent;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getKingdom() == "qun";
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{

        Room *room = player->getRoom();
        ServerPlayer *XJliubiao = room->findPlayerBySkillName(objectName());
        if(player->getPhase() == Player::Discard){
            player->tag["cardnum"] = player->getHandcardNum();
        }
        else if(player->getPhase() == Player::Finish){
            int drawnum = player->tag.value("cardnum", 0).toInt() - player->getHandcardNum();
            if(drawnum > 1 && player->askForSkillInvoke(objectName(), data)){
                    XJliubiao->drawCards(1);
            }
        }
        return false;
    }
};

LiXianJiPackage::LiXianJiPackage()
    :Package("LiXianJipackage")
{

    General *XJzhonghui = new General(this, "XJzhonghui", "qun", 4);
    XJzhonghui->addSkill(new XJzhenggong);

    General *XJcaozhi = new General(this, "XJcaozhi", "wei", 3);
    XJcaozhi->addSkill(new XJwencai);
    XJcaozhi->addSkill(new XJfengliu);

    /*General *XJlidian = new General(this, "XJlidian", "wei", 4);
    XJlidian->addSkill(new XJchenjing);*/

    General *XJliuxie = new General(this, "XJliuxie", "qun", 3);
    XJliuxie->addSkill(new SavageAssaultAvoid("XJhanwei"));
    XJliuxie->addSkill(new XJhanwei);
    XJliuxie->addSkill(new XJzhaoling);

    General *XJpanzhang = new General(this, "XJpanzhang", "wu", 4);
    XJpanzhang->addSkill(new XJjielve);

    General *XJlingtong = new General(this, "XJlingtong", "wu", 4);
    XJlingtong->addSkill(new XJyifen);

    General *XJjiangwei = new General(this, "XJjiangwei", "shu", 4);
    XJjiangwei->addSkill(new XJduwu);/*

    General *XJwenpin = new General(this, "XJwenpin", "qun", 4);
    XJwenpin->addSkill(new XJzhuitao);

    General *XJdengai = new General(this, "XJdengai", "wei", 4);
    XJdengai->addSkill(new XJmouhua);
    XJdengai->addSkill(new XJjixi);

    General *XJzhanghe = new General(this, "XJzhanghe", "wei", 4);
    XJzhanghe->addSkill(new XJqiaobian);*/

    General *XJpanfeng = new General(this, "XJpanfeng", "qun", 4);
    XJpanfeng->addSkill(new XJweiwu);/*

    General *XJyujin = new General(this, "XJyujin", "wei", 4);
    XJyujin->addSkill(new XJyizhong);

    General *XJfazheng = new General(this, "XJfazheng", "shu", 3);
    XJfazheng->addSkill(new XJzongheng);
    XJfazheng->addSkill(new XJliangmou);
    XJfazheng->addSkill(new XJzhiji);

    General *XJzhangren = new General(this, "XJzhangren", "qun", 4);
    XJzhangren->addSkill(new XJjiangchi);

    General *XJwangping = new General(this, "XJwangping", "shu", 4);
    XJwangping->addSkill(new XJyanzheng);

    General *XJliuzhang = new General(this, "XJliuzhang$", "qun", 3);
    XJliuzhang->addSkill(new XJkuanrou);
    XJliuzhang->addSkill(new XJguanwang);
    XJliuzhang->addSkill(new XJsijian);

    General *XJzhangchunhua = new General(this, "XJzhangchunhua", "wei", 3, false);
    XJzhangchunhua->addSkill(new XJfeishi);
    XJzhangchunhua->addSkill(new XJshuangren);

    General *XJdingfeng = new General(this, "XJdingfeng", "wu", 4);
    XJdingfeng->addSkill(new XJduanbing);

    General *XJliushan = new General(this, "XJliushan$", "shu", 3);
    XJliushan->addSkill(new XJhanzuo);
    XJliushan->addSkill(new XJanle);
    XJliushan->addSkill(new XJxiuyang);
    XJliushan->addSkill(new XJxunli);

    General *XJxusheng = new General(this, "XJxusheng", "wu", 4);
    XJxusheng->addSkill(new XJyicheng);

    General *XJmazhong = new General(this, "XJmazhong", "shu", 4);
    XJmazhong->addSkill(new XJpingluan);

    General *XJsunce = new General(this, "XJsunce$", "wu", 4);
    XJsunce->addSkill(new XJyingqi);
    XJsunce->addSkill(new XJjinxin);

    General *XJliyan = new General(this, "XJliyan", "shu", 3);
    XJliyan->addSkill(new XJshusong);
    XJliyan->addSkill(new XJsimou);*/

    General *XJliubiao = new General(this, "XJliubiao$", "qun", 3);
    XJliubiao->addSkill(new XJanju);
    XJliubiao->addSkill(new XJleye);
    XJliubiao->addSkill(new XJshuishou);/*

    General *XJgaoshun = new General(this, "XJgaoshun", "qun", 4);
    XJgaoshun->addSkill(new XJxianzhen);

    General *XJcaohong = new General(this, "XJcaohong", "wei", 4);
    XJcaohong->addSkill(new XJhuwei);

    General *XJmizhu = new General(this, "XJmizhu", "shu", 3);
    XJmizhu->addSkill(new XJwenhou);
    XJmizhu->addSkill(new XJzizhu);

    General *XJjiling = new General(this, "XJjiling", "qun", 4);
    XJjiling->addSkill(new XJwanqiang);

    General *XJchengpu = new General(this, "XJchengpu", "wu", 4);
    XJchengpu->addSkill(new XJchizhong);

    General *XJlejin = new General(this, "XJlejin", "wei", 4);
    XJlejin->addSkill(new XJxiandeng);
    XJlejin->addSkill(new XJxiaoyong);

    General *XJzhangzhao = new General(this, "XJzhangzhao", "wu", 3);
    XJzhangzhao->addSkill(new XJzhiyan);
    XJzhangzhao->addSkill(new XJbizhan);
    */

    addMetaObject<XJfengliuCard>();
    addMetaObject<XJjielveCard>();
    addMetaObject<XJduwuCard>();
    //addMetaObject<XJshuishouCard>();

}

ADD_PACKAGE(LiXianJi);

