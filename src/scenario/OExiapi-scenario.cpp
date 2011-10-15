#include "OExiapi-scenario.h"
#include "scenario.h"
#include "skill.h"
#include "clientplayer.h"
#include "client.h"
#include "engine.h"
#include "carditem.h"
#include "standard.h"

class OExiapiRule: public ScenarioRule{
public:
    OExiapiRule(Scenario *scenario)
        :ScenarioRule(scenario)
    {
        events << GameStart << Death << Damaged << Predamaged << GameOverJudge;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();

        switch(event){
        case GameStart:{
                if(player->isLord()){
                    room->acquireSkill(player, "baonue");
                    room->installEquip(player, "chitu");
                    room->installEquip(player, "halberd");
                }
                else if(player->getGeneralName() == "gaoshun"){
                    if(player->askForSkillInvoke("reselect"))
                        room->transfigure(player, "chengong", true);
                }
                break;
            }
        case Damaged:{

            break;
            }
        case Predamaged:{
                ServerPlayer *lubu = room->findPlayer("lubu");
                if(lubu->getMark("@xiapi_shuiyan")){
                    DamageStruct damage = data.value<DamageStruct>();
                    if(damage.nature == DamageStruct::Fire){
                        return true;
                    }
                    break;
                }
            }

        case GameOverJudge:{
                DamageStar damage = data.value<DamageStar>();
                if(player->isLord() && damage && damage->from && damage->from->getGeneralName() == "zhangliao" && player->getMark("@xiapi_baozou"))
                    room->gameOver("renegade");
                break;
            }

        case Death:{
                ServerPlayer *lubu = room->findPlayer("lubu");
                DamageStar damage = data.value<DamageStar>();
                if(player->getGeneralName() == "diaochan")
                {
                    room->acquireSkill(lubu, "mashu");
                    lubu->gainMark("@xiapi_baozou");
                    room->setPlayerProperty(lubu, "maxhp", lubu->getMaxHP() + 1);
                    RecoverStruct recover;
                    recover.who = lubu;
                    recover.recover = 1;
                    room->recover(lubu, recover);
                }
                if(player->getGeneralName() == "diaochan" &&
                   damage && damage->from && damage->from->getRole()== "rebel")
                {
                    damage->from->drawCards(3, false);
                }
                if(player->getGeneralName() == "chengong" ||player->getGeneralName() == "gaoshun")
                {
                    lubu->gainMark("@xiapi_shuiyan");
                    lubu->throwAllEquips();
                    room->loseHp(lubu,1);
                    ServerPlayer *diaochan = room->findPlayer("diaochan");
                    if(diaochan)
                        room->loseHp(diaochan,1);
                }
                if(player->isLord() && damage && damage->from && damage->from->getGeneralName() == "zhangliao")
                    room->gameOver("zhangliao");
            }

        default:
            break;
        }

        return false;
    }
};

OExiapiScenario::OExiapiScenario()
    :Scenario("OExiapi")
{
    lord = "lubu";
    loyalists << "gaoshun" << "diaochan";
    rebels << "caocao" << "xiahoudun" << "caoren" << "OEtaoyuanxiongdi";
    renegades << "zhangliao";

    rule = new OExiapiRule(this);

    //skills << new Lubu_Baonue;
}


void OExiapiScenario::getRoles(char *roles) const{
    strcpy(roles, "ZCCNFFFF");
}

void OExiapiScenario::onTagSet(Room *room, const QString &key) const{

}

ADD_SCENARIO(OExiapi);



