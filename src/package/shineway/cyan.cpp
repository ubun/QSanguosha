#include "cyan.h"
#include "skill.h"
#include "client.h"
#include "engine.h"
#include "carditem.h"
#include "room.h"

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
        if(player->askForSkillInvoke(objectName(), data)){
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

class Baiming: public TriggerSkill{
public:
    Baiming():TriggerSkill("baiming"){
        events << Predamage;
        frequency = Compulsory;
    }

    virtual int getPriority() const{
        return 2;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        DamageStruct damage = data.value<DamageStruct>();
        const Card *reason = damage.card;
        if(reason && reason->inherits("Slash") && player->getMark("baiming") == 0){
            LogMessage log;
            log.type = "#Baiming";
            log.from = player;
            log.to << damage.to;
            log.arg = objectName();
            room->sendLog(log);

            QList<ServerPlayer *> players;
            foreach(ServerPlayer *tmp, room->getAllPlayers()){
                if(player->inMyAttackRange(tmp) && tmp != damage.to)
                    players << tmp;
            }
            if(players.isEmpty())
                players << player;
            damage.to = room->askForPlayerChosen(player, players, objectName());
            log.type = "#BaimingRechoose";
            log.to.clear();
            log.to << damage.to;
            room->sendLog(log);

            player->setMark("baiming", 1);
            room->damage(damage);
            player->setMark("baiming", 0);
            return true;
        }
        return false;
    }
};

JunlingCard::JunlingCard(){
    once = true;
}

bool JunlingCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    return targets.length() == 2;
}

bool JunlingCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(targets.isEmpty()){
        return true;
    }else if(targets.length() == 1){
        const Player *first = targets.first();
        return first->inMyAttackRange(to_select);
    }else
        return false;
}

void JunlingCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    ServerPlayer *killer = targets.at(0);
    ServerPlayer *victim = targets.at(1);
    QString prompt = QString("junling-slash:%1:%2")
                         .arg(source->objectName()).arg(victim->objectName());
    const Card *slash = room->askForCard(killer, "slash", prompt);
    if(slash){
        CardUseStruct use;
        use.card = slash;
        use.from = killer;
        use.to << victim;
        room->useCard(use);
    }else{
        if(!killer->isKongcheng()){
            int card_id = room->askForCardChosen(source, killer, "h", "junling");
            room->moveCardTo(Sanguosha->getCard(card_id), source, Player::Hand, false);
        }
    }
}

class Junling:public ZeroCardViewAsSkill{
public:
    Junling():ZeroCardViewAsSkill("junling$"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("JunlingCard");
    }

    virtual const Card *viewAs() const{
        return new JunlingCard;
    }
};

CyanPackage::CyanPackage()
    :Package("cyan")
{
    General *cyankongrong = new General(this, "cyankongrong", "qun");
    cyankongrong->addSkill(new Rangli);

    General *cyanzhangxiu = new General(this, "cyanzhangxiu$", "qun");
    cyanzhangxiu->addSkill(new Baiming);
    cyanzhangxiu->addSkill(new Junling);

    addMetaObject<JunlingCard>();
}

ADD_PACKAGE(Cyan)
