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

class Shuaijin: public TriggerSkill{
public:
    Shuaijin():TriggerSkill("shuaijin"){
        events << Damage;
        frequency = Frequent;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();

        if(damage.card && damage.card->isBlack() && player->askForSkillInvoke(objectName(), data)){
            Room *room = player->getRoom();
            player->drawCards(2);
            QList<int> yiji_cards = player->handCards().mid(player->getHandcardNum() - 2);
            while(room->askForYiji(player, yiji_cards));
        }
        return false;
    }
};

class Liufang: public TriggerSkill{
public:
    Liufang():TriggerSkill("liufang"){
        events << CardLost;
        frequency = Frequent;
    }

    virtual int getPriority() const{
        return -1;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target) && target->getPhase() == Player::NotActive;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        CardMoveStar move = data.value<CardMoveStar>();
        if((move->from_place == Player::Hand || move->from_place == Player::Equip)
            && move->to != player && player->askForSkillInvoke(objectName(), data)){
            JudgeStruct judge;
            judge.reason = objectName();
            judge.who = player;
            player->getRoom()->judge(judge);
            if(judge.card->isBlack() && !judge.card->inherits("EquipCard"))
                player->getRoom()->obtainCard(player, move->card_id);
        }
        return false;
    }
};

class Weighing: public TriggerSkill{
public:
    Weighing():TriggerSkill("weighing"){
        events << HandCardNumChange << HpChanged;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *cc, QVariant &) const{
        Room *room = cc->getRoom();
        if(room->getCurrent() && room->getCurrent() != cc){
           int handcard = cc->getHandcardNum();
           int hp = cc->getHp();
           if(handcard != hp){
               LogMessage log;
               log.type = "#TriggerSkill";
               log.from = cc;
               log.arg = objectName();
               room->sendLog(log);

               cc->setMark("cx", 1);
               if(handcard < hp)
                   cc->drawCards(hp - handcard);
               else
                   room->askForDiscard(cc, objectName(), handcard - hp);
               cc->setMark("cx", 0);
           }
        }
        return false;
    }
};

class Kuanhou:public MasochismSkill{
public:
    Kuanhou():MasochismSkill("kuanhou"){
    }

    virtual void onDamaged(ServerPlayer *cc, const DamageStruct &damage) const{
        Room *room = cc->getRoom();
        if(!cc->faceUp())
            return;
        QVariant data = QVariant::fromValue(damage);
        if(room->askForSkillInvoke(cc, objectName(), data)){
            cc->turnOver();
            damage.from->setMark("kuanhou", 1);
        }
    }
};

class KuanhouEffect:public PhaseChangeSkill{
public:
    KuanhouEffect():PhaseChangeSkill("#kuanhou_effect"){
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getMark("kuanhou") > 0;
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        Room *room = player->getRoom();
        ServerPlayer *cc = room->findPlayerBySkillName("kuanhou");
        if(cc && player->getPhase() == Player::NotActive){
            LogMessage log;
            log.type = "#Kuanhou";
            log.from = player;
            log.to << cc;
            room->sendLog(log);

            ServerPlayer *target = room->askForPlayerChosen(cc, room->getOtherPlayers(player), "kuanhou");
            player->setMark("kuanhou", 0);
            QList<Player::Phase> phases;
            phases << Player::Play;
            target->play(phases);
        }
        return false;
    }
};

class Hunren: public TriggerSkill{
public:
    Hunren():TriggerSkill("hunren"){
        events << CardLost << CardFinished;
        frequency = Compulsory;
    }

    virtual int getPriority() const{
        return -1;
    }

    static QString PuyuanBook(QString weapon){
        QMap<QString, QString> map;

        map["crossbow"] = "rende";
        map["double_sword"] = "jizhi";
        map["qinggang_sword"] = "juejing";

        return map.value(weapon, QString());
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        const Card *weapon;
        if(event == CardLost){
            CardMoveStar move = data.value<CardMoveStar>();
            if(move->from_place == Player::Equip){
                weapon = Sanguosha->getCard(move->card_id);
                if(weapon->inherits("Weapon")){
                    LogMessage log;
                    log.type = "#HunrenSkill";
                    log.from = player;
                    log.arg = objectName();
                    log.arg2 = PuyuanBook(weapon->objectName());
                    room->detachSkillFromPlayer(player, log.arg2);
                    room->sendLog(log);
                }
            }
            return false;
        }
        CardUseStruct card_use = data.value<CardUseStruct>();
        if(card_use.card->inherits("Weapon")){
            weapon = player->getWeapon();
            if(weapon){
                LogMessage log;
                log.type = "#TriggerSkill";
                log.from = player;
                log.arg = objectName();
                room->sendLog(log);
                room->acquireSkill(player, PuyuanBook(weapon->objectName()));
            }
        }
        return false;
    }
};

class Cuihuo: public TriggerSkill{
public:
    Cuihuo():TriggerSkill("cuihuo"){
        events << CardLost;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        CardMoveStar move = data.value<CardMoveStar>();
        if(move->from_place == Player::Equip){
            Room *room = player->getRoom();
            if(Sanguosha->getCard(move->card_id)->inherits("Weapon")){
                if(player->getPhase() == Player::Play && player->hasFlag("cuihuo"))
                    return false;
                LogMessage log;
                log.type = "#TriggerSkill";
                log.from = player;
                log.arg = objectName();
                room->sendLog(log);

                RecoverStruct recover;
                recover.who = player;
                room->recover(player, recover);
                if(player->getPhase() == Player::Play)
                    player->setFlags("cuihuo");
            }
        }
        return false;
    }
};

CyanPackage::CyanPackage()
    :Package("cyan")
{
    General *cyankongrong = new General(this, "cyankongrong", "qun");
    cyankongrong->addSkill(new Rangli);

    General *cyancaochong = new General(this, "cyancaochong", "wei", 3);
    cyancaochong->addSkill(new Weighing);
    cyancaochong->addSkill(new Kuanhou);
    cyancaochong->addSkill(new KuanhouEffect);
    related_skills.insertMulti("kuanhou", "#kuanhou_effect");

    General *cyanyufan = new General(this, "cyanyufan", "wu", 3);
    cyanyufan->addSkill(new Shuaijin);
    cyanyufan->addSkill(new Liufang);

    General *cyanzhangxiu = new General(this, "cyanzhangxiu$", "qun");
    cyanzhangxiu->addSkill(new Baiming);
    cyanzhangxiu->addSkill(new Junling);

    General *cyanpuyuan = new General(this, "cyanpuyuan", "shu");
    cyanpuyuan->addSkill(new Hunren);
    cyanpuyuan->addSkill(new Cuihuo);

    addMetaObject<JunlingCard>();
}

ADD_PACKAGE(Cyan)
