#include "general.h"
#include "standard.h"
#include "skill.h"
#include "engine.h"
#include "client.h"
#include "carditem.h"
#include "serverplayer.h"
#include "room.h"
#include "standard-skillcards.h"
#include "ai.h"

class Hujia:public TriggerSkill{
public:
    Hujia():TriggerSkill("hujia$"){
        events << CardAsked;
        default_choice = "ignore";
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasLordSkill("hujia");
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *player, QVariant &data) const{
        QString pattern = data.toString();
        if(pattern != "jink")
            return false;

        Room *room = player->getRoom();
        QList<ServerPlayer *> lieges = room->getLieges("wei", player);
        if(lieges.isEmpty())
            return false;

        if(!room->askForSkillInvoke(player, objectName()))
            return false;

        room->playSkillEffect(objectName());
        QVariant tohelp = QVariant::fromValue((PlayerStar)player);
        foreach(ServerPlayer *liege, lieges){
            const Card *jink = room->askForCard(liege, "jink", "@hujia-jink:" + player->objectName(), tohelp);
            if(jink){
                room->provide(jink);
                return true;
            }
        }

        return false;
    }
};

class TuxiViewAsSkill: public ZeroCardViewAsSkill{
public:
    TuxiViewAsSkill():ZeroCardViewAsSkill("tuxi"){
    }

    virtual const Card *viewAs() const{
        return new TuxiCard;
    }

protected:
    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern == "@@tuxi";
    }
};

class Tuxi:public PhaseChangeSkill{
public:
    Tuxi():PhaseChangeSkill("tuxi"){
        view_as_skill = new TuxiViewAsSkill;
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

            if(can_invoke && room->askForUseCard(zhangliao, "@@tuxi", "@tuxi-card"))
                return true;
        }

        return false;
    }
};

class Tiandu:public TriggerSkill{
public:
    Tiandu():TriggerSkill("tiandu"){
        frequency = Frequent;

        events << FinishJudge;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *guojia, QVariant &data) const{
        JudgeStar judge = data.value<JudgeStar>();
        CardStar card = judge->card;

        QVariant data_card = QVariant::fromValue(card);
        Room *room = guojia->getRoom();
        if(guojia->askForSkillInvoke(objectName(), data_card)){
            if(card->objectName() == "shit"){
                QString result = room->askForChoice(guojia, objectName(), "yes+no");
                if(result == "no")
                    return false;
            }

            guojia->obtainCard(judge->card);
            room->playSkillEffect(objectName());

            return true;
        }

        return false;
    }
};

class Yiji:public MasochismSkill{
public:
    Yiji():MasochismSkill("yiji"){
        frequency = Frequent;
    }

    virtual void onDamaged(ServerPlayer *guojia, const DamageStruct &damage) const{
        Room *room = guojia->getRoom();

        if(!room->askForSkillInvoke(guojia, objectName()))
            return;

        room->playSkillEffect(objectName());

        int n = !guojia->hasArmorEffect("linctus")? 2 : 3;

        int x = damage.damage, i;
        for(i=0; i<x; i++){
            guojia->drawCards(n);
            QList<int> yiji_cards = guojia->handCards().mid(guojia->getHandcardNum() - n);

            while(room->askForYiji(guojia, yiji_cards))
                ; // empty loop
        }

    }
};

class Fankui:public MasochismSkill{
public:
    Fankui():MasochismSkill("fankui"){

    }

    virtual void onDamaged(ServerPlayer *simayi, const DamageStruct &damage) const{
        ServerPlayer *from = damage.from;
        Room *room = simayi->getRoom();
        QVariant data = QVariant::fromValue(from);
        if(from && !from->isNude() && room->askForSkillInvoke(simayi, "fankui", data)){
            int card_id;
            if(simayi->hasArmorEffect("corrfluid")){
                ServerPlayer *plus = room->askForPlayerChosen(simayi, room->getAlivePlayers(), "corrfluid-ask");
                card_id = room->askForCardChosen(simayi, plus, "hej", "fankui");
                if(room->getCardPlace(card_id) == Player::Hand)
                    room->moveCardTo(Sanguosha->getCard(card_id), simayi, Player::Hand, false);
                else
                    room->obtainCard(simayi, card_id);
                room->playSkillEffect(objectName());
            }
            card_id = room->askForCardChosen(simayi, from, "he", "fankui");
            if(room->getCardPlace(card_id) == Player::Hand)
                room->moveCardTo(Sanguosha->getCard(card_id), simayi, Player::Hand, false);
            else
                room->obtainCard(simayi, card_id);
            room->playSkillEffect(objectName());
        }
    }
};

class GuicaiViewAsSkill:public OneCardViewAsSkill{
public:
    GuicaiViewAsSkill():OneCardViewAsSkill(""){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern == "@guicai";
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        if(!Self->hasArmorEffect("corrfluid"))
            return !to_select->isEquipped();
        return true;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        Card *card = new GuicaiCard;
        card->addSubcard(card_item->getFilteredCard());

        return card;
    }
};

class Guicai: public TriggerSkill{
public:
    Guicai():TriggerSkill("guicai"){
        view_as_skill = new GuicaiViewAsSkill;

        events << AskForRetrial;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target) && !target->isKongcheng();
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        JudgeStar judge = data.value<JudgeStar>();

        QStringList prompt_list;
        prompt_list << "@guicai-card" << judge->who->objectName()
                << "" << judge->reason << judge->card->getEffectIdString();
        QString prompt = prompt_list.join(":");

        player->tag["Judge"] = data;
        const Card *card = room->askForCard(player, "@guicai", prompt, data);

        if(card){
            // the only difference for Guicai & Guidao
            room->throwCard(judge->card);

            judge->card = Sanguosha->getCard(card->getEffectiveId());
            room->moveCardTo(judge->card, NULL, Player::Special);

            LogMessage log;
            log.type = "$ChangedJudge";
            log.from = player;
            log.to << judge->who;
            log.card_str = card->getEffectIdString();
            room->sendLog(log);

            room->sendJudgeResult(judge);
        }

        return false;
    }
};

class LuoyiBuff: public TriggerSkill{
public:
    LuoyiBuff():TriggerSkill("#luoyi"){
        events << Predamage;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasFlag("luoyi") && target->isAlive();
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *xuchu, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();

        const Card *reason = damage.card;
        if(reason == NULL)
            return false;

        if(reason->inherits("Slash") || reason->inherits("Duel")){
            LogMessage log;
            log.type = "#LuoyiBuff";
            log.from = xuchu;
            log.to << damage.to;
            log.arg = QString::number(damage.damage);
            log.arg2 = QString::number(damage.damage + 1);
            xuchu->getRoom()->sendLog(log);

            damage.damage ++;
            data = QVariant::fromValue(damage);
        }

        return false;
    }
};

class Luoyi: public DrawCardsSkill{
public:
    Luoyi():DrawCardsSkill("luoyi"){

    }

    virtual int getDrawNum(ServerPlayer *xuchu, int n) const{
        Room *room = xuchu->getRoom();
        if(room->askForSkillInvoke(xuchu, objectName())){
            room->playSkillEffect(objectName());

            xuchu->setFlags(objectName());
            if(xuchu->hasArmorEffect("warmbaby"))
                return n;
            return n - 1;
        }else
            return n;
    }
};

class Luoshen:public TriggerSkill{
public:
    Luoshen():TriggerSkill("luoshen"){
        events << PhaseChange << FinishJudge;

        frequency = Frequent;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *zhenji, QVariant &data) const{
        if(event == PhaseChange && zhenji->getPhase() == Player::Start){
            Room *room = zhenji->getRoom();
            int num=1;
            while(zhenji->askForSkillInvoke("luoshen")){
                room->playSkillEffect(objectName());

                JudgeStruct judge;
                judge.pattern = QRegExp("(.*):(spade|club):(.*)");
                judge.good = true;
                judge.reason = objectName();
                judge.who = zhenji;

                room->judge(judge);
                if(judge.isBad()){
                    if(zhenji->hasArmorEffect("sophie") && num>0){
                        num--;
                        continue;
                    }
                    break;
                }
            }

        }else if(event == FinishJudge){
            JudgeStar judge = data.value<JudgeStar>();
            if(judge->reason == objectName()){
                if(judge->card->isBlack()){
                    zhenji->obtainCard(judge->card);
                    return true;
                }
            }
        }

        return false;
    }
};

class Qingguo:public OneCardViewAsSkill{
public:
    Qingguo():OneCardViewAsSkill("qingguo"){

    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getFilteredCard()->isBlack() && !to_select->isEquipped();
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

class RendeViewAsSkill:public ViewAsSkill{
public:
    RendeViewAsSkill():ViewAsSkill("rende"){
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(ServerInfo.GameMode == "04_1v3"
           && selected.length() + Self->getMark("rende") >= 2)
           return false;
        else
            return !to_select->isEquipped();
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.isEmpty())
            return NULL;

        RendeCard *rende_card = new RendeCard;
        rende_card->addSubcards(cards);
        return rende_card;
    }
};

class Rende: public PhaseChangeSkill{
public:
    Rende():PhaseChangeSkill("rende"){
        view_as_skill = new RendeViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target)
                && target->getPhase() == Player::NotActive
                && target->hasUsed("RendeCard");
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        target->getRoom()->setPlayerMark(target, "rende", 0);

        return false;
    }
};

class JijiangViewAsSkill:public ZeroCardViewAsSkill{
public:
    JijiangViewAsSkill():ZeroCardViewAsSkill("jijiang$"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->hasLordSkill("jijiang") && Slash::IsAvailable(player);
    }

    virtual const Card *viewAs() const{
        return new JijiangCard;
    }
};

class Jijiang: public TriggerSkill{
public:
    Jijiang():TriggerSkill("jijiang$"){
        events << CardAsked;
        default_choice = "ignore";

        view_as_skill = new JijiangViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasLordSkill("jijiang");
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *liubei, QVariant &data) const{
        QString pattern = data.toString();
        if(pattern != "slash")
            return false;

        Room *room = liubei->getRoom();
        QList<ServerPlayer *> lieges = room->getLieges("shu", liubei);
        if(lieges.isEmpty())
            return false;

        if(!room->askForSkillInvoke(liubei, objectName()))
            return false;

        room->playSkillEffect(objectName());

        QVariant tohelp = QVariant::fromValue((PlayerStar)liubei);
        foreach(ServerPlayer *liege, lieges){
            const Card *slash = room->askForCard(liege, "slash", "@jijiang-slash:" + liubei->objectName(), tohelp);
            if(slash){
                room->provide(slash);
                return true;
            }
        }

        return false;
    }
};

class Longdan:public OneCardViewAsSkill{
public:
    Longdan():OneCardViewAsSkill("longdan"){

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
                if(pattern == "slash")
                    return card->inherits("Jink");
                else if(pattern == "jink")
                    return card->inherits("Slash");
            }

        default:
            return false;
        }
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return Slash::IsAvailable(player);
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "jink" || pattern == "slash";
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getFilteredCard();
        if(card->inherits("Slash")){
            Jink *jink = new Jink(card->getSuit(), card->getNumber());
            jink->addSubcard(card);
            jink->setSkillName(objectName());
            return jink;
        }else if(card->inherits("Jink")){
            Slash *slash = new Slash(card->getSuit(), card->getNumber());
            slash->addSubcard(card);
            slash->setSkillName(objectName());
            return slash;
        }else
            return NULL;
    }
};

class Tieji:public SlashBuffSkill{
public:
    Tieji():SlashBuffSkill("tieji"){

    }

    virtual bool buff(const SlashEffectStruct &effect) const{
        ServerPlayer *machao = effect.from;

        Room *room = machao->getRoom();
        if(effect.from->askForSkillInvoke("tieji", QVariant::fromValue(effect))){
            room->playSkillEffect(objectName());
            if(effect.from->hasArmorEffect("harley")){
                room->slashResult(effect, NULL);
                return true;
            }
            JudgeStruct judge;
            judge.pattern = QRegExp("(.*):(heart|diamond):(.*)");
            judge.good = true;
            judge.reason = objectName();
            judge.who = machao;

            room->judge(judge);
            if(judge.isGood()){
                room->slashResult(effect, NULL);
                return true;
            }
        }

        return false;
    }
};

class Guanxing:public PhaseChangeSkill{
public:
    Guanxing():PhaseChangeSkill("guanxing"){
        frequency = Frequent;
    }

    virtual bool onPhaseChange(ServerPlayer *zhuge) const{
        if(zhuge->getPhase() == Player::Start &&
           zhuge->askForSkillInvoke(objectName()))
        {
            Room *room = zhuge->getRoom();
            room->playSkillEffect(objectName());

            int n = qMin(5, room->alivePlayerCount());
            if(zhuge->hasArmorEffect("telescope"))
                n++;
            room->doGuanxing(zhuge, room->getNCards(n, false), false);
        }

        return false;
    }
};

class Kongcheng: public ProhibitSkill{
public:
    Kongcheng():ProhibitSkill("kongcheng"){

    }

    virtual bool isProhibited(const Player *from, const Player *to, const Card *card) const{
        if(card->inherits("Slash") || card->inherits("Duel"))
            return to->isKongcheng();
        else
            return false;
    }
};

class KongchengEffect: public TriggerSkill{
public:
    KongchengEffect():TriggerSkill("#kongcheng-effect"){
        frequency = Compulsory;

        events << CardLost;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        if(player->isKongcheng()){
            CardMoveStar move = data.value<CardMoveStar>();
            if(move->from_place == Player::Hand)
                player->getRoom()->playSkillEffect("kongcheng");
        }

        return false;
    }
};

class Jiuyuan: public TriggerSkill{
public:
    Jiuyuan():TriggerSkill("jiuyuan$"){
        events << Dying << AskForPeachesDone << CardEffected;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasLordSkill("jiuyuan");
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *sunquan, QVariant &data) const{
        Room *room =  sunquan->getRoom();
        switch(event){
        case Dying: {
                foreach(ServerPlayer *wu, room->getOtherPlayers(sunquan)){
                    if(wu->getKingdom() == "wu"){
                        room->playSkillEffect("jiuyuan", 1);
                        break;
                    }
                }
                break;
            }

        case CardEffected: {
                CardEffectStruct effect = data.value<CardEffectStruct>();
                if(effect.card->inherits("Peach") && effect.from->getKingdom() == "wu"
                   && sunquan != effect.from && sunquan->hasFlag("dying"))
                {
                    int index = effect.from->getGeneral()->isMale() ? 2 : 3;
                    room->playSkillEffect("jiuyuan", index);
                    sunquan->setFlags("jiuyuan");

                    LogMessage log;
                    log.type = "#JiuyuanExtraRecover";
                    log.from = sunquan;
                    log.to << effect.from;
                    room->sendLog(log);

                    RecoverStruct recover;
                    recover.who = effect.from;
                    room->recover(sunquan, recover);

                    room->getThread()->delay(2000);
                }

                break;
            }

        case AskForPeachesDone:{
                if(sunquan->getHp() > 0 && sunquan->hasFlag("jiuyuan"))
                    room->playSkillEffect("jiuyuan", 4);
                sunquan->setFlags("-jiuyuan");

                break;
            }

        default:
            break;
        }

        return false;
    }
};

class Fanjian:public ZeroCardViewAsSkill{
public:
    Fanjian():ZeroCardViewAsSkill("fanjian"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->isKongcheng() && ! player->hasUsed("FanjianCard");
    }

    virtual const Card *viewAs() const{
        return new FanjianCard;
    }
};

class Keji: public TriggerSkill{
public:
    Keji():TriggerSkill("keji"){
        events << CardResponsed;

        frequency = Frequent;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *lumeng, QVariant &data) const{
        CardStar card_star = data.value<CardStar>();
        if(card_star->inherits("Slash"))
            lumeng->setFlags("keji_use_slash");

        return false;
    }
};

class KejiSkip: public PhaseChangeSkill{
public:
    KejiSkip():PhaseChangeSkill("#keji-skip"){
    }

    virtual int getPriority() const{
        return 3;
    }

    virtual bool onPhaseChange(ServerPlayer *lumeng) const{
        if(lumeng->getPhase() == Player::Start){
            lumeng->setFlags("-keji_use_slash");
        }else if(lumeng->getPhase() == Player::Discard){
            if(lumeng->hasArmorEffect("towel") && lumeng->askForSkillInvoke("keji")){
                lumeng->getRoom()->playSkillEffect("keji");
                lumeng->skip(Player::Discard);
                return true;
            }
            if(!lumeng->hasFlag("keji_use_slash") &&
               lumeng->getSlashCount() == 0 &&
               lumeng->askForSkillInvoke("keji"))
            {
                lumeng->getRoom()->playSkillEffect("keji");

                return true;
            }
        }

        return false;
    }
};

class Qixi: public OneCardViewAsSkill{
public:
    Qixi():OneCardViewAsSkill("qixi"){

    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getFilteredCard()->isBlack();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *first = card_item->getCard();
        Dismantlement *dismantlement = new Dismantlement(first->getSuit(), first->getNumber());
        dismantlement->addSubcard(first->getId());
        dismantlement->setSkillName(objectName());
        return dismantlement;
    }
};

class LiuliViewAsSkill: public OneCardViewAsSkill{
public:
    LiuliViewAsSkill():OneCardViewAsSkill("liuli"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern == "@liuli";
    }

    virtual bool viewFilter(const CardItem *) const{
        return true;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        LiuliCard *liuli_card = new LiuliCard;
        liuli_card->addSubcard(card_item->getFilteredCard());

        return liuli_card;
    }
};

class Liuli: public TriggerSkill{
public:
    Liuli():TriggerSkill("liuli"){
        view_as_skill = new LiuliViewAsSkill;

        events << CardEffected;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *daqiao, QVariant &data) const{
        Room *room = daqiao->getRoom();

        CardEffectStruct effect = data.value<CardEffectStruct>();

        if(effect.card->inherits("Slash") && !daqiao->isNude()){
            QList<ServerPlayer *> players;
            bool can_invoke = false;
            if(daqiao->hasArmorEffect("underwear") && room->alivePlayerCount() > 1){
                players = room->getOtherPlayers(daqiao);
                can_invoke = true;
            }
            else if(!daqiao->hasArmorEffect("underwear") && room->alivePlayerCount() > 2){
                players = room->getOtherPlayers(daqiao);
                players.removeOne(effect.from);

                foreach(ServerPlayer *player, players){
                  if(daqiao->inMyAttackRange(player)){
                      can_invoke = true;
                      break;
                  }
                }
            }

            if(can_invoke){
                QString prompt = "@liuli:" + effect.from->objectName();
                room->setPlayerFlag(effect.from, "slash_source");
                if(room->askForUseCard(daqiao, "@@liuli", prompt)){
                    foreach(ServerPlayer *player, players){
                        if(player->hasFlag("liuli_target")){
                            room->setPlayerFlag(effect.from, "-slash_source");
                            room->setPlayerFlag(player, "-liuli_target");
                            effect.to = player;

                            room->cardEffect(effect);
                            return true;
                        }
                    }
                }
            }
        }

        return false;
    }
};

class Jieyin: public ViewAsSkill{
public:
    Jieyin():ViewAsSkill("jieyin"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("JieyinCard");
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(selected.length() > 2)
            return false;

        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() != 2)
            return NULL;

        JieyinCard *jieyin_card = new JieyinCard();
        jieyin_card->addSubcards(cards);

        return jieyin_card;
    }
};

class Xiaoji: public TriggerSkill{
public:
    Xiaoji():TriggerSkill("xiaoji"){
        events << CardLost;

        frequency = Frequent;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *sunshangxiang, QVariant &data) const{
        CardMoveStar move = data.value<CardMoveStar>();
        if(move->from_place == Player::Equip){
            Room *room = sunshangxiang->getRoom();
            if(room->askForSkillInvoke(sunshangxiang, objectName())){
                room->playSkillEffect(objectName());
                sunshangxiang->drawCards(2);
            }
        }

        return false;
    }
};

class Lijian: public OneCardViewAsSkill{
public:
    Lijian():OneCardViewAsSkill("lijian"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("LijianCard");
    }

    virtual bool viewFilter(const CardItem *) const{
        return true;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        LijianCard *lijian_card = new LijianCard;
        lijian_card->addSubcard(card_item->getCard()->getId());

        return lijian_card;
    }
};

class Qingnang: public OneCardViewAsSkill{
public:
    Qingnang():OneCardViewAsSkill("qingnang"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("QingnangCard");
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        QingnangCard *qingnang_card = new QingnangCard;
        qingnang_card->addSubcard(card_item->getCard()->getId());

        return qingnang_card;
    }
};

class Zhizhi: public TriggerSkill{
public:
    Zhizhi():TriggerSkill("zhizhi"){
        events << SlashProceed;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        Room *room = player->getRoom();
        room->playSkillEffect(objectName());

        QString slasher = player->objectName();

        const Card *first_jink = NULL, *second_jink = NULL;
        first_jink = room->askForCard(effect.to, "jink", "@wushuang-jink-1:" + slasher);
        if(first_jink)
            second_jink = room->askForCard(effect.to, "jink", "@wushuang-jink-2:" + slasher);

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

class Shensi:public DrawCardsSkill{
public:
    Shensi():DrawCardsSkill("shensi"){
        frequency = Compulsory;
    }

    virtual int getDrawNum(ServerPlayer *, int) const{
        return 4;
    }
};

class Lingxi: public TriggerSkill{
public:
    Lingxi():TriggerSkill("lingxi"){
        events << CardLost;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        if(player->isKongcheng()){
            CardMoveStar move = data.value<CardMoveStar>();
            if(move->from_place == Player::Hand){
                Room *room = player->getRoom();
                room->playSkillEffect(objectName());
                player->drawCards(1);
            }
        }
        return false;
    }
};

class Jizhi:public DrawCardsSkill{
public:
    Jizhi():DrawCardsSkill("jizhi"){
        frequency = Compulsory;
    }

    virtual int getDrawNum(ServerPlayer *heiji, int n) const{
        int a = heiji->getLostHp();
        return n + a;
    }
};

class Kaituo:public ViewAsSkill{
public:
    Kaituo():ViewAsSkill("kaituo"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("KaituoCard");
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(selected.length() > 2)
            return false;
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() != 2)
            return NULL;
        KaituoCard *ard = new KaituoCard;
        ard->addSubcards(cards);

        return ard;
    }
};

class Boxue: public PhaseChangeSkill{
public:
    Boxue():PhaseChangeSkill("boxue"){
        frequency = Compulsory;
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        if(player->getPhase() == Player::NotActive){
            player->drawCards(player->getHp());
        }

        return false;
    }
};

class TongqingViewAsSkill: public OneCardViewAsSkill{
public:
    TongqingViewAsSkill():OneCardViewAsSkill("tongqing"){
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern.contains("peach");
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getCard()->inherits("Jink");
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *first = card_item->getCard();
        Peach *peach = new Peach(first->getSuit(), first->getNumber());
        peach->addSubcard(first->getId());
        peach->setSkillName(objectName());
        return peach;
    }
};

class Tongqing: public TriggerSkill{
public:
    Tongqing():TriggerSkill("tongqing"){
        view_as_skill = new TongqingViewAsSkill;
        events << CardUsed << CardFinished;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        CardUseStruct use = data.value<CardUseStruct>();
        if(event == CardUsed){
            if(use.card->getSkillName() == "tongqing")
                player->addMark("tongqing");
        }
        else{
            Room *room = player->getRoom();
            if(player->getMark("tongqing") > 0)
                player->drawCards(qMin(room->getAlivePlayers().length(), player->getMark("tongqing")));
        }
        return false;
    }
};

class Weiya: public TriggerSkill{
public:
    Weiya():TriggerSkill("weiya"){
        events << Damage;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.to)
            player->getRoom()->throwCard(damage.to->getRandomHandCardId());
        return false;
    }
};

class Tiemian:public MasochismSkill{
public:
    Tiemian():MasochismSkill("tiemian"){
        frequency = Compulsory;
    }

    virtual void onDamaged(ServerPlayer *player, const DamageStruct &damage) const{
        Room *room = player->getRoom();
        if(damage.from && damage.to && player->getLostHp() > damage.from->getLostHp() &&
           room->askForDiscard(damage.to, objectName(), 1)){
            DamageStruct damage;
            damage.from = player;
            damage.to = damage.from;
            room->setEmotion(player, "good");
            room->damage(damage);
        }
    }
};

class Checha: public TriggerSkill{
public:
    Checha():TriggerSkill("checha"){
        events << Death;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return !target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        ServerPlayer *miwako = player->getRoom()->findPlayerBySkillName(objectName());
        if(miwako)
            miwako->drawCards(2);
        return false;
    }
};

class Fanjie:public MasochismSkill{
public:
    Fanjie():MasochismSkill("fanjie"){
    }

    virtual void onDamaged(ServerPlayer *player, const DamageStruct &damage) const{
        Room *room = player->getRoom();
        const Card *card = damage.card;
        if(!room->obtainable(card, player))
            return;

        QVariant data = QVariant::fromValue(card);
        if(room->askForSkillInvoke(player, "jianxiong", data)){
            room->playSkillEffect(objectName());
            player->obtainCard(card);
        }
    }
};

class Gouxian:public OneCardViewAsSkill{
public:
    Gouxian():OneCardViewAsSkill("gouxian"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return Slash::IsAvailable(player);
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "slash";
    }

    virtual bool viewFilter(const CardItem *) const{
        return true;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getCard();
        Card *slash = new Slash(card->getSuit(), card->getNumber());
        slash->addSubcard(card->getId());
        slash->setSkillName(objectName());
        return slash;
    }
};

class Shexian: public ZeroCardViewAsSkill{
public:
    Shexian():ZeroCardViewAsSkill("shexian"){

    }
    virtual const Card *viewAs() const{
        return new ShexianCard;
    }
};

void StandardPackage::addGenerals(){
    General *conan = new General(this, "conan", "45s");
    conan->addSkill(new Zhizhi);

    General *ai = new General(this, "ai", "45s", 3, false);
    ai->addSkill(new Shensi);

    General *ran = new General(this, "ran", "45s", 4, false);
    ran->addSkill(new Lingxi);

    General *heiji = new General(this, "heiji", "45s");
    heiji->addSkill(new Jizhi);

    General *kazuha = new General(this, "kazuha", "45s", 4, false);
    kazuha->addSkill(new Skill("jianchi"));

    General *sonoko = new General(this, "sonoko", "45s", 4, false);
    sonoko->addSkill(new Kaituo);
    addMetaObject<KaituoCard>();

    General *hiroshi = new General(this, "hiroshi", "45s");
    hiroshi->addSkill(new Boxue);

    General *tanteitan = new General(this, "tanteitan", "45s");
    tanteitan->addSkill(new Tongqing);

    General *kogorou = new General(this, "kogorou", "45s");
    kogorou->addSkill(new Weiya);

    General *takagi = new General(this, "kogorou", "45s", 5);
    takagi->addSkill(new Skill("zhengzhi"));

    General *sato = new General(this, "sato", "45s", 5);
    sato->addSkill(new Checha);

    General *meguri = new General(this, "meguri", "45s");
    meguri->addSkill(new Tiemian);

    General *akai = new General(this, "akai", "45s");
    akai->addSkill(new Skill("zhuisuo"));

    General *jodie = new General(this, "jodie", "45s", 4, false);
    jodie->addSkill(new Fanjie);

    General *gin = new General(this, "gin", "45s");
    gin->addSkill(new Gouxian);

    General *vodka = new General(this, "vodka", "45s");
    vodka->addSkill(new Shexian);
    addMetaObject<ShexianCard>();



    General *liubei, *zhaoyun, *machao, *zhugeliang;
    liubei = new General(this, "liubei$", "shu");
    liubei->addSkill(new Rende);
    liubei->addSkill(new Jijiang);

    zhugeliang = new General(this, "zhugeliang", "shu", 3);
    zhugeliang->addSkill(new Guanxing);
    zhugeliang->addSkill(new Kongcheng);
    zhugeliang->addSkill(new KongchengEffect);
    related_skills.insertMulti("kongcheng", "#kongcheng-effect");

    zhaoyun = new General(this, "zhaoyun", "shu");
    zhaoyun->addSkill(new Longdan);

    machao = new General(this, "machao", "shu");
    machao->addSkill(new Tieji);

    General *sunquan, *zhouyu, *lumeng, *ganning, *daqiao, *sunshangxiang;
    sunquan = new General(this, "sunquan$", "wu");
    sunquan->addSkill(new Jiuyuan);

    ganning = new General(this, "ganning", "wu");
    ganning->addSkill(new Qixi);

    lumeng = new General(this, "lumeng", "wu");
    lumeng->addSkill(new Keji);
    lumeng->addSkill(new KejiSkip);
    related_skills.insertMulti("keji", "#keji-skip");

    zhouyu = new General(this, "zhouyu", "wu", 3);
    zhouyu->addSkill(new Fanjian);

    daqiao = new General(this, "daqiao", "wu", 3, false);
    daqiao->addSkill(new Liuli);

    sunshangxiang = new General(this, "sunshangxiang", "wu", 3, false);
    sunshangxiang->addSkill(new Jieyin);
    sunshangxiang->addSkill(new Xiaoji);

    General *huatuo, *diaochan;

    huatuo = new General(this, "huatuo", "qun", 3);
    huatuo->addSkill(new Qingnang);

    diaochan = new General(this, "diaochan", "qun", 3, false);
    diaochan->addSkill(new Lijian);

    // for skill cards
    addMetaObject<RendeCard>();
    addMetaObject<TuxiCard>();
    addMetaObject<JieyinCard>();
    addMetaObject<LijianCard>();
    addMetaObject<FanjianCard>();
    addMetaObject<GuicaiCard>();
    addMetaObject<QingnangCard>();
    addMetaObject<LiuliCard>();
    addMetaObject<JijiangCard>();
    addMetaObject<HuanzhuangCard>();
    addMetaObject<CheatCard>();
}

TestPackage::TestPackage()
    :Package("test")
{
    new General(this, "sujiang", "god", 5, true, true);
    new General(this, "sujiangf", "god", 5, false, true);
}

ADD_PACKAGE(Test)
