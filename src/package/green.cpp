#include "green.h"
#include "skill.h"
#include "client.h"
#include "engine.h"
#include "carditem.h"
#include "room.h"
//#include "maneuvering.h"

class Rangli: public TriggerSkill{
public:
    Rangli():TriggerSkill("rangli"){
        events << PhaseChange << DrawNCards;
    }
    virtual bool triggerable(const ServerPlayer *player) const{;
        return player->hasSkill(objectName()) || player->getMark("@pear") > 0;
    }
    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        if(event == DrawNCards){
            int pearnum = player->getMark("@pear");
            data = data.toInt() + pearnum * 2;
            player->loseAllMarks("@pear");
            return false;
        }
        if(player->getMark("@pear") > 0 || player->getPhase() != Player::Draw)
            return false;
        if(player->askForSkillInvoke(objectName())){
            QList<ServerPlayer *> players;
            foreach(ServerPlayer *tmp, room->getAlivePlayers()){
                if(tmp->getHandcardNum() < 2){
                    if(tmp->hasSkill("lianying"))
                        players << tmp;
                    else if(tmp->hasSkill("shangshi") && tmp->isWounded())
                        players << tmp;
                    else
                        continue;
                }
                else
                    players << tmp;
            }
            if(players.isEmpty())
                return false;
            ServerPlayer *target = room->askForPlayerChosen(player, players, objectName());
            target->gainMark("@pear");
            player->obtainCard(room->askForCardShow(target, player, objectName()));
            player->obtainCard(room->askForCardShow(target, player, objectName()));
            return true;
        }
        return false;
    }
};

GreenPackage::GreenPackage()
    :Package("green")
{
    General *greenkongrong = new General(this, "greenkongrong", "qun");
    greenkongrong->addSkill(new Rangli);

}

ADD_PACKAGE(Green)
