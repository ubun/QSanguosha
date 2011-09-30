#include "OExiaoyaojin-scenario.h"
#include "scenario.h"
#include "skill.h"
#include "clientplayer.h"
#include "client.h"
#include "engine.h"
#include "carditem.h"
#include "standard.h"

class ganlingdebuhe: public TriggerSkill{

public:
    ganlingdebuhe():TriggerSkill("ganlingdebuhe"){
        events << HpRecover << CardUsed;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();

        if(event == HpRecover){
            RecoverStruct recover = data.value<RecoverStruct>();
            if(recover.who && recover.who->getGeneralName() == "lingtong"){
                room->loseHp(player,recover.recover);
            }
        }
        return false;
    }
};

class linggandebuhe: public TriggerSkill{

public:
    linggandebuhe():TriggerSkill("linggandebuhe"){
        events << HpRecover << CardUsed;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();

        if(event == HpRecover){
            RecoverStruct recover = data.value<RecoverStruct>();
            if(recover.who && recover.who->getGeneralName() == "OEganningXYJ"){
                room->loseHp(player,recover.recover);
            }
        }
        return false;
    }
};




class OEXiaoyaojinRule: public ScenarioRule{
public:
    OEXiaoyaojinRule(Scenario *scenario)
        :ScenarioRule(scenario)
    {
        events << GameStart << Death << TurnStart << Predamaged;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();

        switch(event){
        case GameStart:{
                if(player->isLord()){
                    player->drawCards(1, false);
                    player->gainMark("@qiaoxiao",2);

                    ServerPlayer *OEganningXYJ = room->findPlayer("OEganningXYJ");
                    room->acquireSkill(OEganningXYJ, "ganlingdebuhe");

                    ServerPlayer *lingtong = room->findPlayer("lingtong");
                    room->acquireSkill(lingtong, "linggandebuhe");
                    ServerPlayer *OEsunquanXYJ = room->findPlayer("OEsunquanXYJ");
                    OEsunquanXYJ->gainMark("@Mengjiangdeliqu");
                }
                break;
            }
            /*room->setCurrent(player);
                    room->getThread()->trigger(TurnStart, player);
                    room->setCurrent(liushan);*/
            /*if(player->getGeneralName() == "sp_pangde" &&*/
        case TurnStart:{
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
            break;
            }

        case Predamaged:{
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
                break;
            }

        case Death:{
                ServerPlayer *OEgodzhangliao = room->findPlayer("OEgodzhangliao");
                ServerPlayer *OEsunquanXYJ = room->findPlayer("OEsunquanXYJ");
                ServerPlayer *OEganningXYJ = room->findPlayer("OEganningXYJ");
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
                    OEganningXYJ->loseSkill("ganlingdebuhe");
                }
                if(/*player->getGeneralName() == "sp_pangde" &&
                   */damage && damage->from /*&& damage->from->isLord()*/)
                {
                    damage = NULL;
                    data = QVariant::fromValue(damage);
                }
                break;
            }

        default:
            break;
        }

        return false;
    }
};

OEXiaoyaojinScenario::OEXiaoyaojinScenario()
    :Scenario("OEXiaoyaojin")
{
    lord = "OEgodzhangliao";
    /*loyalists << "huatuo";*/
    rebels << "xusheng" << "lingtong" << "OEganningXYJ" << "zhoutai" << "taishici" << "OEsunquanXYJ";
    /*renegades << "lumeng";*/

    rule = new OEXiaoyaojinRule(this);

    /*skills << new zhoutai_sizhan;*/
    skills << new ganlingdebuhe;
    skills << new linggandebuhe;

}


void OEXiaoyaojinScenario::getRoles(char *roles) const{
    strcpy(roles, "ZFFFFFF");
}

void OEXiaoyaojinScenario::onTagSet(Room *room, const QString &key) const{

}

ADD_SCENARIO(OEXiaoyaojin);

