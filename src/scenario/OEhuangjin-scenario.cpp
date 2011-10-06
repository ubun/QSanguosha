#include "OEhuangjin-scenario.h"
#include "scenario.h"
#include "skill.h"
#include "clientplayer.h"
#include "client.h"
#include "engine.h"
#include "carditem.h"
#include "standard.h"

//OE TAOYUANDESANXIONGDI


class taoyuanjieyi: public GameStartSkill{
public:
    taoyuanjieyi():GameStartSkill("taoyuanjieyi"){

    }

    static void AcquireGenerals(ServerPlayer *OEtaoyuanxiongdi, int n){
        QStringList list = GetAvailableGenerals(OEtaoyuanxiongdi);
        qShuffle(list);

        QStringList acquired = list.mid(0, n);
        QVariantList taoyuanjieyis = OEtaoyuanxiongdi->tag["taoyuanjieyis"].toList();
        foreach(QString taoyuanjieyi, acquired){
            taoyuanjieyis << taoyuanjieyi;
                const General *general = Sanguosha->getGeneral(taoyuanjieyi);
                foreach(const TriggerSkill *skill, general->getTriggerSkills()){
                    OEtaoyuanxiongdi->getRoom()->getThread()->addTriggerSkill(skill);
                }
            }


        OEtaoyuanxiongdi->tag["taoyuanjieyis"] = taoyuanjieyis;

        OEtaoyuanxiongdi->invoke("animate", "taoyuanjieyi:" + acquired.join(":"));

        LogMessage log;
        log.type = "#Gettaoyuanjieyi";
        log.from = OEtaoyuanxiongdi;
        log.arg = QString::number(n);
        log.arg2 = QString::number(taoyuanjieyis.length());
        OEtaoyuanxiongdi->getRoom()->sendLog(log);
    }

    static QStringList GetAvailableGenerals(ServerPlayer *OEtaoyuanxiongdi){
        QSet<QString> all = Sanguosha->getLimitedGeneralNames().toSet();
        QSet<QString> taoyuanjieyi_set, room_set;
        QVariantList taoyuanjieyis = OEtaoyuanxiongdi->tag["taoyuanjieyis"].toList();
        foreach(QVariant taoyuanjieyi, taoyuanjieyis)
            taoyuanjieyi_set << taoyuanjieyi.toString();/*
        taoyuanjieyi_set << "liubei" << "guanyu" << "zhangfei";*/

        Room *room = OEtaoyuanxiongdi->getRoom();
        QList<const ServerPlayer *> players = room->findChildren<const ServerPlayer *>();
        foreach(const ServerPlayer *player, players){
            room_set << player->getGeneralName();
            if(player->getGeneral2())
                room_set << player->getGeneral2Name();
        }

        static QSet<QString> banned;
        if(banned.isEmpty()){
            banned << "OEtaoyuanxiongdi" << "OEtaoyuanxiongdif" << "guzhielai" << "dengshizai" << "caochong";
        }

        static QSet<QString> availble;
        if(availble.isEmpty()){
            availble << "liubei" << "guanyu" << "zhangfei";
        }

        return (/*all - banned*/ availble - taoyuanjieyi_set - room_set).toList();
    }

    static QString SelectSkill(ServerPlayer *OEtaoyuanxiongdi, bool acquire_instant = true){
        Room *room = OEtaoyuanxiongdi->getRoom();

        QString taoyuanjieyi_skill = OEtaoyuanxiongdi->tag["taoyuanjieyiSkill"].toString();
        if(!taoyuanjieyi_skill.isEmpty())
            room->detachSkillFromPlayer(OEtaoyuanxiongdi, taoyuanjieyi_skill);

        QVariantList taoyuanjieyis = OEtaoyuanxiongdi->tag["taoyuanjieyis"].toList();
        QStringList taoyuanjieyi_generals;
        foreach(QVariant taoyuanjieyi, taoyuanjieyis)
            taoyuanjieyi_generals << taoyuanjieyi.toString();

        QString general_name = room->askForGeneral(OEtaoyuanxiongdi, taoyuanjieyi_generals);
        const General *general = Sanguosha->getGeneral(general_name);
        QString kingdom = general->getKingdom();
        if(OEtaoyuanxiongdi->getKingdom() != kingdom){
            if(kingdom == "god")
                kingdom = room->askForKingdom(OEtaoyuanxiongdi);
            room->setPlayerProperty(OEtaoyuanxiongdi, "kingdom", kingdom);
        }
        if(OEtaoyuanxiongdi->getGeneral()->isMale() != general->isMale())
            room->setPlayerProperty(OEtaoyuanxiongdi, "general", general->isMale() ? "OEtaoyuanxiongdi" : "OEtaoyuanxiongdif");

        QStringList skill_names;
        foreach(const Skill *skill, general->getVisibleSkillList()){
            if(skill->isLordSkill() || skill->getFrequency() == Skill::Limited
               || skill->getFrequency() == Skill::Wake)
                continue;

            skill_names << skill->objectName();
        }

        if(skill_names.isEmpty())
            return QString();

        QString skill_name;
        if(skill_names.length() == 1)
            skill_name = skill_names.first();
        else
            skill_name = room->askForChoice(OEtaoyuanxiongdi, "taoyuanjieyi", skill_names.join("+"));

        OEtaoyuanxiongdi->tag["taoyuanjieyiSkill"] = skill_name;

        if(acquire_instant)
            room->acquireSkill(OEtaoyuanxiongdi, skill_name);

        return skill_name;
    }

    virtual void onGameStart(ServerPlayer *OEtaoyuanxiongdi) const{
        AcquireGenerals(OEtaoyuanxiongdi, 3);
        SelectSkill(OEtaoyuanxiongdi);
    }

    virtual QDialog *getDialog() const{
        static taoyuanjieyiDialog *dialog;

        if(dialog == NULL)
            dialog = new taoyuanjieyiDialog;

        return dialog;
    }
};

taoyuanjieyiDialog::taoyuanjieyiDialog()
{
    setWindowTitle(tr("Incarnation"));
}

void taoyuanjieyiDialog::popup(){
    QVariantList taoyuanjieyi_list = Self->tag["taoyuanjieyis"].toList();
    QList<const General *> taoyuanjieyis;
    foreach(QVariant taoyuanjieyi, taoyuanjieyi_list)
        taoyuanjieyis << Sanguosha->getGeneral(taoyuanjieyi.toString());

    fillGenerals(taoyuanjieyis);

    show();
}

class taoyuanjieyiBegin: public PhaseChangeSkill{
public:
    taoyuanjieyiBegin():PhaseChangeSkill("#taoyuanjieyi-begin"){

    }

    virtual int getPriority() const{
        return 3;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target) && target->getPhase() == Player::Start;
    }

    virtual bool onPhaseChange(ServerPlayer *OEtaoyuanxiongdi) const{
        QString skill_name = taoyuanjieyi::SelectSkill(OEtaoyuanxiongdi, false);
        OEtaoyuanxiongdi->getRoom()->acquireSkill(OEtaoyuanxiongdi, skill_name);

        return false;
    }
};

class taoyuanjieyiEnd: public PhaseChangeSkill{
public:
    taoyuanjieyiEnd():PhaseChangeSkill("#taoyuanjieyi-end"){

    }

    virtual int getPriority() const{
        return -2;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target) && target->getPhase() == Player::NotActive;
    }

    virtual bool onPhaseChange(ServerPlayer *OEtaoyuanxiongdi) const{
        taoyuanjieyi::SelectSkill(OEtaoyuanxiongdi);

        return false;
    }
};
//OE TAOYUANDESANXIONGDI END

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

    General *OEtaoyuanxiongdi = new General(this, 8011, "OEtaoyuanxiongdi", "qun", 4, true, true);
    OEtaoyuanxiongdi->addSkill(new taoyuanjieyi);
    OEtaoyuanxiongdi->addSkill(new taoyuanjieyiBegin);
    OEtaoyuanxiongdi->addSkill(new taoyuanjieyiEnd);
}


void OEhuangjinScenario::getRoles(char *roles) const{
    strcpy(roles, "ZCNFFF");
}

void OEhuangjinScenario::onTagSet(Room *room, const QString &key) const{

}

ADD_SCENARIO(OEhuangjin);


