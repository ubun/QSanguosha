#include "OEhuangjin-scenario.h"
#include "scenario.h"
#include "skill.h"
#include "clientplayer.h"
#include "client.h"
#include "engine.h"
#include "carditem.h"
#include "standard.h"


class zhangjiao_OEhuangjin:public OneCardViewAsSkill{
public:
    zhangjiao_OEhuangjin():OneCardViewAsSkill("zhangjiao_OEhuangjin"){

    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return (to_select->getCard()->getSuit() == Card::Diamond) && !to_select->isEquipped();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getCard();
        Jink *jink = new Jink(card->getSuit(), card->getNumber());
        jink->setSkillName(objectName());
        jink->addSubcard(card->getId());
        return jink;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern == "jink";
    }
};

class HuangJinJiTan: public FilterSkill{
public:
    HuangJinJiTan():FilterSkill("HuangJinJiTan"){

    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped() && to_select->getCard()->objectName() == "savage_assault";
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *c = card_item->getCard();
        ArcheryAttack *archeryattack = new ArcheryAttack(c->getSuit(), c->getNumber());
        archeryattack->setSkillName(objectName());
        archeryattack->addSubcard(card_item->getCard());

        return archeryattack;
    }
};

class OEhuangjinRule: public ScenarioRule{
public:
    OEhuangjinRule(Scenario *scenario)
        :ScenarioRule(scenario)
    {
        events << GameStart << Death << TurnStart << Predamaged;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();

        switch(event){
        case GameStart:{
                room->acquireSkill(player, "HuangJinJiTan");
                if(player->isLord()){

                    ServerPlayer *zhangjiao = room->findPlayer("zhangjiao");
                    room->acquireSkill(zhangjiao, "zhangjiao_OEhuangjin");

                }
                break;
            }
            /*room->setCurrent(player);
                    room->getThread()->trigger(TurnStart, player);
                    room->setCurrent(liushan);*/
            /*if(player->getGeneralName() == "sp_pangde" &&*/
        case TurnStart:{
                /*
            ServerPlayer *OEgodzhangliao = room->findPlayer("OEgodzhangliao");

            if(!player->isLord()){
                OEgodzhangliao->gainMark("@qiaoxiao");
                if(OEgodzhangliao->getMark("@qiaoxiao")>=3){
                    room->setCurrent(OEgodzhangliao);
                    room->getThread()->trigger(TurnStart, OEgodzhangliao);
                    room->setCurrent(player);}
                }
            if(player->isLord()){
                if(OEgodzhangliao->getMark("@qiaoxiao")>=2){
                    OEgodzhangliao->loseMark("@qiaoxiao",2);
                }
                else
                    player->turnOver();
                }
                */
            break;
            }

        case Predamaged:{
                /*
                ServerPlayer *zhoutai = room->findPlayer("zhoutai");
                ServerPlayer *OEsunquanXYJ = room->findPlayer("OEsunquanXYJ");
                DamageStruct damage = data.value<DamageStruct>();
                if(player->getGeneralName() == "OEsunquanXYJ" && player->getLostHp() >= 3){
                    RecoverStruct recover;
                    recover.who = OEsunquanXYJ;
                    recover.recover = damage.damage;
                    room->recover(OEsunquanXYJ, recover);
                    room->loseHp(zhoutai, damage.damage+1);
                }
                */
                break;
            }

        case Death:{

                /*
                ServerPlayer *OEgodzhangliao = room->findPlayer("OEgodzhangliao");
                ServerPlayer *OEsunquanXYJ = room->findPlayer("OEsunquanXYJ");
                ServerPlayer *zhangjiao = room->findPlayer("zhangjiao");
                ServerPlayer *lingtong = room->findPlayer("lingtong");
                DamageStar damage = data.value<DamageStar>();
                if(player->getGeneralName() == "taishici" && damage && damage->from && damage->from->isLord())
                {
                    OEgodzhangliao->drawCards(1, false);
                    if(OEsunquanXYJ)
                    {
                        RecoverStruct recover;
                        recover.who = OEsunquanXYJ;
                        recover.recover = 1;
                        room->recover(OEsunquanXYJ, recover);
                    }
                }
                if(player->getRole() == "rebel")
                    {
                    lingtong->loseSkill("linggandebuhe");
                    zhangjiao->loseSkill("ganlingdebuhe");
                }
                if(damage && damage->from)
                {
                    damage = NULL;
                    data = QVariant::fromValue(damage);
                }
                */
                break;
            }

        default:
            break;
        }

        return false;
    }
};

OEhuangjinScenario::OEhuangjinScenario()
    :Scenario("OEhuangjin")
{
    lord = "zhangjiao";
    loyalists << "zuoci";
    rebels << "caocao" << "sunjian" << "OEtaoyuanxiongdi";
    renegades << "yuji";

    rule = new OEhuangjinRule(this);

    skills << new HuangJinJiTan;
    skills << new zhangjiao_OEhuangjin;


}


void OEhuangjinScenario::getRoles(char *roles) const{
    strcpy(roles, "ZCNFFF");
}

void OEhuangjinScenario::onTagSet(Room *room, const QString &key) const{

}

ADD_SCENARIO(OEhuangjin);


