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

/*OE3126 edado-寡言，拟态*/
/*寡言：
  回合开始阶段，你可进行一次判定：
  若为红桃，你回复1点体力；
  若为黑桃，获得【天义】拼点胜利效果直至回合结束；
  若为方片，你可和任意一名角色拼点，若你赢，你抽取该角色一半手牌（向上取整）；
  若为梅花，你获得【裸衣】效果直至回合结束。
  然后将判定牌放在你的武将牌上（移出游戏）

  改成天义和裸衣纯粹偷懒，天义和裸衣的代码都不少来着
  方片的效果改为可选（不然如果你没有手牌或场上其他人都没有手牌怎么办？注：若你无手牌，则点确定也无事发生）
  判定牌放在武将牌上是为了拟态的发动（不然你得记得你判出过三张同花色牌，面杀还可以，这里不能接受）*/
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
    ServerPlayer *tiger = targets.first();//驱虎遗留，别吐槽了
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

/*拟态：觉醒技，回合开始阶段，若你拥有三张同花色【寡言】牌，须减1点体力上限，并永久获得技能【天妒】【MOD】。
  话说被不屈代码误导了好几天。。。
  2血加个英姿忒脆，所以我又加了个【MOD】（围观群众：把妹自重！！！），其实本来想加【化身】的，预料到会有严重BUG，放弃了*/
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
//OE3144 天音-天道，无情（代码来自宇文天启，话说技能名真瞎！！！）
//话说这位是版主，那么主公技待补
/*天道：锁定技，回合外你每受到1点伤害增加1点体力上限。*/
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

/*无情：锁定技，你受到伤害时，若你的体力是全场最少或同时为最少，则所有人必须减少1点体力或1点体力上限
  阵阵寒意。。。因为我出了不少2血。。。*/
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

//OE3134 猴子-消失，出现，指引
/*消失（代码误作Tuiyin）：你可以弃一张手牌来跳过自己的出牌阶段和弃牌阶段。若如此做，获得一个【退隐】标记。*/
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

    skills << new Zhiyin;

    addMetaObject<GuayanCard>();
}

ADD_PACKAGE(OETanB)


