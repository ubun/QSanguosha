#include "OEhulao-scenario.h"
#include "scenario.h"
#include "skill.h"
#include "clientplayer.h"
#include "client.h"
#include "engine.h"
#include "carditem.h"
#include "standard.h"


class OEhulaoRule: public ScenarioRule{
public:
    OEhulaoRule(Scenario *scenario)
        :ScenarioRule(scenario)
    {
        events << GameStart << Death << TurnStart << Predamaged;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();

        switch(event){
        case GameStart:{

                if(player->isLord()){

                    ServerPlayer *lubu = room->findPlayer("lubu");
                    room->acquireSkill(lubu, "Hulaohalberd");
                    ServerPlayer *yuanshao = room->findPlayer("yuanshao");
                    room->acquireSkill(yuanshao,"OEHulaoxueyi");
                    /*yitianjian->addSkill(new Skill("zhengfeng", Skill::Compulsory));*/
                }else if(player->getGeneralName() == "sunjian"){
                    if(player->askForSkillInvoke("reselect"))
                        room->transfigure(player, "caocao", true);
                }else if(player->getGeneralName() == "yuanshu"){
                    if(player->askForSkillInvoke("reselect"))
                        room->transfigure(player, "gongsunzan", true);
                }
                break;
            }
        case TurnStart:{
            break;
            }

        case Predamaged:{
                break;
            }

        case Death:{
                break;
            }

        default:
            break;
        }

        return false;
    }
};

OEhulaoScenario::OEhulaoScenario()
    :Scenario("OEhulao")
{
    lord = "dongzhuo";
    loyalists << "lubu" << "jiaxu" ;
    rebels << "OEtaoyuanxiongdi" << "sunjian" << "yuanshu" << "yuanshao";
    renegades << "diaochan";

    rule = new OEhulaoRule(this);

    skills << new Skill("Hulaohalberd", Skill::Compulsory);
    skills << new Skill("OEHulaoxueyi", Skill::Compulsory);

}


void OEhulaoScenario::getRoles(char *roles) const{
    strcpy(roles, "ZCCNFFFF");
}

void OEhulaoScenario::onTagSet(Room *room, const QString &key) const{/*
    ServerPlayer *yuanshao = room->findPlayer("yuanshao");
    if(yuanshao)
        room->setPlayerProperty(yuanshao, "xueyi", 8);*/
}

ADD_SCENARIO(OEhulao);



