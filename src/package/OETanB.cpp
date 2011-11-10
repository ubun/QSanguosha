#include "OETanB.h"
#include "standard-skillcards.h"
#include "general.h"
#include "skill.h"
#include "engine.h"
#include "standard.h"
#include "carditem.h"
#include "generaloverview.h"
#include "clientplayer.h"
#include "client.h"
#include "maneuvering.h"
#include "room.h"
#include "ai.h"

/*OE3126 edado-���ԣ���̬*/
/*���ԣ�
  �غϿ�ʼ�׶Σ���ɽ���һ���ж���
  ��Ϊ���ң���ظ�1��������
  ��Ϊ���ң���á����塿ƴ��ʤ��Ч��ֱ���غϽ�����
  ��Ϊ��Ƭ����ɺ�����һ����ɫƴ�㣬����Ӯ�����ȡ�ý�ɫһ�����ƣ�����ȡ������
  ��Ϊ÷�������á����¡�Ч��ֱ���غϽ�����
  Ȼ���ж��Ʒ�������佫���ϣ��Ƴ���Ϸ��

  �ĳ���������´���͵������������µĴ��붼��������
  ��Ƭ��Ч����Ϊ��ѡ����Ȼ�����û�����ƻ��������˶�û��������ô�죿ע�����������ƣ����ȷ��Ҳ���·�����
  �ж��Ʒ����佫������Ϊ����̬�ķ�������Ȼ��üǵ����г�������ͬ��ɫ�ƣ���ɱ�����ԣ����ﲻ�ܽ��ܣ�*/
GuayanCard::GuayanCard(){
    will_throw = false;
}

bool GuayanCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;

    if(to_select == Self)
        return false;

    return !to_select->isKongcheng();
}

void GuayanCard::use(Room *room, ServerPlayer *OEedado, const QList<ServerPlayer *> &targets) const{
    ServerPlayer *tiger = targets.first();//�������������²���
    if(tiger->isKongcheng())
        return;

    //room->playSkillEffect("guayan", 1);

    bool success = OEedado->pindian(tiger, "guayan");
    if(success){
        //room->playSkillEffect("guayan", 2);
        int wolfnum = tiger->getHandcardNum();int i;
        for(i = 0; i < (wolfnum+1)/2; i++){
            room->moveCardTo(Sanguosha->getCard(room->askForCardChosen(OEedado, tiger, "h", "tuxi"))
                         , OEedado, Player::Hand, false);
        }
    }
}

class GuayanViewAsSkill: public ZeroCardViewAsSkill{
public:
    GuayanViewAsSkill():ZeroCardViewAsSkill("guayan"){
    }

    virtual const Card *viewAs() const{
        return new GuayanCard;
    }

//protected:
    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern == "@@guayan";
    }
};

class Guayan:public TriggerSkill{
public:
    Guayan():TriggerSkill("guayan"){
        view_as_skill = new GuayanViewAsSkill;

        events << PhaseChange << FinishJudge;
        frequency = Frequent;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *OEedado, QVariant &data) const{
        Room *room = OEedado->getRoom();
        if(event == PhaseChange && OEedado->getPhase() == Player::Judge){
            if(OEedado->askForSkillInvoke("guayan")){
                //room->playSkillEffect(objectName());

                JudgeStruct judge;
                judge.pattern = QRegExp("(.*):(.*):(.*)");
                judge.good = true;
                judge.reason = objectName();
                judge.who = OEedado;

                room->judge(judge);

                switch(judge.card->getSuit()){
                case Card::Heart:{
                        RecoverStruct recover;
                        recover.who = OEedado;
                        room->recover(OEedado, recover);
                        break;
                    }

                case Card::Diamond:{
                        if(OEedado->isKongcheng())
                            return false;
                        room->askForUseCard(OEedado, "@@guayan", "@guayan");
                        break;
                    }

                case Card::Club:{
                        OEedado->setFlags("luoyi");
                        break;
                    }

                case Card::Spade:{
                        room->setPlayerFlag(OEedado, "tianyi_success");
                        break;
                    }

                default:
                    break;
                }
                return false;
            }
        }else if(event == PhaseChange && OEedado->getPhase() == Player::Finish){
            if(OEedado->hasFlag("tianyi_success")){
                room->setPlayerFlag(OEedado, "-tianyi_success");
            }
        }else if(event == FinishJudge){
            JudgeStar judge = data.value<JudgeStar>();
            if(judge->reason == "guayan" && judge->isGood() && !(OEedado->hasSkill("tiandu")) ){
                OEedado->addToPile("guayanpile", judge->card->getEffectiveId());
                return true;
            }
        }
        return false;
    }
};

/*��̬�����Ѽ����غϿ�ʼ�׶Σ�����ӵ������ͬ��ɫ�����ԡ��ƣ����1���������ޣ������û�ü��ܡ���ʡ���MOD����
  ��˵�������������˺ü��졣����
  2Ѫ�Ӹ�Ӣ��߯�࣬�������ּ��˸���MOD����Χ��Ⱥ�ڣ��������أ�����������ʵ������ӡ������ģ�Ԥ�ϵ���������BUG��������*/
class Nitai: public PhaseChangeSkill{
public:
    Nitai():PhaseChangeSkill("nitai"){
        frequency = Wake;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return (PhaseChangeSkill::triggerable(target)
            && target->getMark("nitai") == 0
            && target->getPhase() == Player::Start);
            //target->getPile("guayanp").length() >= 3;
    }

    virtual bool onPhaseChange(ServerPlayer *OEedado) const{
        int m = 0;
        Room *room = OEedado->getRoom();
        //const QList<int> &guayanpile = OEedado->getPile("guanyanile");
        QList<int> hearts, clubs, spades, diamonds;
        foreach(int card_id, OEedado->getPile("guayanpile")){
            const Card *card = Sanguosha->getCard(card_id);
            Card::Suit suit = card->getSuit();

            switch(suit){
            case Card::Heart:   hearts << card_id;  break;
            case Card::Diamond: diamonds << card_id;break;
            case Card::Spade:   spades << card_id;  break;
            case Card::Club:    clubs << card_id;   break;
            default: break;
            }
        }
        if(hearts.length() > 2 || diamonds.length() > 2 || spades.length() > 2 || clubs.length() > 2)
            m++;

        if(!m) return false;

        LogMessage log;
        log.type = "#NitaiWake";
        log.from = OEedado;
        room->sendLog(log);
        /*
        room->playSkillEffect("nitai");

        room->broadcastInvoke("animate", "lightbox:$nitai:5000");*/
        room->getThread()->delay(5000);

        room->setPlayerMark(OEedado, "nitai", 1);
        room->acquireSkill(OEedado, "tiandu");
        room->acquireSkill(OEedado, "mod");

        room->loseMaxHp(OEedado);

        return false;
    }
};

//FROM UBUN TENKEI
//OE3144 ����-��������飨��������������������˵��������Ϲ��������
//��˵��λ�ǰ�������ô����������
/*��������������غ�����ÿ�ܵ�1���˺�����1���������ޡ�*/
class Skydao:public MasochismSkill{
public:
    Skydao():MasochismSkill("skydao"){
        frequency = Compulsory;
    }

    virtual void onDamaged(ServerPlayer *player, const DamageStruct &damage) const{
        Room *room = player->getRoom();
        if(damage.to && damage.to == player && player->getPhase() == Player::NotActive){
            LogMessage log;
            log.from = player;
            log.type = "#SkydaoMAXHP";
            log.arg2 = objectName();
            room->setPlayerProperty(player, "maxhp", player->getMaxHP() + 1);
            log.arg = QString::number(player->getMaxHP());
            room->sendLog(log);
        }
    }
};

/*���飺�����������ܵ��˺�ʱ�������������ȫ�����ٻ�ͬʱΪ���٣��������˱������1��������1����������
  �����⡣������Ϊ�ҳ��˲���2Ѫ������*/
class Noqing:public MasochismSkill{
public:
    Noqing():MasochismSkill("noqing"){
        frequency = Compulsory;
    }

    virtual int getPriority() const{
        return -1;
    }

    virtual QString getDefaultChoice(ServerPlayer *player) const{
        if(player->getMaxHP() >= player->getHp() + 2)
            return "maxhp";
        else
            return "hp";
    }

    virtual void onDamaged(ServerPlayer *player, const DamageStruct &damage) const{
        Room *room = player->getRoom();
        if(damage.to && damage.to == player){
            foreach(ServerPlayer *tmp, room->getOtherPlayers(player))
                if(tmp->getHp() < player->getHp())
                    return;
            foreach(ServerPlayer *tmp, room->getAllPlayers()){
                QString choice = room->askForChoice(tmp, objectName(), "hp+max_hp");
                LogMessage log;
                log.from = player;
                log.to << tmp;
                log.arg = objectName();
                if(choice == "hp"){
                    log.type = "#NoqingLoseHp";
                    room->sendLog(log);
                    room->loseHp(tmp);
                }else{
                    log.type = "#NoqingLoseMaxHp";
                    room->sendLog(log);
                    room->loseMaxHp(tmp);
                }
            }
        }
    }
};

//OE3134 ����-��ʧ�����֣�ָ��
/*��ʧ����������Tuiyin�����������һ�������������Լ��ĳ��ƽ׶κ����ƽ׶Ρ�������������һ������������ǡ�*/
class Tuiyin: public PhaseChangeSkill{
public:
    Tuiyin():PhaseChangeSkill("tuiyin"){

    }

    virtual int getPriority() const{
        return 3;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target) && !target->isKongcheng();
    }

    virtual bool onPhaseChange(ServerPlayer *OEjuzu) const{
        Room *room = OEjuzu->getRoom();

        if(OEjuzu->getPhase() == Player::Play){
            if(!room->askForDiscard(OEjuzu, "tuiyin", 1, true, false))
                return false;
            OEjuzu->gainMark("@tuiyin");
            OEjuzu->setFlags("disappear");
            return true;
        }else if(OEjuzu->getPhase() == Player::Discard || OEjuzu->hasFlag("disappear")){
            OEjuzu->setFlags("-disappear");
            return true;
        }
        return false;
    }
};

class Chuxian: public PhaseChangeSkill{
public:
    Chuxian():PhaseChangeSkill("chuxian"){
        frequency = Wake;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target)
                && target->getPhase() == Player::Start
                && target->getMark("chuxian") == 0
                && target->getMark("@tuiyin") >= 3;
    }

    virtual bool onPhaseChange(ServerPlayer *OEjuzu) const{
        Room *room = OEjuzu->getRoom();

        room->setPlayerMark(OEjuzu, "chuxian", 1);
        room->loseMaxHp(OEjuzu);

        LogMessage log;
        log.type = "#ChuxianWake";
        log.from = OEjuzu;
        log.arg = QString::number(OEjuzu->getPile("field").length());
        room->sendLog(log);

        room->acquireSkill(OEjuzu, "zhiyin");

        return false;
    }
};

class Zhiyin: public PhaseChangeSkill{
public:
    Zhiyin():PhaseChangeSkill("zhiyin"){

    }

    virtual int getPriority() const{
        return 3;
    }


    virtual bool onPhaseChange(ServerPlayer *OEjuzu) const{
        if(OEjuzu->getPhase() == Player::Finish && OEjuzu->getMark("@tuiyin")){
            if(OEjuzu->askForSkillInvoke(objectName())){
                OEjuzu->loseMark("@tuiyin");
                Room *room = OEjuzu->getRoom();

                ServerPlayer *player = room->askForPlayerChosen(OEjuzu, room->getOtherPlayers(OEjuzu), objectName());

                LogMessage log;
                log.type = "#Zhiyin";
                log.from = OEjuzu;
                log.to << player;
                room->sendLog(log);

                room->setCurrent(player);
                room->getThread()->trigger(TurnStart, player);
                room->setCurrent(OEjuzu);
            }
        }
        return false;
    }
};

OETanBPackage::OETanBPackage()
    :Package("OEtanb")
{
    General *OEedado = new General(this, "edado", "tan", 3, false);
    OEedado->addSkill(new Guayan);
    OEedado->addSkill("#luoyi");
    OEedado->addSkill(new Nitai);
    related_skills.insertMulti("guayan", "#luoyi");

    General *OEtianyin = new General(this, "tianyin$", "tan", 4, false);
    OEtianyin->addSkill(new Skydao);
    OEtianyin->addSkill(new Noqing);

    General *OEjuzu = new General(this, "jujuzu", "tan", 4, true);
    OEjuzu->addSkill(new Tuiyin);
    OEjuzu->addSkill(new Chuxian);

    //General *OEwilliam = new General(this, "william", "tan", 4, true);

    skills << new Zhiyin;

    addMetaObject<GuayanCard>();
}

ADD_PACKAGE(OETanB)


