#include "sp-package.h"
#include "general.h"
#include "skill.h"
#include "standard-skillcards.h"
#include "carditem.h"
#include "engine.h"
#include "standard.h"
#include "clientplayer.h"
#include "client.h"
#include "ai.h"
#include "settings.h"

class JileiClear: public PhaseChangeSkill{
public:
    JileiClear():PhaseChangeSkill("#jilei-clear"){

    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        if(target->getPhase() == Player::NotActive){
            Room *room = target->getRoom();
            QList<ServerPlayer *> players = room->getAllPlayers();
            foreach(ServerPlayer *player, players){
                if(player->hasFlag("jilei")){
                    player->jilei(".");
                    player->invoke("jilei");

                    LogMessage log;
                    log.type = "#JileiClear";
                    log.from = player;
                    room->sendLog(log);
                }
            }
        }

        return false;
    }
};


class Jilei: public TriggerSkill{
public:
    Jilei():TriggerSkill("jilei"){
        events << Predamaged;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *yangxiu, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();

        if(damage.from == NULL)
           return false;

        Room *room = yangxiu->getRoom();
        if(room->askForSkillInvoke(yangxiu, objectName(), data)){
            QString choice = room->askForChoice(yangxiu, objectName(), "basic+equip+trick");
            room->playSkillEffect(objectName());

            damage.from->jilei(choice);
            damage.from->invoke("jilei", choice);
            damage.from->setFlags("jilei");

            LogMessage log;
            log.type = "#Jilei";
            log.from = yangxiu;
            log.to << damage.from;
            log.arg = choice;
            room->sendLog(log);
        }

        return false;
    }
};

class Danlao: public TriggerSkill{
public:
    Danlao():TriggerSkill("danlao"){
        events << CardEffected;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        CardEffectStruct effect = data.value<CardEffectStruct>();

        if(effect.multiple && effect.card->inherits("TrickCard")){
            Room *room = player->getRoom();
            if(room->askForSkillInvoke(player, objectName(), data)){
                room->playSkillEffect(objectName());

                LogMessage log;

                log.type = "#DanlaoAvoid";
                log.from = player;
                log.arg = effect.card->objectName();

                room->sendLog(log);

                player->drawCards(1);
                return true;
            }
        }

        return false;
    }
};


class Yongsi: public TriggerSkill{
public:
    Yongsi():TriggerSkill("yongsi"){
        events << DrawNCards << PhaseChange;
        frequency = Compulsory;
    }

    int getKingdoms(ServerPlayer *yuanshu) const{
        QSet<QString> kingdom_set;
        Room *room = yuanshu->getRoom();
        foreach(ServerPlayer *p, room->getAlivePlayers()){
            kingdom_set << p->getKingdom();
        }

        return kingdom_set.size();
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *yuanshu, QVariant &data) const{
        if(event == DrawNCards){
            int x = getKingdoms(yuanshu);
            data = data.toInt() + x;

            Room *room = yuanshu->getRoom();
            LogMessage log;
            log.type = "#YongsiGood";
            log.from = yuanshu;
            log.arg = QString::number(x);
            room->sendLog(log);

            room->playSkillEffect("yongsi", x);

        }else if(event == PhaseChange && yuanshu->getPhase() == Player::Discard){
            int x = getKingdoms(yuanshu);
            int total = yuanshu->getEquips().length() + yuanshu->getHandcardNum();
            Room *room = yuanshu->getRoom();

            if(total <= x){
                yuanshu->throwAllHandCards();
                yuanshu->throwAllEquips();

                LogMessage log;
                log.type = "#YongsiWorst";
                log.from = yuanshu;
                log.arg = QString::number(total);
                room->sendLog(log);

            }else{
                room->askForDiscard(yuanshu, "yongsi", x, false, true);

                LogMessage log;
                log.type = "#YongsiBad";
                log.from = yuanshu;
                log.arg = QString::number(x);
                room->sendLog(log);
            }
        }

        return false;
    }
};

class Weidi:public ZeroCardViewAsSkill{
public:
    Weidi():ZeroCardViewAsSkill("weidi"){
        frequency = Compulsory;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->hasLordSkill("jijiang") && Slash::IsAvailable(player);
    }

    virtual const Card *viewAs() const{
        return Sanguosha->cloneSkillCard("JijiangCard");
    }
};


class Yicong: public DistanceSkill{
public:
    Yicong():DistanceSkill("yicong"){

    }

    virtual int getCorrect(const Player *from, const Player *to) const{
        int correct = 0;
        if(from->hasSkill(objectName()) && from->getHp() > 2)
            correct --;
        if(to->hasSkill(objectName()) && to->getHp() <= 2)
            correct ++;

        return correct;
    }
};

class Xuwei: public ZeroCardViewAsSkill{
public:
    Xuwei():ZeroCardViewAsSkill("xuwei"){
        huanzhuang_card = new HuanzhuangCard;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        if(player->hasUsed("HuanzhuangCard"))
            return false;

        return player->getGeneralName() == "sp_diaochan";
    }

    virtual const Card *viewAs() const{
        return huanzhuang_card;
    }

private:
    HuanzhuangCard *huanzhuang_card;
};

class Xiuluo: public PhaseChangeSkill{
public:
    Xiuluo():PhaseChangeSkill("xiuluo"){

    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target)
                && target->getPhase() == Player::Start
                && !target->isKongcheng()
                && !target->getJudgingArea().isEmpty();
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        if(!target->askForSkillInvoke(objectName()))
            return false;

        Room *room = target->getRoom();
        int card_id = room->askForCardChosen(target, target, "j", objectName());
        const Card *card = Sanguosha->getCard(card_id);

        QString suit_str = card->getSuitString();
        QString pattern = QString(".%1").arg(suit_str.at(0).toUpper());
        QString prompt = QString("@xiuluo:::%1").arg(suit_str);
        if(room->askForCard(target, pattern, prompt)){
            room->throwCard(card);
        }

        return false;
    }
};

class Shenwei: public DrawCardsSkill{
public:
    Shenwei():DrawCardsSkill("shenwei"){
        frequency = Compulsory;
    }

    virtual int getDrawNum(ServerPlayer *player, int n) const{
        return n + 2;
    }
};

class Danji: public PhaseChangeSkill{
public:
    Danji():PhaseChangeSkill("danji"){
        frequency = Wake;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target)
                && target->getPhase() == Player::Start
                && target->getMark("danji") == 0
                && target->getHandcardNum() > target->getHp();
    }

    virtual bool onPhaseChange(ServerPlayer *guanyu) const{
        Room *room = guanyu->getRoom();
        ServerPlayer *the_lord = room->getLord();
        if(the_lord && the_lord->getGeneralName() == "caocao"){
            LogMessage log;
            log.type = "#DanjiWake";
            log.from = guanyu;
            log.arg = QString::number(guanyu->getHandcardNum());
            log.arg2 = QString::number(guanyu->getHp());
            room->sendLog(log);

            guanyu->setMark("danji", 1);

            room->loseMaxHp(guanyu);
            room->acquireSkill(guanyu, "mashu");
        }

        return false;
    }
};

//OMEGA ERA XIAOYAOJINZHIZHAN-GOD ZHANGLIAO, GANNING, SUNQUAN

PozhenCard::PozhenCard(){
}

bool PozhenCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(targets.length() >= 3)
        return false;

    if(to_select == Self)
        return false;

    return !to_select->isKongcheng();
}

void PozhenCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    int card_id = room->askForCardChosen(effect.from, effect.to, "h", "pozhen");
    const Card *card = Sanguosha->getCard(card_id);
    room->moveCardTo(card, effect.from, Player::Hand, false);

    room->setEmotion(effect.to, "bad");
    room->setEmotion(effect.from, "good");
}


class PozhenViewAsSkill: public ZeroCardViewAsSkill{
public:
    PozhenViewAsSkill():ZeroCardViewAsSkill("Pozhen"){
    }

    virtual const Card *viewAs() const{
        return new PozhenCard;
    }

protected:
    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern == "@@Pozhen";
    }
};

class Pozhen:public PhaseChangeSkill{
public:
    Pozhen():PhaseChangeSkill("Pozhen"){
        view_as_skill = new PozhenViewAsSkill;
    }

    virtual bool onPhaseChange(ServerPlayer *zhangliao) const{
        if(zhangliao->getPhase() == Player::Draw){
            Room *room = zhangliao->getRoom();
            bool can_invoke = false;
            QList<ServerPlayer *> other_players = room->getOtherPlayers(zhangliao);
            foreach(ServerPlayer *player, other_players){
                if(!player->isKongcheng()){
                    can_invoke = true;
                    break;
                }
            }

            if(can_invoke && room->askForUseCard(zhangliao, "@@Pozhen", "@Pozhen-card"))
                return true;
        }

        return false;
    }
};


class Zhangliao_shenwei:public OneCardViewAsSkill{
public:
    Zhangliao_shenwei():OneCardViewAsSkill("Zhangliao_shenwei"){

    }

    virtual bool viewFilter(const CardItem *to_select) const{
        const Card *card = to_select->getFilteredCard();

        switch(ClientInstance->getStatus()){
        case Client::Playing:{
                // jink as slash
                return card->inherits("Jink");
            }

        case Client::Responsing:{
                QString pattern = ClientInstance->getPattern();
                /*if(pattern == "slash")
                    return card->inherits("Jink");
                else */if(pattern == "jink")
                    return card->inherits("Slash");
            }

        default:
            return false;
        }
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return Slash::IsAvailable(player)&&! player->hasUsed("Zhangliao_shenwei");
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "jink" /*|| pattern == "slash"*/;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getFilteredCard();
        /*if(card->inherits("Slash")){
            Jink *jink = new Jink(card->getSuit(), card->getNumber());
            jink->addSubcard(card);
            jink->setSkillName(objectName());
            return jink;
        }else*/ if(card->inherits("Jink")){
            Slash *slash = new Slash(card->getSuit(), card->getNumber());
            slash->addSubcard(card);
            slash->setSkillName(objectName());
            return slash;
        }else
            return NULL;
    }
};
/*

xiaoqixiCard::xiaoqixiCard(){
    once = true;
    mute = true;
}

bool xiaoqixiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select->canSlash(Self);
}

void xiaoqixiCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    room->throwCard(room->askForCardChosen(effect.from, effect.to, "he", "xiaoqixi"));
}

class xiaoqixi: public ZeroCardViewAsSkill{
public:
    xiaoqixi():ZeroCardViewAsSkill("xiaoqixi"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("xiaoqixiCard");
    }

    virtual const Card *viewAs() const{
        return new xiaoqixiCard;
    }
};*/

class xiaoqixi: public OneCardViewAsSkill{
public:
    xiaoqixi():OneCardViewAsSkill("xiaoqixi"){

    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getFilteredCard()->isBlack();
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return (player->usedTimes("Dismantlement")<=1);
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *first = card_item->getCard();
        Dismantlement *dismantlement = new Dismantlement(first->getSuit(), first->getNumber());
        dismantlement->addSubcard(first->getId());
        dismantlement->setSkillName(objectName());
        return dismantlement;
    }
};


xiaozhihengCard::xiaozhihengCard(){
    target_fixed = true;
    once = true;
}

void xiaozhihengCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    room->throwCard(this);
    if(source->isAlive())
        room->drawCards(source, subcards.length());
}

class xiaozhiheng:public ViewAsSkill{
public:
    xiaozhiheng():ViewAsSkill("xiaozhiheng"){

    }
/*
    virtual bool viewFilter(const QList<CardItem *> &, const CardItem *) const{
        return true;
    }
*/
    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        return selected.length() < 3;
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.isEmpty())
            return NULL;

        xiaozhihengCard *xiaozhiheng_card = new xiaozhihengCard;
        xiaozhiheng_card->addSubcards(cards);

        return xiaozhiheng_card;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("xiaozhihengCard");
    }
};


class taoyuanjieyi: public GameStartSkill{
public:
    taoyuanjieyi():GameStartSkill("taoyuanjieyi"){

    }

    static void AcquireGenerals(ServerPlayer *OEtaoyuanxiongdi, int n){
        QStringList list = GetAvailableGenerals(OEtaoyuanxiongdi);
        qShuffle(list);

        QStringList acquired = list.mid(0, n);
        QVariantList taoyuanjieyis = OEtaoyuanxiongdi->tag["taoyuanjieyis"].toList();
        foreach(QString taoyuanjieyi, acquired)
            taoyuanjieyis << taoyuanjieyi;

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
        const TriggerSkill *skill = Sanguosha->getTriggerSkill(skill_name);
        bool inSkillSet = OEtaoyuanxiongdi->getRoom()->getThread()->inSkillSet(skill);
        OEtaoyuanxiongdi->getRoom()->acquireSkill(OEtaoyuanxiongdi, skill_name);

        if(skill && !inSkillSet &&
           skill->getTriggerEvents().contains(PhaseChange)
            && skill->triggerable(OEtaoyuanxiongdi)){

            QVariant void_data;
            skill->trigger(PhaseChange, OEtaoyuanxiongdi, void_data);
        }


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


class Qijin: public TriggerSkill{
public:
    Qijin():TriggerSkill("qijin"){
        events << CardLost;

        frequency = Frequent;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *OEgodzhaoyun, QVariant &data) const{
        if(OEgodzhaoyun->getCardCount(false) <7 ){
            CardMoveStar move = data.value<CardMoveStar>();

            if(move->from_place == Player::Hand){
                Room *room = OEgodzhaoyun->getRoom();
                if(room->askForSkillInvoke(OEgodzhaoyun, objectName())){
                    room->playSkillEffect(objectName());

                    OEgodzhaoyun->drawCards(1);
                }
            }
        }

        return false;
    }
};

SPPackage::SPPackage()
    :Package("sp")
{
    General *yangxiu = new General(this, "yangxiu", "wei", 3);
    yangxiu->addSkill(new Jilei);
    yangxiu->addSkill(new JileiClear);
    yangxiu->addSkill(new Danlao);

    related_skills.insertMulti("jilei", "#jilei-clear");

    General *gongsunzan = new General(this, "gongsunzan", "qun");
    gongsunzan->addSkill(new Yicong);

    General *yuanshu = new General(this, "yuanshu", "qun");
    yuanshu->addSkill(new Yongsi);
    yuanshu->addSkill(new Weidi);

    General *sp_diaochan = new General(this, "sp_diaochan", "qun", 3, false, true);
    sp_diaochan->addSkill("lijian");
    sp_diaochan->addSkill("biyue");
    sp_diaochan->addSkill(new Xuwei);

    General *sp_sunshangxiang = new General(this, "sp_sunshangxiang", "shu", 3, false, true);
    sp_sunshangxiang->addSkill("jieyin");
    sp_sunshangxiang->addSkill("xiaoji");

    General *shenlvbu1 = new General(this, "shenlvbu1", "god", 8, true, true);
    shenlvbu1->addSkill("mashu");
    shenlvbu1->addSkill("wushuang");

    General *shenlvbu2 = new General(this, "shenlvbu2", "god", 4, true, true);
    shenlvbu2->addSkill("mashu");
    shenlvbu2->addSkill("wushuang");
    shenlvbu2->addSkill(new Xiuluo);
    shenlvbu2->addSkill(new Shenwei);
    shenlvbu2->addSkill(new Skill("shenji"));

    General *sp_guanyu = new General(this, "sp_guanyu", "wei", 4);
    sp_guanyu->addSkill("wusheng");
    sp_guanyu->addSkill(new Danji);

    General *OEgodzhangliao = new General(this, "OEgodzhangliao", "god", 4, true, true);
    OEgodzhangliao->addSkill(new Pozhen);
    OEgodzhangliao->addSkill(new Skill("changdao", Skill::Compulsory));
    OEgodzhangliao->addSkill(new Zhangliao_shenwei);

    General *OEganningXYJ = new General(this, "OEganningXYJ", "wu", 4, true, true);
    OEganningXYJ->addSkill(new xiaoqixi);

    General *OEsunquanXYJ = new General(this, "OEsunquanXYJ", "wu", 4, true, true);
    OEsunquanXYJ->addSkill(new xiaozhiheng);

    General *OEtaoyuanxiongdi = new General(this, "OEtaoyuanxiongdi", "qun", 4, true, true);
    OEtaoyuanxiongdi->addSkill(new taoyuanjieyi);
    OEtaoyuanxiongdi->addSkill(new taoyuanjieyiBegin);
    OEtaoyuanxiongdi->addSkill(new taoyuanjieyiEnd);

    General *OEgodzhaoyun = new General(this, "OEgodzhaoyun", "shu", 2, true, true);
    OEgodzhaoyun->addSkill(new Qijin);
    OEgodzhaoyun->addSkill("paoxiao");
    OEgodzhaoyun->addSkill("longhun");

    addMetaObject<PozhenCard>();
}

ADD_PACKAGE(SP);
