#include "ghost.h"
#include "client.h"
#include "engine.h"
#include "carditem.h"
#include "settings.h"
#include "maneuvering.h"
class SuperJuejing: public TriggerSkill{
public:
    SuperJuejing():TriggerSkill("super_juejing"){
        events   <<GameStart << CardLost << PhaseChange ;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &) const{

        if(event == GameStart){
            player->setMark("SuperMan", 1);
            Self->setMark("SuperMan", 1);
            return false;
        }

        if(player->getPhase() == Player::Draw){
            QVariant draw_num = 0;
            player->getRoom()->getThread()->trigger(DrawNCards, player, draw_num);
            player->drawCards(0, false);

            return true;
        }



        else if(player->getHandcardNum()<4){
                    player->getRoom()->playSkillEffect(objectName());
                    player->drawCards(4-player->getHandcardNum());
                }
        return false;
    }
};

class Duojian: public TriggerSkill{
public:
    Duojian():TriggerSkill("duojian"){
        events   << PhaseChange ;
        frequency = Frequent;
    }
    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &) const{
        Room *room = player->getRoom();
        if(player->getPhase() == Player::Start){
            foreach(ServerPlayer *other, room->getOtherPlayers(player)){
                if(other->getWeapon() && other->getWeapon()->objectName() == "qinggang_sword"){
                    player->obtainCard(other->getWeapon());
                    return false;
                }
            }
            foreach(ServerPlayer *players, room->getAlivePlayers()){
                QList<const Card *> judges = players->getCards("j");
                if(judges.isEmpty())
                continue;
                foreach(const Card *judge, judges){
                      if(judge){
                          int judge_id = judge->getEffectiveId();
                          const Card *card = Sanguosha->getCard(judge_id);
                          if(card->inherits("QinggangSword")){
                              player->obtainCard(card);
                              return false;
                         }
                     }
                  }

            }
        }
        return false;
    }
};
GhostPackage::GhostPackage()
    :Package("ghost")
{
    General *yixueshenzhaoyun = new General(this, "yixueshenzhaoyun", "god", 1);
    yixueshenzhaoyun->addSkill(new SuperJuejing);
    yixueshenzhaoyun->addSkill(longhun);
    yixueshenzhaoyun->addSkill(new Duojian);
}

ADD_PACKAGE(Ghost)
