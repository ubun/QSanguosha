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
    /*if(targets.length() >= 2)
        return false;*/
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
};

LiXianJiPackage::LiXianJiPackage()
    :Package("LiXianJipackage")
{

    General *XJzhonghui = new General(this, "XJzhonghui", "qun", 4);
    XJzhonghui->addSkill(new XJzhenggong);

    General *XJcaozhi = new General(this, "XJcaozhi", "wei", 3);
    XJcaozhi->addSkill(new XJwencai);
    XJcaozhi->addSkill(new XJfengliu);

    General *XJlidian = new General(this, "XJlidian", "wei", 4);
    /*XJlidian->addSkill(new XJchenjing);/*

    General *XJliuxie = new General(this, "XJliuxie", "qun", 3);
    XJliuxie->addSkill(new XJzhaoling);
    XJliuxie->addSkill(new XJhanwei);

    General *XJpanzhang = new General(this, "XJpanzhang", "wu", 4);
    XJpanzhang->addSkill(new XJjielve);

    General *XJlingtong = new General(this, "XJlingtong", "wu", 4);
    XJlingtong->addSkill(new XJyifen);

    General *XJjiangwei = new General(this, "XJjiangwei", "shu", 4);
    XJjiangwei->addSkill(new XJduwu);

    General *XJwenpin = new General(this, "XJwenpin", "qun", 4);
    XJwenpin->addSkill(new XJzhuitao);

    General *XJdengai = new General(this, "XJdengai", "wei", 4);
    XJdengai->addSkill(new XJmouhua);
    XJdengai->addSkill(new XJjixi);

    General *XJzhanghe = new General(this, "XJzhanghe", "wei", 4);
    XJzhanghe->addSkill(new XJqiaobian);

    General *XJpanfeng = new General(this, "XJpanfeng", "qun", 4);
    XJpanfeng->addSkill(new XJweiwu);

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
    XJliyan->addSkill(new XJsimou);

    General *XJliubiao = new General(this, "XJliubiao$", "qun", 3);
    XJliubiao->addSkill(new XJanju);
    XJliubiao->addSkill(new XJleye);
    XJliubiao->addSkill(new XJshuishou);

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
}

ADD_PACKAGE(LiXianJi);

