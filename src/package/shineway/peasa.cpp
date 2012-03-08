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
            room->playSkillEffect(objectName());
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
        int x = myself->getLostHp();
        if(x > 0){
            LogMessage log;
            log.from = myself;
            log.type = "#TriggerSkill";
            log.arg = objectName();
            myself->getRoom()->sendLog(log);
        }
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
        room->playSkillEffect(objectName());

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
        Room *room = player->getRoom();
        if(!effect.from->hasFlag("qiuhe") && player->askForSkillInvoke(objectName())){
            int index = effect.card->inherits("Slash") ? 1: 2;
            room->playSkillEffect(objectName(), index);
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
        Room *room = player->getRoom();
        if(event == SlashEffect){
            if(!player->getWeapon()){
                room->playSkillEffect(objectName(), 1);
                effect.to->addMark("qinggang");
            }
        }
        else{
            if(player->getArmor())
                return false;
            room->playSkillEffect(objectName(), 2);
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
    mute = true;
}

bool GuiouCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty();
}

void GuiouCard::onEffect(const CardEffectStruct &effect) const{
    int index = effect.to->hasSkill("guiou") ? 2: 1;
    effect.from->getRoom()->playSkillEffect("guiou", index);
    effect.to->gainMark("@gi");
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

class GuiouPro: public TriggerSkill{
public:
    GuiouPro():TriggerSkill("#guioupro"){
        events << CardEffected;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getMark("@gi") > 0;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        CardEffectStruct effect = data.value<CardEffectStruct>();
        if(effect.card && effect.card->inherits("TrickCard") && effect.card->isRed())
            return true;
        return false;
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
                taregt->loseAllMarks("@gi");
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
                if(judge.card->getSuit() == Card::Spade){
                    room->playSkillEffect(objectName(), 1);
                    player->obtainCard(judge.card);
                }
                else{
                    room->playSkillEffect(objectName(), 2);
                    room->askForUseCard(player, "slash", "@xiaoguo");
                }
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
                room->playSkillEffect(objectName());
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
        room->playSkillEffect(objectName());

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
    mute = true;
}

bool MingwangCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    return targets.length() == 2;
}

bool MingwangCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(targets.isEmpty()){
        return to_select != Self;
    }else if(targets.length() == 1){
        const Player *first = targets.first();
        return to_select != Self && to_select != first &&
                (to_select->getHandcardNum() > Self->getHandcardNum() ||
                 to_select->getHp() > Self->getHp());
    }else
        return false;
}

void MingwangCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    ServerPlayer *target = targets.first();
    ServerPlayer *targeT = targets.last();
    target->obtainCard(this);
    int index = targeT->getHp() > source->getHp() ? 1: 2;
    room->playSkillEffect("mingwang", index);
    DamageStruct damage;
    damage.from = target;
    damage.to = targeT;
    damage.card = this;
    room->damage(damage);
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
        return target->hasSkill(objectName()) && target->getMark("lixin") == 0
                && target->getHp() > 0;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *playeR, QVariant &) const{
        Room *room = playeR->getRoom();
        LogMessage log;
        log.type = "#ZizhuWake";
        log.from = playeR;
        log.arg = objectName();
        room->sendLog(log);
        room->playSkillEffect(objectName());

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
        QVariantList guishus = player->property("guipus").toList();
        foreach(QString guishu, acquired)
            guishus << guishu;
        player->getRoom()->setPlayerProperty(player, "guipus", guishus);
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
        QVariantList guishus = player->property("guipus").toList();
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
            banned << "beimihu" << "anjiang";
        }
        return (all - banned - guishu_set - room_set).toList();
    }

    static QString SelectGeneral(ServerPlayer *player){
        if(player->getMark("@pu") < 1)
            return QString();
        QVariantList guishus = player->property("guipus").toList();

        Room *room = player->getRoom();
        PlayEffect(player, "guishu");
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
    setWindowTitle(Sanguosha->translate("guishu"));
}

void GuishuDialog::popup(){
    if(ServerInfo.FreeChoose){
        QVariantList guishu_list = Self->property("guipus").toList();
        QList<const General *> guishus;
        guishus << Sanguosha->getGeneral("beimihu");
        foreach(QVariant guishu, guishu_list)
            guishus << Sanguosha->getGeneral(guishu.toString());

        fillGenerals(guishus);
        show();
    }
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
        if(jj->hasLordSkill("yaoji") && jj->getMark("yaoji") == 0){
            Room *room = jj->getRoom();
            int guishunum = jj->property("guipus").toList().count();
            int deadnum = room->getPlayers().count() - room->getAlivePlayers().length();
            if(guishunum <= deadnum){
                LogMessage log;
                log.type = "#ZizhuWake";
                log.from = jj;
                log.arg = "yaoji";
                room->sendLog(log);
                room->playSkillEffect("yaoji");

                room->acquireSkill(jj, "guicai");
                room->acquireSkill(jj, "huangtian");
                const TriggerSkill *skill = Sanguosha->getTriggerSkill("huangtian");
                QVariant d;
                skill->trigger(GameStart, jj, d);
                room->setPlayerMark(jj, "yaoji", 1);
            }
        }
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        DamageStar damage = data.value<DamageStar>();
        ServerPlayer *killer = damage ? damage->from : NULL;

        if(killer && killer->hasSkill("guishu")){
            if(killer->getMark("@pu") > 0 && killer->askForSkillInvoke("guishu")){
                QVariantList guishus = killer->property("guipus").toList();
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
                QString king = general->getKingdom();
                if(king == "wei"){
                    room->playSkillEffect("guishu", 2);
                    room->setPlayerProperty(killer, "kingdom", king);
                }
                else{
                    room->playSkillEffect("guishu", 1);
                    room->setPlayerProperty(killer, "kingdom", "qun");
                }
                guishus.clear();
                guishus << guishu;
                log.type = "#GuishuLost";
                log.arg = guishu;
                room->sendLog(log);

                room->setPlayerProperty(killer, "guipus", guishus);
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

    virtual const Card *viewAs() const{
        Card *slash = new Slash(Card::NoSuit, 0);
        slash->setSkillName("yugui");
        return slash;
    }
};

class Yugui: public TriggerSkill{
public:
    Yugui():TriggerSkill("yugui"){
        events << CardUsed << CardResponsed << CardAsked << Predamage;
        view_as_skill = new YuguiViewAsSkill;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        if(event == Predamage){
            DamageStruct damage = data.value<DamageStruct>();
            if(damage.card && damage.card->inherits("Slash") &&
               damage.card->getSkillName() == objectName() && damage.to->isAlive()){
                player->getRoom()->loseHp(damage.to, damage.damage);
                return true;
            }
            return false;
        }
        if(player->getMark("@pu") < 1)
            return false;
        QVariantList guipus = player->property("guipus").toList();
        if(event == CardAsked){
            QString asked = data.toString();
            if(asked != "slash" && asked != "jink" && asked != "nullification")
                return false;
            Room *room = player->getRoom();
            if(room->askForSkillInvoke(player, objectName(), data)){
                if(asked == "slash"){
                    Slash *yugui_card = new Slash(Card::NoSuit, 0);
                    yugui_card->setSkillName(objectName());
                    room->provide(yugui_card);
                }
                else if(asked == "jink"){
                    Jink *yugui_card = new Jink(Card::NoSuit, 0);
                    yugui_card->setSkillName(objectName());
                    room->provide(yugui_card);
                }
                else if(asked == "nullification"){
                    Nullification *yugui_card = new Nullification(Card::NoSuit, 0);
                    yugui_card->setSkillName(objectName());
                    room->provide(yugui_card);
                }
                return true;
            }
            return false;
        }
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

            player->getRoom()->setPlayerProperty(player, "guipus", guipus);
            GuishuEffect::YaojiWake(player);
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
    yuejin->addSkill(new GuiouPro);
    yuejin->addSkill(new Xiaoguo);
    related_skills.insertMulti("guiou", "#guioupro");

    General *xunyou = new General(this, "xunyou", "wei", 3);
    xunyou->addSkill(new Huace);
    xunyou->addSkill(new Baiuu);
    xunyou->addSkill(new Skill("shibiao", Skill::Compulsory));

    General *wangyun = new General(this, "wangyun", "qun", 3);
    wangyun->addSkill(new Zhonglian);
    wangyun->addSkill(new Mingwang);
    wangyun->addSkill(new Lixin);
/*
    General *lvlingqi = new General(this, "lvlingqi", "qun", 3, false);
    lvlingqi->addSkill(new Dancer);
    lvlingqi->addSkill(new Fuckmoon);
*/
    General *beimihu = new General(this, "beimihu$", "qun", 3, false);
    beimihu->addSkill(new Guishu);
    beimihu->addSkill(new GuishuEffect);
    beimihu->addSkill(new Yugui);
    beimihu->addSkill(new Skill("yaoji$", Skill::Wake));
    related_skills.insertMulti("guishu", "#guishu-effect");

    addMetaObject<GuiouCard>();
    addMetaObject<ZhonglianCard>();
    addMetaObject<MingwangCard>();
}

ADD_PACKAGE(Peasa);