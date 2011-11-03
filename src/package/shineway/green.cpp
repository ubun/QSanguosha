#include "green.h"
#include "skill.h"
#include "client.h"
#include "engine.h"
#include "carditem.h"
#include "room.h"

class Yabian: public TriggerSkill{
public:
    Yabian():TriggerSkill("yabian"){
        events << Damage;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *yanpeng, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();

        if(damage.card && damage.card->inherits("Slash") &&
           damage.from == yanpeng && damage.to != yanpeng){
            Room *room = yanpeng->getRoom();
            if(room->askForSkillInvoke(yanpeng, objectName(), data)){
                if(damage.to->getGeneralName() == "shenzhugeliang"){
                    foreach(int card_id, damage.to->getPile("stars"))
                        yanpeng->addToPile("stars", card_id, false);
                }
                //some special generals
                QList<QString> marks;
                marks
                        << "@chaos" /*luanwu*/
                        << "@nirvana" /*niepan*/
                        << "@flame" /*yeyan*/
                        ;
                foreach(QString mark, marks){
                    if(damage.to->getMark(mark)){
                        yanpeng->gainMark(mark, damage.to->getMark(mark));
                        damage.to->loseMark(mark, damage.to->getMark(mark));
                    }
                }
                //some other marks
                QStringList inscrire;
                inscrire << damage.to->getGeneralName() << damage.to->getKingdom();
                //QString myname = damage.from->getGeneralName();
                //QString mykindom = damage.from->getKingdom();
                if(damage.to->isAlive()){
                    room->transfigure(damage.to, damage.from->getGeneralName(), false, false);
                    room->setPlayerProperty(damage.to, "kingdom", damage.from->getKingdom());
                }
                room->transfigure(damage.from, inscrire.first(), false, false);
                room->setPlayerProperty(damage.from, "kingdom", inscrire.last());
            }
        }
        return false;
    }
};

typedef Skill SkillClass;

YuanlvCard::YuanlvCard(){
    once = true;
}

YuanlvStruct::YuanlvStruct()
    :kingdom("wei"), generalA("guojia"), generalB("simayi"), maxhp(5), skills(NULL)
{
}

void YuanlvCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    ServerPlayer *target = targets.first();
    room->loseHp(source);
    QStringList skills;
    foreach(const SkillClass *skill, target->getVisibleSkillList()){
        QString skill_name = skill->objectName();
        if(skill_name == "spear" || skill_name == "axe")
            continue;

        skills << skill_name;
        room->detachSkillFromPlayer(target, skill_name);
    }
    YuanlvStruct yuanlv_data;
    yuanlv_data.kingdom = target->getKingdom();
    yuanlv_data.generalA = target->getGeneralName();
    yuanlv_data.maxhp = target->getMaxHP();
    QString to_transfigure = target->getGeneral()->isMale() ? "sujiang" : "sujiangf";
    //room->setPlayerProperty(target, "general", to_transfigure);
    room->transfigure(target, to_transfigure, false, false);
    room->setPlayerProperty(target, "maxhp", yuanlv_data.maxhp);
    if(target->getGeneral2()){
        yuanlv_data.generalB = target->getGeneral2Name();
        room->setPlayerProperty(target, "general2", to_transfigure);
    }
    room->setPlayerProperty(target, "kingdom", yuanlv_data.kingdom);
    yuanlv_data.skills = skills;
    target->tag["YuanlvStore"] = QVariant::fromValue(yuanlv_data);
    target->addMark("yuanlv_target");
}

class Yuanlv: public ZeroCardViewAsSkill{
public:
    Yuanlv(): ZeroCardViewAsSkill("yuanlv"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("YuanlvCard");
    }

    virtual const Card *viewAs() const{
        return new YuanlvCard;
    }
};

class YuanlvReset:public TriggerSkill{
public:
    YuanlvReset(): TriggerSkill("#yuanlv_clear"){
        events << PhaseChange;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getMark("yuanlv_target") > 0;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        if(player->getPhase() != Player::NotActive)
            return false;

        player->setMark("yuanlv_target", 0);
        Room *room = player->getRoom();
        YuanlvStruct yuanlv_data = player->tag.value("YuanlvStore").value<YuanlvStruct>();

        foreach(QString skill_name, yuanlv_data.skills){
            room->acquireSkill(player, skill_name);
        }
        room->setPlayerProperty(player, "general", yuanlv_data.generalA);
        if(player->getGeneral2()){
            room->setPlayerProperty(player, "general2", yuanlv_data.generalB);
        }
        room->setPlayerProperty(player, "kingdom", yuanlv_data.kingdom);

        player->tag["YuanlvStore"] = NULL;
        return false;
    }
};

class Zhongjian: public TriggerSkill{
public:
    Zhongjian(): TriggerSkill("zhongjian"){
        events << PhaseChange;
        frequency = Frequent;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        ServerPlayer *target;
        Room *room = player->getRoom();
        if(player->getPhase() == Player::Discard && room->askForSkillInvoke(player, objectName(), data)){
            target = room->askForPlayerChosen(player, room->getOtherPlayers(player), objectName());
            target->gainMark("@vy");
        }
        else if(player->getPhase() == Player::Start){
            foreach(target, player->getRoom()->getAllPlayers())
                if(target->getMark("@vy") > 0)
                    target->loseAllMarks("@vy");
        }
        return false;
    }
};
/*
class LastCardPattern: public CardPattern{
public:
    virtual bool match(const Player *player, const Card *card) const{
        const ServerPlayer *source = qobject_cast<const ServerPlayer *>(player);
        return card->getId() == source->handCards().last();
    }
};
*/
class ZhongjianTarget: public TriggerSkill{
public:
    ZhongjianTarget(): TriggerSkill("#zhongjian_target"){
        events << CardUsed << PreSkillInvoke;
        untriggerable_skill << "spear" << "eight_diagram";
    }

    bool untriggerSkill(QString skill_name) const{
        return untriggerable_skill.contains(skill_name);
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getRoom()->findPlayerBySkillName(objectName()) && target->getMark("@vy") > 0;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *jushou = room->findPlayerBySkillName(objectName());
        if(!jushou)
            return false;
        LogMessage log;
        if(event == CardUsed){
            CardUseStruct use = data.value<CardUseStruct>();
            if(use.card->getSkillName().isEmpty() || untriggerSkill(use.card->getSkillName()) || use.from == jushou)
                return false;
            log.arg2 = use.card->getSkillName();
            log.type = "#Zj_SkillCard";
        }
        else if(event == PreSkillInvoke){
            SkillInvokeStruct ski = data.value<SkillInvokeStruct>();
            if(!ski.invoked)
                return false;
            log.arg2 = ski.skillname;
            log.type = "#Zj_TriggerSkill";
        }

        log.from = jushou;
        log.to << player;
        log.arg = "zhongjian";
        room->sendLog(log);

        JudgeStruct judge;
        judge.who = jushou;
        judge.good = true;
        judge.reason = "zhongjian";
        room->judge(judge);

        if(judge.card->isRed())
            player->obtainCard(judge.card);
        else{
            jushou->obtainCard(judge.card);
            //room->askForUseCard(jushou, ".ZJ", "@zhongjian");
        }

        return false;
    }

private:
    QStringList untriggerable_skill;
};

class Diezhi: public TriggerSkill{
public:
    Diezhi():TriggerSkill("diezhi"){
        events << ToDrawNCards;
    }

    virtual bool triggerable(const ServerPlayer *player) const{;
        return player->getMark("Exception") == 0;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *kanze = room->findPlayerBySkillName(objectName());
        if(!kanze || room->getCurrent() == player)
            return false;
        DrawStruct draw_data = data.value<DrawStruct>();
        if(draw_data.draw > 0 && kanze->askForSkillInvoke(objectName())){
            room->askForDiscard(kanze, objectName(), 1, false, true);
            return true;
        }
        return false;
    }
};

GreenPackage::GreenPackage()
    :Package("green")
{
    General *greenyanpeng = new General(this, "greenyanpeng", "shu");
    greenyanpeng->addSkill(new Yabian);
    greenyanpeng->addSkill(new Diezhi);

    General *greenjushou = new General(this, "greenjushou", "qun", 3);
    greenjushou->addSkill(new Yuanlv);
    greenjushou->addSkill(new YuanlvReset);
    related_skills.insertMulti("yuanlv", "#yuanlv_clear");
    greenjushou->addSkill(new Zhongjian);
    greenjushou->addSkill(new ZhongjianTarget);
    related_skills.insertMulti("zhongjian", "#zhongjian_target");
    //patterns[".ZJ"] = new LastCardPattern;

    addMetaObject<YuanlvCard>();
}

ADD_PACKAGE(Green)
