#include "standard.h"
#include "skill.h"
#include "peasa.h"
#include "client.h"
#include "carditem.h"
#include "engine.h"
#include "ai.h"

class Piaoyong: public PhaseChangeSkill{
public:
    Piaoyong():PhaseChangeSkill("piaoyong"){
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        if(player->getPhase() != Player::Start || !player->askForSkillInvoke(objectName()))
            return false;
        Room *room = player->getRoom();
        player->skip(Player::Judge);
        if(room->askForChoice(player, objectName(), "first+second") == "first")
            player->skip(Player::Draw);
        else
            player->skip(Player::Play);
        foreach(const Card *card, player->getJudgingArea())
            room->throwCard(card);
        ServerPlayer *target = room->askForPlayerChosen(player, room->getAllPlayers(), objectName());
        Slash *slash = new Slash(Card::NoSuit, 0);
        slash->setSkillName(objectName());
        CardUseStruct use;
        use.card = slash;
        use.from = player;
        use.to << target;
        room->useCard(use, false);
        return false;
    }
};

class Wuzong: public PhaseChangeSkill{
public:
    Wuzong():PhaseChangeSkill("wuzong"){
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        Room *room = player->getRoom();
        if(player->getPhase() == Player::Start && player->askForSkillInvoke(objectName())){
            player->drawCards(3);
            room->acquireSkill(player, "wusheng");
            room->acquireSkill(player, "paoxiao");
            room->setPlayerFlag(player, "wuzong");
            return false;
        }
        if(player->getPhase() == Player::NotActive && player->hasFlag("wuzong"))
            room->killPlayer(player);
        return false;
    }
};

class Fugui:public DrawCardsSkill{
public:
    Fugui():DrawCardsSkill("fugui"){
        frequency = Compulsory;
    }

    virtual int getDrawNum(ServerPlayer *myself, int n) const{
        Room *room = myself->getRoom();
        int x = myself->getLostHp();
        LogMessage log;
        log.from = myself;
        log.type = "#TriggerSkill";
        log.arg = objectName();
        room->sendLog(log);
        return n + x;
    }
};

class Zizhu: public PhaseChangeSkill{
public:
    Zizhu():PhaseChangeSkill("zizhu"){
        frequency = Wake;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target)
                && target->getMark("zizhu") == 0
                && target->getPhase() == Player::Start
                && target->getHp() > 1;
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        Room *room = player->getRoom();
        bool caninvoke = false;
        foreach(ServerPlayer *tmp, room->getAlivePlayers()){
            if(tmp->isKongcheng()){
                caninvoke = true;
                break;
            }
        }
        if(!caninvoke)
            return false;

        LogMessage log;
        log.type = "#ZizhuWake";
        log.from = player;
        log.arg = objectName();
        room->sendLog(log);

        room->loseHp(player);

        if(room->askForChoice(player, objectName(), "rende+jujian") == "rende")
            room->acquireSkill(player, "rende");
        else
            room->acquireSkill(player, "jujian");

        room->setPlayerMark(player, "zizhu", 1);
        return false;
    }
};

class Qiuhe: public TriggerSkill{
public:
    Qiuhe():TriggerSkill("qiuhe"){
        events << CardEffected;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *player, QVariant &data) const{
        CardEffectStruct effect = data.value<CardEffectStruct>();
        if(!effect.from || (!effect.card->inherits("Slash") && !effect.card->isNDTrick()))
            return false;
        if(!effect.from->hasFlag("qiuhe") && player->askForSkillInvoke(objectName())){
            effect.from->setFlags("qiuhe");
            effect.from->drawCards(1);
            player->obtainCard(effect.card);
            return true;
        }
        return false;
    }
};

class Duanbing: public TriggerSkill{
public:
    Duanbing():TriggerSkill("duanbing"){
        events << SlashEffect << SlashProceed;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        if(event == SlashEffect){
            if(!player->getWeapon())
                effect.to->addMark("qinggang");
        }
        else{
            if(player->getArmor())
                return false;
            Room *room = player->getRoom();
            QString slasher = player->objectName();

            const Card *first_jink = NULL, *second_jink = NULL;
            first_jink = room->askForCard(effect.to, "jink", "@duanbing-jink-1:" + slasher);
            if(first_jink)
                second_jink = room->askForCard(effect.to, "jink", "@duanbing-jink-2:" + slasher);

            Card *jink = NULL;
            if(first_jink && second_jink){
                jink = new DummyCard;
                jink->addSubcard(first_jink);
                jink->addSubcard(second_jink);
            }
            room->slashResult(effect, jink);
            return true;
        }
        return false;
    }
};

GuiouCard::GuiouCard(){
}

bool GuiouCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty();
}

void GuiouCard::onEffect(const CardEffectStruct &effect) const{
    effect.from->getRoom()->acquireSkill(effect.to, "#guioupro");
    effect.from->tag["GuiouTarget"] = QVariant::fromValue(effect.to);
}

class GuiouViewAsSkill: public OneCardViewAsSkill{
public:
    GuiouViewAsSkill():OneCardViewAsSkill("guiou"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "@@guiou";
    }

    virtual bool viewFilter(const CardItem *watch) const{
        return watch->getCard()->isBlack();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        GuiouCard *card = new GuiouCard;
        card->addSubcard(card_item->getCard()->getId());
        return card;
    }
};

class GuiouPro: public ProhibitSkill{
public:
    GuiouPro():ProhibitSkill("#guioupro"){
    }

    virtual bool isProhibited(const Player *from, const Player *to, const Card *card) const{
        return card->inherits("TrickCard") && card->isRed();
    }
};

class Guiou: public PhaseChangeSkill{
public:
    Guiou():PhaseChangeSkill("guiou"){
        view_as_skill = new GuiouViewAsSkill;
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        Room *room = player->getRoom();
        if(player->getPhase() == Player::Finish){
            room->askForUseCard(player, "@@guiou", "@guiou");
        }
        else if(player->getPhase() == Player::Start){
            PlayerStar taregt = player->tag["GuiouTarget"].value<PlayerStar>();
            if(taregt)
                room->detachSkillFromPlayer(taregt, "#guioupro");
        }
        return false;
    }
};

class Xiaoguo: public TriggerSkill{
public:
    Xiaoguo():TriggerSkill("xiaoguo"){
        events << Damage;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        DamageStruct damage = data.value<DamageStruct>();
        if(!damage.card->inherits("Slash"))
            return false;
        if(player->askForSkillInvoke(objectName())){
            JudgeStruct judge;
            judge.pattern = QRegExp("(.*):(spade|club):(.*)");
            judge.good = true;
            judge.reason = objectName();
            judge.who = player;

            room->judge(judge);
            if(judge.isGood()){
                if(judge.card->getSuit() == Card::Spade)
                    player->obtainCard(judge.card);
                else
                    room->askForUseCard(player, "slash", "@xiaoguo");
            }
        }
        return false;
    }
};

class Huace:public MasochismSkill{
public:
    Huace():MasochismSkill("huace"){
    }

    virtual void onDamaged(ServerPlayer *player, const DamageStruct &damage) const{
        Room *room = player->getRoom();
        if(damage.damage < 1)
            return;
        for(int i = 0; i < damage.damage; i ++){
            if(player->askForSkillInvoke(objectName())){
                ServerPlayer *target = room->askForPlayerChosen(player, room->getAllPlayers(), objectName());
                target->drawCards(target->getLostHp());
            }
            else
                break;
        }
    }
};

class Baiuu: public PhaseChangeSkill{
public:
    Baiuu():PhaseChangeSkill("baiuu"){
        frequency = Wake;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target)
                && target->getMark("baiuu") == 0
                && target->getPhase() == Player::Start
                && target->getEquips().count() >= 3;
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        Room *room = player->getRoom();

        LogMessage log;
        log.type = "#ZizhuWake";
        log.from = player;
        log.arg = objectName();
        room->sendLog(log);

        room->throwCard(room->askForCardChosen(player, player, "e", objectName()));
        room->throwCard(room->askForCardChosen(player, player, "e", objectName()));

        room->acquireSkill(player, "quhu");
        room->acquireSkill(player, "duanliang");

        room->setPlayerMark(player, "baiuu", 1);
        return false;
    }
};

ZhonglianCard::ZhonglianCard(){
    target_fixed = true;
}

void ZhonglianCard::onUse(Room *room, const CardUseStruct &card_use) const{
    Peach *peach = new Peach(Card::NoSuit, 0);
    peach->setSkillName("zhonglian");
    foreach(int x, getSubcards())
        peach->addSubcard(Sanguosha->getCard(x));
    CardUseStruct use;
    use.card = peach;
    use.from = card_use.from;
    use.to << card_use.from->tag["ZhonglianTarget"].value<PlayerStar>();
    room->useCard(use);
}

class ZhonglianViewAsSkill: public ViewAsSkill{
public:
    ZhonglianViewAsSkill():ViewAsSkill("zhonglian"){
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        return selected.length() < 3;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "@@zhonglian";
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() != 2 ||
           cards.first()->getCard()->getNumber() != cards.last()->getCard()->getNumber())
            return NULL;
        ZhonglianCard *card = new ZhonglianCard;
        card->addSubcards(cards);
        return card;
    }
};

class Zhonglian: public TriggerSkill{
public:
    Zhonglian():TriggerSkill("zhonglian"){
        events << Dying;
        view_as_skill = new ZhonglianViewAsSkill;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        QList<ServerPlayer *> helpers;
        foreach(ServerPlayer *tmp, room->getAlivePlayers()){
            if(tmp->getCardCount(true) > 1)
                helpers << tmp;
        }
        if(helpers.isEmpty())
            return false;
        foreach(ServerPlayer *helper, helpers){
            helper->tag["ZhonglianTarget"] = QVariant::fromValue(player);
            room->askForUseCard(helper, "@@zhonglian", "@zhonglian:" + player->objectName());
            if(player->getHp() > 0)
                return true;
        }
        return false;
    }
};

MingwangCard::MingwangCard(){
    once = true;
    will_throw = false;
}

void MingwangCard::onEffect(const CardEffectStruct &effect) const{
    effect.to->obtainCard(this);

    Room *room = effect.to->getRoom();
    QList<ServerPlayer *> targets;
    foreach(ServerPlayer *player, room->getOtherPlayers(effect.to)){
        if(player->getHandcardNum() > effect.from->getHandcardNum())
            targets << player;
        if(player->getHp() > effect.from->getHp())
            targets << player;
    }
    if(!targets.isEmpty()){
        ServerPlayer *target = room->askForPlayerChosen(effect.from, targets, "mingwang");
        DamageStruct damage;
        damage.from = effect.to;
        damage.to = target;
        damage.card = this;
        room->damage(damage);
    }
}

class Mingwang: public OneCardViewAsSkill{
public:
    Mingwang():OneCardViewAsSkill("mingwang"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("MingwangCard");
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return true;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        MingwangCard *card = new MingwangCard;
        card->addSubcard(card_item->getFilteredCard());
        return card;
    }
};

class Lixin: public TriggerSkill{
public:
    Lixin():TriggerSkill("lixin"){
        events << AskForPeachesDone;
        frequency = Wake;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getMark("lixin") == 0
                && target->getHp() > 0;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *playeR, QVariant &) const{
        Room *room = playeR->getRoom();
        LogMessage log;
        log.type = "#ZizhuWake";
        log.from = playeR;
        log.arg = objectName();
        room->sendLog(log);

        if(room->askForChoice(playeR, objectName(), "recover+draw") == "recover"){
            RecoverStruct recover;
            recover.who = playeR;
            room->recover(playeR, recover);
        }else
            room->drawCards(playeR, 2);

        room->acquireSkill(playeR, "lijian");
        room->acquireSkill(playeR, "weimu");
        room->setPlayerMark(playeR, "lixin", 1);
        return false;
    }
};

class Dancer: public TriggerSkill{
public:
    Dancer():TriggerSkill("dancer"){
        events << SlashProceed;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *lubu, QVariant &data) const{
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        Room *room = lubu->getRoom();
        room->playSkillEffect(objectName());

        QString slasher = lubu->objectName();

        const Card *first_jink = NULL, *second_jink = NULL;
        first_jink = room->askForCard(effect.to, "jink", "@dancer-jink-1:" + slasher);
        if(first_jink)
            second_jink = room->askForCard(effect.to, ".", "@dancer-jink-2:" + slasher);

        Card *jink = NULL;
        if(first_jink && second_jink){
            jink = new DummyCard;
            jink->addSubcard(first_jink);
            jink->addSubcard(second_jink);
        }
        room->slashResult(effect, jink);
        return true;
    }
};

class Fuckmoon: public PhaseChangeSkill{
public:
    Fuckmoon():PhaseChangeSkill("fuckmoon"){
        frequency = Frequent;
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        //Room *room = player->getRoom();
        if(player->getPhase() == Player::NotActive && player->askForSkillInvoke(objectName())){
            int num = player->getMaxHP() - player->getHandcardNum();
            if(num > 0)
                player->drawCards(num);
        }
        return false;
    }
};

// beimihu edit by player

#include <QCommandLinkButton>
class Guishu: public GameStartSkill{
public:
    Guishu():GameStartSkill("guishu"){

    }

    static void PlayEffect(ServerPlayer *player, const QString &skill_name){
        int r = qrand() % 2;
        if(player->getGender() == General::Female)
            r += 2;

        player->getRoom()->playSkillEffect(skill_name, r);
    }

    static void AcquireGenerals(ServerPlayer *player, int n){
        QStringList list = GetAvailableGenerals(player);
        qShuffle(list);

        QStringList acquired = list.mid(0, n);
        QVariantList guishus = player->tag["Guipus"].toList();
        foreach(QString guishu, acquired)
            guishus << guishu;
        player->tag["Guipus"] = guishus;
        //player->invoke("animate", "guishu:" + acquired.join(":"));

        LogMessage log;
        log.type = "#GetGuipu";
        log.from = player;
        log.arg = QString::number(n);
        log.arg2 = QString::number(guishus.length());
        player->getRoom()->sendLog(log);
        player->gainMark("@pu", n);
    }

    static QStringList GetAvailableGenerals(ServerPlayer *player){
        QSet<QString> all = Sanguosha->getLimitedGeneralNames().toSet();
        QSet<QString> guishu_set, room_set;
        QVariantList guishus = player->tag["Guipus"].toList();
        foreach(QVariant guishu, guishus)
            guishu_set << guishu.toString();

        Room *room = player->getRoom();
        QList<const ServerPlayer *> players = room->findChildren<const ServerPlayer *>();
        foreach(const ServerPlayer *player, players){
            room_set << player->getGeneralName();
            if(player->getGeneral2())
                room_set << player->getGeneral2Name();
        }

        static QSet<QString> banned;
        if(banned.isEmpty()){
            banned << "beimihu";
        }
        return (all - banned - guishu_set - room_set).toList();
    }

    static QString SelectGeneral(ServerPlayer *player){
        Room *room = player->getRoom();
        PlayEffect(player, "guishu");

        QVariantList guishus = player->tag["Guipus"].toList();
        if(guishus.isEmpty())
            return QString();

        QStringList guishu_generals;
        foreach(QVariant guishu, guishus)
            guishu_generals << guishu.toString();

        return room->askForGeneral(player, guishu_generals);
    }

    virtual void onGameStart(ServerPlayer *player) const{
        AcquireGenerals(player, player->getRoom()->getPlayers().count() + 2);
    }

    virtual QDialog *getDialog() const{
        static GuishuDialog *dialog;

        if(dialog == NULL)
            dialog = new GuishuDialog;

        return dialog;
    }
};

GuishuDialog::GuishuDialog()
{
    setWindowTitle(tr("Incarnation"));
}

void GuishuDialog::popup(){
    QVariantList guishu_list = Self->tag["Guipus"].toList();
    QList<const General *> guishus;
    foreach(QVariant guishu, guishu_list)
        guishus << Sanguosha->getGeneral(guishu.toString());

    fillGenerals(guishus);
    show();
}

class GuishuEffect: public TriggerSkill{
public:
    GuishuEffect():TriggerSkill("#guishu-effect"){
        events << Death;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    static void YaojiWake(ServerPlayer *jj){
        if(jj->hasSkill("yaoji") && jj->getMark("yaoji") == 0){
            Room *room = jj->getRoom();
            int guishunum = jj->tag["Guipus"].toList().count();
            int deadnum = room->getPlayers().count() - room->getAlivePlayers().length();
            if(guishunum <= deadnum){
                LogMessage log;
                log.type = "#ZizhuWake";
                log.from = jj;
                log.arg = "yaoji";
                room->sendLog(log);

                room->acquireSkill(jj, "guicai");
                room->acquireSkill(jj, "huangtian");
                room->setPlayerMark(jj, "yaoji", 1);
            }
        }
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        DamageStar damage = data.value<DamageStar>();
        ServerPlayer *killer = damage ? damage->from : NULL;

        if(killer && killer->hasSkill("guishu")){
            if(killer->askForSkillInvoke("guishu")){
                QVariantList guishus = killer->tag["Guipus"].toList();
                qShuffle(guishus);
                QString guishu = guishus.first().toString();
                const General* general = Sanguosha->getGeneral(guishu);
                LogMessage log;
                log.type = "#Guishu";
                log.from = killer;
                log.arg = guishu;
                Room *room = killer->getRoom();
                room->sendLog(log);

                foreach(const Skill *skill, general->getVisibleSkillList()){
                    if(!killer->getVisibleSkillList().contains(skill))
                        room->acquireSkill(killer, skill->objectName());
                }
                guishus.clear();
                guishus << guishu;
                log.type = "#GuishuLost";
                log.arg = guishu;
                room->sendLog(log);

                killer->tag["Guipus"] = guishus;
                killer->loseMark("@pu", killer->getMark("@pu") - 1);
                YaojiWake(killer);
            }
        }
        return false;
    }
};

class YuguiViewAsSkill:public ZeroCardViewAsSkill{
public:
    YuguiViewAsSkill():ZeroCardViewAsSkill("yugui"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return Slash::IsAvailable(player) && player->getMark("@pu") > 0;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        if(player->getMark("@pu") < 1)
            return false;
        return pattern == "slash" ||
                pattern == "jink" ||
                pattern == "nullification";
    }

    virtual const Card *viewAs() const{
        switch(ClientInstance->getStatus()){
        case Client::Playing:{
                Card *slash = new Slash(Card::NoSuit, 0);
                slash->setSkillName("yugui");
                return slash;
            }
        case Client::Responsing:{
                QString pattern = ClientInstance->getPattern();
                if(pattern == "slash"){
                    Card *slash = new Slash(Card::NoSuit, 0);
                    slash->setSkillName("yugui");
                    return slash;
                }
                else if(pattern == "jink"){
                    Card *jink = new Jink(Card::NoSuit, 0);
                    jink->setSkillName("yugui");
                    return jink;
                }
                else if(pattern == "nullification"){
                    Card *nullification = new Nullification(Card::NoSuit, 0);
                    nullification->setSkillName("yugui");
                    return nullification;
                }
            }
        default:
            return NULL;
        }
    }
};

class Yugui: public TriggerSkill{
public:
    Yugui():TriggerSkill("yugui"){
        events << CardUsed << CardResponsed;
        view_as_skill = new YuguiViewAsSkill;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        QVariantList guipus = player->tag["Guipus"].toList();
        if(guipus.isEmpty())
            return false;
        CardStar card = NULL;
        if(event == CardUsed){
            CardUseStruct use = data.value<CardUseStruct>();
            card = use.card;
        }else if(event == CardResponsed)
            card = data.value<CardStar>();
        if(card->getSkillName() == objectName()){
            LogMessage log;
            log.type = "#GuishuRemove";
            log.from = player;
            log.arg = guipus.first().toString();
            guipus.removeFirst();
            player->loseMark("@pu");
            player->getRoom()->sendLog(log);

            player->tag["Guipus"] = guipus;
        }
        return false;
    }
};

PeasaPackage::PeasaPackage()
    :Package("peasa")
{
    General *guanzhang = new General(this, "guanzhang", "shu");
    guanzhang->addSkill(new Piaoyong);
    guanzhang->addSkill(new Wuzong);

    General *mizhu = new General(this, "mizhu", "shu", 3);
    mizhu->addSkill(new Fugui);
    mizhu->addSkill(new Zizhu);

    General *zhugejin = new General(this, "zhugejin", "wu", 3);
    zhugejin->addSkill(new Qiuhe);
    zhugejin->addSkill(new Skill("kuanhp", Skill::Compulsory));

    General *dingfeng = new General(this, "dingfeng", "wu");
    dingfeng->addSkill(new Duanbing);

    General *yuejin = new General(this, "yuejin", "wei");
    yuejin->addSkill(new Guiou);
    skills << new GuiouPro;
    yuejin->addSkill(new Xiaoguo);

    General *xunyou = new General(this, "xunyou", "wei", 3);
    xunyou->addSkill(new Huace);
    xunyou->addSkill(new Baiuu);
    xunyou->addSkill(new Skill("shibiao", Skill::Compulsory));

    General *wangyun = new General(this, "wangyun", "qun", 3);
    wangyun->addSkill(new Zhonglian);
    wangyun->addSkill(new Mingwang);
    wangyun->addSkill(new Lixin);

    General *lvlingqi = new General(this, "lvlingqi", "qun", 3, false);
    lvlingqi->addSkill(new Dancer);
    lvlingqi->addSkill(new Fuckmoon);

    General *beimihu = new General(this, "beimihu", "qun", 3, false);
    beimihu->addSkill(new Guishu);
    beimihu->addSkill(new GuishuEffect);
    beimihu->addSkill(new Yugui);
    beimihu->addSkill(new Skill("yaoji", Skill::Wake));
    related_skills.insertMulti("guishu", "#guishu-effect");

    addMetaObject<GuiouCard>();
    addMetaObject<ZhonglianCard>();
    addMetaObject<MingwangCard>();
}

ADD_PACKAGE(Peasa);
