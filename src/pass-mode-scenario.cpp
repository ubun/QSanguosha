#include "pass-mode-scenario.h"
#include "engine.h"
#include "standard-skillcards.h"
#include "clientplayer.h"
#include "client.h"
#include "carditem.h"

class PQingxin: public PhaseChangeSkill{
public:
    PQingxin():PhaseChangeSkill("pt_qingxin"){

    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target)
                && target->getPhase() == Player::Start
                && !target->getJudgingArea().isEmpty();
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        if(!target->askForSkillInvoke(objectName()))
            return false;

        Room *room = target->getRoom();

        int card_id = room->askForCardChosen(target, target, "j", objectName());
        const Card *card = Sanguosha->getCard(card_id);
        room->loseHp(target);
        room->throwCard(card);
        return false;
    }
};

class PDuanyan:public ZeroCardViewAsSkill{
public:
    PDuanyan():ZeroCardViewAsSkill("pt_duanyan"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("PDuanyanCard");
    }

    virtual const Card *viewAs() const{
        return new PDuanyanCard;
    }
};

PDuanyanCard::PDuanyanCard(){
    once = true;
}

bool PDuanyanCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && !to_select->isKongcheng() && to_select != Self ;
}

void PDuanyanCard::onEffect(const CardEffectStruct &effect) const{
    ServerPlayer *player = effect.from;
    ServerPlayer *target = effect.to;
    Room *room = player->getRoom();

    QString choice = room->askForChoice(player, "pt_duanyan", "slash+jink+peatic+other");

    int card_id = target->getRandomHandCardId();
    const Card *card = Sanguosha->getCard(card_id);
    LogMessage log;
    log.type = "#PChooseType";
    log.from = player;
    log.to << target;
    log.arg = choice;
    room->sendLog(log);

    room->showCard(target, card_id);
    room->getThread()->delay();

    bool correct ;
    bool isSlash = card->inherits("Slash") ;
    bool isJink = card->inherits("Jink") ;
    bool isPna = card->inherits("Peach") || card->inherits("Analeptic") ;

    if(choice == "slash"){
        correct = isSlash ;
    }else if(choice == "jink"){
        correct = isJink ;
    }else if(choice == "peatic"){
        correct = isPna ;
    }else{
        correct = !(isSlash || isJink || isPna);
    }

    if(correct){
        room->throwCard(card_id);
    }else{
        if(!room->askForDiscard(player, objectName(), 1, true , true)){
            player->drawCards(1);
            DamageStruct damage;
            damage.card = NULL;
            damage.from = target;
            damage.to = player;
            room->damage(damage);
        }
    }
}

class PShiqi:public TriggerSkill{
public:
    PShiqi():TriggerSkill("ps_shiqi"){
        events << PhaseChange << FinishJudge;
        frequency = Frequent;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *shizu, QVariant &data) const{
        if(event == PhaseChange && shizu->getPhase() == Player::Draw){
            Room *room = shizu->getRoom();
            if(shizu->askForSkillInvoke("ps_shiqi")){
                shizu->setFlags("ps_shiqi");
                room->playSkillEffect(objectName());

                JudgeStruct judge;
                judge.pattern = QRegExp("(.*):(heart|diamond):(.*)");
                judge.good = true;
                judge.reason = objectName();
                judge.who = shizu;

                room->judge(judge);
            }
            shizu->setFlags("-shiqi");
        }else if(event == FinishJudge){
            if(shizu->hasFlag("ps_shiqi")){
                JudgeStar judge = data.value<JudgeStar>();
                if(judge->card->isRed()){
                    shizu->obtainCard(judge->card);
                    return true;
                }
            }
        }
        return false;
    }
};

class PQianggong: public SlashBuffSkill{
public:
    PQianggong():SlashBuffSkill("ps_qianggong"){

    }

    virtual bool buff(const SlashEffectStruct &effect) const{
        ServerPlayer *gongshou = effect.from;
        Room *room = gongshou->getRoom();

        if(gongshou->getHp() <= 1 || gongshou->getAttackRange() > 3){
            if(gongshou->askForSkillInvoke("liegong", QVariant::fromValue(effect))){
                room->playSkillEffect("liegong");
                room->slashResult(effect, NULL);

                return true;
            }
        }
        return false;
    }
};


class PPojia: public TriggerSkill{
public:
    PPojia():TriggerSkill("ps_pojia"){
        events << SlashEffect << SlashMissed;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        if(event == SlashEffect){
            if(effect.slash->isBlack()){
                effect.to->addMark("qinggang");
            }
        }else if(event == SlashMissed){
            if(!effect.slash->isRed())
                return false;
            if(effect.to->hasSkill("kongcheng") && effect.to->isKongcheng())
                return false;
            Room *room = player->getRoom();
            const Card *card = room->askForCard(player, "slash", "blade-slash");
            if(card){
                // if player is drank, unset his flag
                if(player->hasFlag("drank"))
                    room->setPlayerFlag(player, "-drank");

                CardUseStruct use;
                use.card = card;
                use.from = player;
                use.to << effect.to;
                room->useCard(use, false);
            }
        }
        return false;
    }
};


class PQishu: public DistanceSkill{
public:
    PQishu():DistanceSkill("ps_qishu"){

    }

    virtual int getCorrect(const Player *from, const Player *to) const{
        int correct = 0;
        if(from->hasSkill(objectName())){
            correct --;
            if(from->getOffensiveHorse() != NULL)
                correct -= from->getOffensiveHorse()->getCorrect() ;
        }
        if(to->hasSkill(objectName())){
            correct ++;
            if(to->getDefensiveHorse() != NULL)
                correct -= to->getDefensiveHorse()->getCorrect() ;
        }

        return correct;
    }
};

class PChenwen: public TriggerSkill{
public:
    PChenwen():TriggerSkill("ps_chenwen"){
        events << SlashEffected;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target) && target->getArmor() == NULL;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        SlashEffectStruct effect = data.value<SlashEffectStruct>();

        if(effect.slash->getSuit() == Card::Club){
            player->getRoom()->playSkillEffect(objectName());

            LogMessage log;
            log.type = "#SkillNullify";
            log.from = player;
            log.arg = objectName();
            log.arg2 = effect.slash->objectName();

            player->getRoom()->sendLog(log);

            return true;
        }

        return false;
    }
};

class Phongzhuang: public TriggerSkill{
public:
    Phongzhuang():TriggerSkill("ps_zhongzhuang"){
        events << Predamaged;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.damage > 1){
            LogMessage log;
            log.type = "#Phongzhuang";
            log.from = player;
            log.arg = QString::number(damage.damage);
            player->getRoom()->sendLog(log);

            damage.damage = 1;
            data = QVariant::fromValue(damage);
        }

        return false;
    }
};


class PYaoshu: public OneCardViewAsSkill{
public:
    PYaoshu():OneCardViewAsSkill("ps_yaoshu"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("PYaoshuCard");
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped() && to_select->getCard()->getSuit() == Card::Spade;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        PYaoshuCard *yaoshu_card = new PYaoshuCard;
        yaoshu_card->addSubcard(card_item->getCard()->getId());

        return yaoshu_card;
    }
};

PYaoshuCard::PYaoshuCard(){
    once = true;
}

bool PYaoshuCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty();
}

void PYaoshuCard::onEffect(const CardEffectStruct &effect) const{
    ServerPlayer *yaodao = effect.from;
    ServerPlayer *target = effect.to;

    Room *room = yaodao->getRoom();
    room->setEmotion(target, "bad");

    JudgeStruct judge;
    judge.pattern = QRegExp("(.*):(spade|club):([2-9])");
    judge.good = false;
    judge.reason = "leiji";
    judge.who = target;

    room->judge(judge);

    if(judge.isBad()){
        DamageStruct damage;
        damage.card = NULL;
        damage.damage = 1;
        damage.from = yaodao;
        damage.to = target;
        damage.nature = DamageStruct::Thunder;

        room->damage(damage);
    }else
        room->setEmotion(yaodao, "bad");
}



class PJitian: public TriggerSkill{
public:
    PJitian():TriggerSkill("ps_jitian"){
        events << Predamaged;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.nature == DamageStruct::Thunder){
            LogMessage log;
            log.type = "#PJitian";
            log.from = player;
            player->getRoom()->sendLog(log);

            damage.damage = 0;
            data = QVariant::fromValue(damage);
        }

        return false;
    }
};

class RendePassViewAsSkill:public ViewAsSkill{
public:
    RendePassViewAsSkill():ViewAsSkill("rende_pass"){
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        return true ;
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.isEmpty())
            return NULL;

        RendePassCard *rende_pass_card = new RendePassCard;
        rende_pass_card->addSubcards(cards);
        return rende_pass_card;
    }
};

class RendePass: public PhaseChangeSkill{
public:
    RendePass():PhaseChangeSkill("rende_pass"){
        view_as_skill = new RendePassViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target)
                && target->getPhase() == Player::NotActive
                && target->hasUsed("RendePassCard");
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        target->getRoom()->setPlayerMark(target, "rende_pass", 0);
        return false;
    }
};


RendePassCard::RendePassCard(){
    will_throw = false;
}

void RendePassCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    if(targets.isEmpty())
        return;

    ServerPlayer *target = targets.first();
    room->moveCardTo(this, target, Player::Hand, false);

    int old_value = source->getMark("rende_pass");
    int new_value = old_value + subcards.length();
    room->setPlayerMark(source, "rende_pass", new_value);
    room->playSkillEffect("rende");

    if(old_value < 1 && new_value >= 1){
        RecoverStruct recover;
        recover.card = this;
        recover.who = source;
        room->recover(source, recover);
    }
}


class JijiangPass: public ZeroCardViewAsSkill{
public:
    JijiangPass():ZeroCardViewAsSkill("jijiang_pass"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return (player->canSlashWithoutCrossbow() || player->hasWeapon("crossbow")) && !player->hasUsed("JijiangPassCard");
    }

    virtual const Card *viewAs() const{
        return new JijiangPassCard;
    }
};


JijiangPassCard::JijiangPassCard(){
    mute = true;
}

bool JijiangPassCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;

    if(to_select->hasSkill("kongcheng") && to_select->isKongcheng())
        return false;

    if(to_select == Self)
        return false;

    return true;
}

void JijiangPassCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    room->playSkillEffect("jijiang");
    Slash *slash = new Slash(Card::NoSuit, 0);
    slash->setSkillName("jijiang_pass");
    CardUseStruct use;
    use.card = slash;
    use.from = source;
    use.to = targets;

    room->useCard(use);
}


class PWenjiu:public TriggerSkill{
public:
    PWenjiu():TriggerSkill("pwenjiu"){
        events << PhaseChange ;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *guanyu, QVariant &data) const{
        Room *room = guanyu->getRoom() ;
        if(event == PhaseChange){
            if(guanyu->getPhase() == Player::Start){
                if(guanyu->askForSkillInvoke("pwenjiu")){
                    room->setPlayerFlag(guanyu, "pwenjiu");
                    guanyu->gainMark("@wenjiu",2);
                    guanyu->drawCards(1);
                }
                if(guanyu->getMark("@wenjiu") > 0){
                    guanyu->setFlags("luoyi");
                }
            }else if(guanyu->getPhase() == Player::Finish){
                if(guanyu->getMark("@wenjiu") > 0){
                    guanyu->loseMark("@wenjiu");
                }
                if(guanyu->hasFlag("pwenjiu")){
                    room->setPlayerFlag(guanyu, "-wenjiu");
                }
            }
        }
        return false;
    }
};


class PBaonu: public TriggerSkill{
public:
    PBaonu():TriggerSkill("pbaonu"){
        events << Predamage;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target) && target->getLostHp() >= 3 && target->isAlive();
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *zhangfei, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();

        const Card *reason = damage.card;
        if(reason == NULL)
            return false;

        if(reason->inherits("Slash") || reason->inherits("Duel")){
            LogMessage log;
            log.type = "#PBaonu";
            log.from = zhangfei;
            log.to << damage.to;
            log.arg = QString::number(damage.damage);
            log.arg2 = QString::number(damage.damage + 1);
            zhangfei->getRoom()->sendLog(log);

            damage.damage ++;
            data = QVariant::fromValue(damage);
        }
        return false;
    }
};

class TiejiPass:public TriggerSkill{
public:
    TiejiPass():TriggerSkill("tieji_pass"){
        events << SlashProceed << Predamage;
    }
    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        if(event == SlashProceed){
            if(data.canConvert<SlashEffectStruct>()){
                SlashEffectStruct effect = data.value<SlashEffectStruct>();

                if(player->isAlive()){
                    ServerPlayer *machao = effect.from;

                    Room *room = machao->getRoom();
                    if(effect.from->askForSkillInvoke("tieji", QVariant::fromValue(effect))){
                        room->playSkillEffect("tieji");

                        JudgeStruct judge;
                        judge.pattern = QRegExp("(.*):(heart|diamond):(.*)");
                        judge.good = true;
                        judge.reason = objectName();
                        judge.who = machao;

                        room->judge(judge);
                        if(judge.isGood()){
                            if(judge.card->getSuit() == Card::Diamond){
                                effect.to->setFlags("tieji_damage");
                            }else if(judge.card->getSuit() == Card::Heart){
                                machao->obtainCard(judge.card);
                            }
                            room->slashResult(effect, NULL);
                            return true;
                        }
                    }
                    return false;
                }
            }
        }else if(event == Predamage){
            DamageStruct damage = data.value<DamageStruct>();
            if(damage.to->hasFlag("tieji_damage")){
                damage.damage ++ ;
                data = QVariant::fromValue(damage);
                damage.to->setFlags("-tieji_damage");
            }
        }
        return false;
    }
};


class MashuPass: public DistanceSkill{
public:
    MashuPass():DistanceSkill("mashu_pass"){

    }

    virtual int getCorrect(const Player *from, const Player *to) const{
        int correct = 0;
        if(from->hasSkill(objectName())){
            correct -- ;
        }
        if(to->hasSkill(objectName())){
            if(to->getOffensiveHorse() != NULL)
                correct ++ ;
            if(to->getDefensiveHorse() != NULL)
                correct ++ ;
        }
        return correct;
    }
};


class PLonghou: public TriggerSkill{
public:
    PLonghou():TriggerSkill("plonghou"){
        events << SlashHit;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *zhaoyun, QVariant &data) const{
        SlashEffectStruct effect = data.value<SlashEffectStruct>();

        Room *room = zhaoyun->getRoom();

        if(effect.to->getWeapon() != NULL && zhaoyun->askForSkillInvoke(objectName(), QVariant::fromValue(effect.to))){
            room->moveCardTo(effect.to->getWeapon(),zhaoyun,Player::Hand);
        }
        return false;
    }
};

class JizhiPass:public TriggerSkill{
public:
    JizhiPass():TriggerSkill("jizhi_pass"){
        frequency = Frequent;
        events << CardUsed << CardResponsed;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *yueying, QVariant &data) const{
        CardStar card = NULL;
        if(event == CardUsed){
            CardUseStruct use = data.value<CardUseStruct>();
            card = use.card;
        }else if(event == CardResponsed)
            card = data.value<CardStar>();

        if(card->inherits("TrickCard")){
            Room *room = yueying->getRoom();
            if(room->askForSkillInvoke(yueying, objectName())){
                room->playSkillEffect("jizhi");
                yueying->drawCards(1);
            }
        }
        return false;
    }
};



class JianxiongPass:public MasochismSkill{
public:
    JianxiongPass():MasochismSkill("jianxiong_pass"){
    }

    virtual void onDamaged(ServerPlayer *caocao, const DamageStruct &damage) const{
        Room *room = caocao->getRoom();
        const Card *card = damage.card;
        if(!room->obtainable(card, caocao))
            return;

        QVariant data = QVariant::fromValue(card);
        room->playSkillEffect("jianxiong"); 
        if(room->askForSkillInvoke(caocao, "jianxiong", data)){
            caocao->obtainCard(card);
        }else{
            caocao->drawCards(1);
        }
        caocao->gainMark("@jianxiong");
    }
};

class PXietian: public TriggerSkill{
public:
    PXietian():TriggerSkill("pxietian"){
        frequency = Frequent;
        events << Predamage;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *caocao, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();

        const Card *reason = damage.card;
        if(reason == NULL)
            return false;
        Room *room = caocao->getRoom();
        if(reason->inherits("AOE") && room->askForSkillInvoke(caocao, objectName(), data)){
            LogMessage log;
            log.from = caocao;
            log.to << damage.to;
            if(!damage.to->isKongcheng()){
                const Card *card = damage.to->getRandomHandCard();
                room->moveCardTo(card, caocao, Player::Hand, false);
                log.type = "#PXietian1";
            }else{
                damage.damage ++;
                data = QVariant::fromValue(damage);
                log.type = "#PXietian2";
                log.arg = damage.card->getName() ;
                log.arg2 = damage.damage ;
            }
            caocao->getRoom()->sendLog(log);
        }
        return false;
    }
};

class PBadao:public OneCardViewAsSkill{
public:
    PBadao():OneCardViewAsSkill("pbadao"){
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped();
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getMark("@jianxiong") >= 3 ;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        Card *card ;
        const Card *sub_card = card_item->getFilteredCard() ;
        if(sub_card->isRed()){
            card = new ArcheryAttack(sub_card->getSuit(), sub_card->getNumber());
        }else{
            card = new SavageAssault(sub_card->getSuit(), sub_card->getNumber());
        }
        card->addSubcard(sub_card);
        card->setSkillName(objectName());
        return card;
    }
};

class PBadaoCost: public TriggerSkill{
public:
    PBadaoCost():TriggerSkill("#badao_cost"){
        events << CardUsed;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *caocao, QVariant &data) const{
        CardUseStruct use = data.value<CardUseStruct>();
        if(use.card->getSkillName() == "pbadao"){
            caocao->loseMark("@jianxiong",3);
        }
        return false ;
    }
};


class PLanggu: public PhaseChangeSkill{
public:
    PLanggu():PhaseChangeSkill("planggu"){
        frequency = Frequent;
    }

    virtual bool onPhaseChange(ServerPlayer *simayi) const{
        if(simayi->getPhase() == Player::Finish){
            Room *room = simayi->getRoom();
            int n = 0 ;
            foreach(ServerPlayer *player, room->getAlivePlayers()){
                n = qMax(n,player->getHandcardNum());
            }
            if(n > simayi->getHandcardNum() && room->askForSkillInvoke(simayi, objectName())){
                simayi->drawCards(n - simayi->getHandcardNum());
            }
        }
        return false;
    }
};


class FankuiPass:public MasochismSkill{
public:
    FankuiPass():MasochismSkill("fankui_pass"){

    }

    virtual void onDamaged(ServerPlayer *simayi, const DamageStruct &damage) const{
        ServerPlayer *from = damage.from;
        Room *room = simayi->getRoom();
        QVariant data = QVariant::fromValue(from);
        if(from && room->askForSkillInvoke(simayi, "fankui", data)){
            int x = damage.damage, i;
            for(i=0; i<x; i++){
                if(!from->isNude()){
                    int card_id = room->askForCardChosen(simayi, from, "he", "fankui");
                    if(room->getCardPlace(card_id) == Player::Hand)
                        room->moveCardTo(Sanguosha->getCard(card_id), simayi, Player::Hand, false);
                    else
                        room->obtainCard(simayi, card_id);
                }else{
                    DamageStruct damage;
                    damage.from = simayi;
                    damage.to = from;
                    room->damage(damage);
                    break ;
                }
            }
            room->playSkillEffect("fankui");
        }
    }
};

class GangliePass:public MasochismSkill{
public:
    GangliePass():MasochismSkill("ganglie_pass"){

    }

    virtual void onDamaged(ServerPlayer *xiahou, const DamageStruct &damage) const{
        ServerPlayer *from = damage.from;
        Room *room = xiahou->getRoom();
        QVariant source = QVariant::fromValue(from);

        if(from && from->isAlive() && room->askForSkillInvoke(xiahou, "ganglie", source)){
            room->playSkillEffect("ganglie");

            JudgeStruct judge;
            QString num_str = QString("A23456789").left(xiahou->getHp());
            judge.pattern = QRegExp(QString("(.*):(.*):([%1])").arg(num_str)) ;
            judge.good = false;
            judge.reason = objectName();
            judge.who = xiahou;

            room->judge(judge);
            if(judge.isGood()){
                if(!room->askForDiscard(from, "ganglie", 2, true)){
                    DamageStruct damage;
                    damage.from = xiahou;
                    damage.to = from;
                    if(judge.card->getNumber() > xiahou->getHp() + 7)
                        damage.damage ++ ;
                    room->setEmotion(xiahou, "good");
                    room->damage(damage);
                }
            }else{
                room->setEmotion(xiahou, "bad");
                xiahou->obtainCard(judge.card);
            }
        }
    }
};

class PJueduan: public TriggerSkill{
public:
    PJueduan():TriggerSkill("pjueduan"){
        events << Damage;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *xiahou, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.card && damage.card->inherits("Slash") && damage.to->isAlive() && !damage.to->isNude()){
            Room *room = xiahou->getRoom();
            if(room->askForSkillInvoke(xiahou, objectName(),QVariant::fromValue(damage.to))){
                int card_id = room->askForCardChosen(xiahou, damage.to, "he", objectName());
                room->throwCard(card_id);
            }
        }

        return false;
    }
};


class YijiPass:public TriggerSkill{
public:
    YijiPass():TriggerSkill("yiji_pass"){
        frequency = Frequent;
        events << Predamaged << Damaged << AskForPeachesDone;
    }
    virtual bool trigger(TriggerEvent event, ServerPlayer *guojia, QVariant &data) const{
        if(event == Predamaged){
            if(guojia->hasFlag("yiji_dying"))
                guojia->setFlags("-yiji_dying");
        }else if(event == Damaged){
            DamageStruct damage = data.value<DamageStruct>();
            if(guojia->isAlive()){
                Room *room = guojia->getRoom();

                if(!room->askForSkillInvoke(guojia, objectName()))
                    return false;

                //room->playSkillEffect(objectName());
                int n = damage.damage * (guojia->hasFlag("yiji_dying") ? 3 : 2);
                guojia->drawCards(n);
                guojia->setFlags("-yiji_dying");
            }
        }else if(event == AskForPeachesDone){
            if(guojia->getHp() > 0)
                guojia->setFlags("yiji_dying");
        }
        return false;
    }
};

class PGuimou:public TriggerSkill{
public:
    PGuimou():TriggerSkill("pguimou"){
        events << FinishJudge;
        frequency = Frequent;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true ;
    }

    virtual int getPriority() const{
        return 3;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *guojia = room->findPlayerBySkillName(objectName());

        if(guojia == NULL)
            return false;

        JudgeStar judge = data.value<JudgeStar>();
        if(judge->card->isRed() && judge->card->getNumber() > guojia->getHp() * 2 && guojia->askForSkillInvoke(objectName(), data)){
            LogMessage log;
            log.type = "#PGuimou";
            log.from = player;
            log.to << guojia;
            room->sendLog(log);
            guojia->drawCards(1);
        }
        return false;
    }
};

class LuoshenPass:public TriggerSkill{
public:
    LuoshenPass():TriggerSkill("luoshen_pass"){
        events << PhaseChange << FinishJudge;
        frequency = Frequent;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *zhenji, QVariant &data) const{
        if(event == PhaseChange && zhenji->getPhase() == Player::Start){
            Room *room = zhenji->getRoom();
            while(zhenji->askForSkillInvoke(objectName())){
                zhenji->setFlags("luoshen");
                room->playSkillEffect("luoshen");

                JudgeStruct judge;
                judge.pattern = QRegExp("(.*):(spade|club):(.*)");
                judge.good = true;
                judge.reason = objectName();
                judge.who = zhenji;

                room->judge(judge);
                if(judge.isBad())
                    break;
            }

            zhenji->setFlags("-luoshen");
        }else if(event == FinishJudge){
            if(zhenji->hasFlag("luoshen")){
                JudgeStar judge = data.value<JudgeStar>();
                zhenji->obtainCard(judge->card);
                return true;
            }
        }

        return false;
    }
};

class LuoyiPass:public ViewAsSkill{
public:
    LuoyiPass():ViewAsSkill("luoyi_pass"){

    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(selected.length() >= 2){
            return false ;
        }else if(selected.length() == 1){
            return ! (selected.first()->isEquipped() || to_select->isEquipped()) ;
        }else{
            return true ;
        }
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.isEmpty())
            return NULL;
        if(cards.length()== 1 && !cards.first()->getCard()->inherits("EquipCard"))
            return NULL;
        LuoyiPassCard *luoyi_pass_card = new LuoyiPassCard;
        luoyi_pass_card->addSubcards(cards);

        return luoyi_pass_card;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("LuoyiPassCard");
    }
};


LuoyiPassCard::LuoyiPassCard(){
    target_fixed = true;
    once = true;
}

void LuoyiPassCard::use(Room *room, ServerPlayer *xuchu, const QList<ServerPlayer *> &) const{
    room->throwCard(this);
    xuchu->setFlags("luoyi");
    room->playSkillEffect("luoyi");
}



class PGuantong: public TriggerSkill{
public:
    PGuantong():TriggerSkill("pguantong"){
        events << DamageComplete;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();

        if(damage.from == NULL)
            return false;

        if(!damage.from->hasSkill(objectName()))
            return false;

        if(!damage.card->inherits("Slash") && !damage.card->inherits("Duel"))
            return false;

        ServerPlayer *xuchu = damage.from;
        Room *room = xuchu->getRoom();
        if(!xuchu->isKongcheng() && xuchu->askForSkillInvoke(objectName(), data))
        {
            room->askForDiscard(xuchu, objectName(), 1) ;
            damage.to = player->getNextAlive();
            damage.nature = DamageStruct::Normal ;
            damage.damage = 1 ;
            room->damage(damage);
        }
        return false;
    }
};


class TuxiPassViewAsSkill: public ZeroCardViewAsSkill{
public:
    TuxiPassViewAsSkill():ZeroCardViewAsSkill("tuxi_pass"){
    }

    virtual const Card *viewAs() const{
        return new TuxiPassCard;
    }

protected:
    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern == "@@tuxi_pass";
    }
};

class TuxiPass:public PhaseChangeSkill{
public:
    TuxiPass():PhaseChangeSkill("tuxi_pass"){
        view_as_skill = new TuxiPassViewAsSkill;
    }

    virtual bool onPhaseChange(ServerPlayer *zhangliao) const{
        if(zhangliao->getPhase() == Player::Draw){
            Room *room = zhangliao->getRoom();
            bool can_invoke = false;
            QList<ServerPlayer *> other_players = room->getOtherPlayers(zhangliao);
            foreach(ServerPlayer *player, other_players){
                if(!player->isNude()){
                    can_invoke = true;
                    break;
                }
            }

            if(can_invoke && room->askForUseCard(zhangliao, "@@tuxi_pass", "@tuxi-card"))
                return true;
        }

        return false;
    }
};

TuxiPassCard::TuxiPassCard(){
}

bool TuxiPassCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(targets.length() >= 2)
        return false;

    if(to_select == Self)
        return false;

    return true ;
}

void TuxiPassCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    if(! effect.to->isNude()){
        int card_id = room->askForCardChosen(effect.from, effect.to, "he", "tuxi");
        const Card *card = Sanguosha->getCard(card_id);
        room->moveCardTo(card, effect.from, Player::Hand, false);
    }else{
        DamageStruct damage;
        damage.card = NULL;
        damage.from = effect.from;
        damage.to = effect.to;

        room->damage(damage);
    }
    room->setEmotion(effect.to, "bad");
    room->setEmotion(effect.from, "good");
}

void TuxiPassCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
   room->playSkillEffect("tuxi");
   foreach(ServerPlayer *target, targets){
       CardEffectStruct effect;
       effect.card = this;
       effect.from = source;
       effect.to = target;
       effect.multiple = true;

       room->cardEffect(effect);
   }
   if(targets.length() < 2)
       source->drawCards(2-targets.length());
}

class ZhihengPass:public ViewAsSkill{
public:
    ZhihengPass():ViewAsSkill("zhiheng_pass"){

    }

    virtual bool viewFilter(const QList<CardItem *> &, const CardItem *) const{
        return true;
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.isEmpty())
            return NULL;

        ZhihengPassCard *zhiheng_pass_card = new ZhihengPassCard;
        zhiheng_pass_card->addSubcards(cards);

        return zhiheng_pass_card;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("ZhihengPassCard");
    }
};


ZhihengPassCard::ZhihengPassCard(){
    target_fixed = true;
    once = true;
}

void ZhihengPassCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    room->playSkillEffect("zhiheng");
    room->throwCard(this);
    bool all_same = true;
    int number = 0 ;
    foreach(int subcard, subcards){
        const Card *card = Sanguosha->getCard(subcard);
        if(number != 0 && number != card->getNumber()){
            all_same = false ;
            break ;
        }
        number = card->getNumber() ;
    }
    int n = subcards.length() ;
    if(all_same)
        n = n * 2 - 1 ;
    room->drawCards(source, n);
}

class FanjianPass: public OneCardViewAsSkill{
public:
    FanjianPass():OneCardViewAsSkill("fanjian_pass"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("FanjianPassCard");
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        FanjianPassCard *fanjian_pass_card = new FanjianPassCard;
        fanjian_pass_card->addSubcard(card_item->getCard()->getId());

        return fanjian_pass_card;
    }
};

FanjianPassCard::FanjianPassCard(){
    will_throw = false ;
}

void FanjianPassCard::onEffect(const CardEffectStruct &effect) const{
    ServerPlayer *zhouyu = effect.from;
    ServerPlayer *target = effect.to;
    Room *room = zhouyu->getRoom();

    int card_id = getSubcards().first();
    const Card *card = Sanguosha->getCard(card_id);
    Card::Suit suit = room->askForSuit(target);

    LogMessage log;
    log.type = "#ChooseSuit";
    log.from = target;
    log.arg = Card::Suit2String(suit);
    room->sendLog(log);

    room->showCard(zhouyu, card_id);
    room->getThread()->delay();

    if(card->getSuit() != suit){
        DamageStruct damage;
        damage.card = NULL;
        damage.from = zhouyu;
        damage.to = target;

        room->damage(damage);

        if(target->isAlive()){
            target->obtainCard(card);
        }else{
            zhouyu->obtainCard(card);
        }
    }else{
        zhouyu->obtainCard(card);
    }
}

class KurouPass: public ZeroCardViewAsSkill{
public:
    KurouPass():ZeroCardViewAsSkill("kurou_pass"){

    }

    virtual const Card *viewAs() const{
        return new KurouPassCard;
    }
};

KurouPassCard::KurouPassCard(){
    target_fixed = true;
}

void KurouPassCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    room->loseHp(source);
    if(source->isAlive())
        room->drawCards(source, 3);
}

class PZhaxiang:public ViewAsSkill{
public:
    PZhaxiang():ViewAsSkill("pzhaxiang"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->isWounded() && !player->hasUsed("PZhaxiangCard");
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        return selected.length() < 4 ;
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() == 4){
            PZhaxiangCard *zhaxiang_card = new PZhaxiangCard();
            zhaxiang_card->addSubcards(cards);
            return zhaxiang_card;
        }else
            return NULL;
    }
};

PZhaxiangCard::PZhaxiangCard(){
    once = true;
    target_fixed = true;
}

void PZhaxiangCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    room->throwCard(this);
    RecoverStruct recover;
    recover.card = this;
    recover.who = source;
    source->getRoom()->recover(source, recover);
}


class KejiPass: public PhaseChangeSkill{
public:
    KejiPass():PhaseChangeSkill("keji_pass"){
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target) && target->getPhase() == Player::Discard;
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        return player->getHandcardNum() < 10;
    }
};


class PBaiyi:public TriggerSkill{
public:
    PBaiyi():TriggerSkill("pbaiyi"){
        events << PhaseChange;

        frequency = Frequent;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *lumeng, QVariant &data) const{
        if(lumeng->getPhase() == Player::Start){
            Room *room = lumeng->getRoom();
            if(lumeng->getEquips().empty() && room->askForSkillInvoke(lumeng, objectName())){
                lumeng->drawCards(1);
            }
        }
        return false;
    }
};


class LianyingPass: public TriggerSkill{
public:
    LianyingPass():TriggerSkill("lianying_pass"){
        events << CardUsed << CardResponsed << CardLost;
        frequency = Frequent;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *luxun, QVariant &data) const{
        Room *room = luxun->getRoom();
        if(event == CardUsed || event == CardResponsed){
            if(luxun->getPhase() != Player::NotActive)
                return false;
            CardStar card = NULL;
            if(event == CardUsed){
                CardUseStruct use = data.value<CardUseStruct>();
                card = use.card;
            }else if(event == CardResponsed)
                card = data.value<CardStar>();
            if(card == NULL || card->isVirtualCard())
                return false;

            if(luxun->askForSkillInvoke(objectName(), data))
                luxun->drawCards(1);
        }if(event == CardLost){
            if(luxun->isKongcheng()){
                CardMoveStar move = data.value<CardMoveStar>();
                if(move->from_place == Player::Hand){
                    if(room->askForSkillInvoke(luxun, objectName())){
                        room->playSkillEffect("lianying");
                        luxun->drawCards(1);
                    }
                }
            }
        }
        return false;
    }
};


class JieyinPass: public OneCardViewAsSkill{
public:
    JieyinPass():OneCardViewAsSkill("jieyin_pass"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("JieyinPassCard");
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return true;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        JieyinPassCard *jieyin_pass_card = new JieyinPassCard;
        jieyin_pass_card->addSubcard(card_item->getCard()->getId());

        return jieyin_pass_card;
    }
};

JieyinPassCard::JieyinPassCard(){
}

bool JieyinPassCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;

    return to_select->getGeneral()->isMale() ;
}

void JieyinPassCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();

    RecoverStruct recover;
    recover.card = this;
    recover.who = effect.from;

    room->recover(effect.from, recover, true);
    room->recover(effect.to, recover, true);

    room->playSkillEffect("jieyin");
    if(!effect.to->isNude()){
        int card_id = room->askForCardChosen(effect.from, effect.to, "he", "jieyin_pass");
        const Card *card = Sanguosha->getCard(card_id);
        room->moveCardTo(card, effect.from, Player::Hand, false);
    }
}


class PYuyue: public OneCardViewAsSkill{
public:
    PYuyue():OneCardViewAsSkill("pyuyue"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("PYuyueCard");
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return true;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        PYuyueCard *yuyue_card = new PYuyueCard;
        yuyue_card->addSubcard(card_item->getCard()->getId());

        return yuyue_card;
    }
};


PYuyueCard::PYuyueCard(){
}

bool PYuyueCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;

    return ! to_select->getJudgingArea().empty() ;
}

void PYuyueCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    Slash *slash = new Slash(Card::NoSuit, 0);
    slash->setSkillName("pyuyue");
    for(int i=0;i<effect.to->getJudgingArea().length();i++){
        CardUseStruct card_use;
        card_use.card = slash;
        card_use.from = effect.from;
        card_use.to << effect.to;
        room->useCard(card_use, false);
    }
}


class PShuangxing: public PhaseChangeSkill{
public:
    PShuangxing():PhaseChangeSkill("pshuangxing"){
        frequency = Frequent;
    }

    virtual bool onPhaseChange(ServerPlayer *daqiao) const{
        if(daqiao->getPhase() == Player::Start || daqiao->getPhase() == Player::Finish){
            Room *room = daqiao->getRoom();
            int n = 0 ;
            foreach(ServerPlayer *player, room->getAlivePlayers()){
                n = qMax(n,player->getJudgingArea().length());
            }
            if(n > 0 && room->askForSkillInvoke(daqiao, objectName())){
                daqiao->drawCards(n);
            }
        }

        return false;
    }
};


class PTongji: public TriggerSkill{
public:
    PTongji():TriggerSkill("ptongji"){
        frequency = Compulsory;
        events << Predamage;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.card && (damage.card->inherits("Slash") || damage.card->inherits("Duel")) && damage.to->isKongcheng())
        {
            Room *room = damage.to->getRoom();

            LogMessage log;
            log.type = "#PTongji";
            log.from = player;
            log.to << damage.to;
            log.arg = QString::number(damage.damage);
            log.arg2 = QString::number(damage.damage + 1);
            room->sendLog(log);

            damage.damage ++;
            data = QVariant::fromValue(damage);
        }

        return false;
    }
};

class PJielue: public TriggerSkill{
public:
    PJielue():TriggerSkill("pjielue"){
        events << CardUsed;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *ganning, QVariant &data) const{
        const Card *card = NULL;
        CardUseStruct use = data.value<CardUseStruct>();
        card = use.card;
        if(!card || !card->inherits("Dismantlement"))
            return false ;
        ServerPlayer *to = use.to.first();
        Room *room = ganning->getRoom();
        if(!to->getEquips().empty() && ganning->askForSkillInvoke(objectName(), QVariant::fromValue(to))){
            int card_id = room->askForCardChosen(ganning, to, "e", objectName());
            const Card *card = Sanguosha->getCard(card_id);
            room->moveCardTo(card, ganning, Player::Hand, false);
        }
        return false;
    }
};




class PZhanshen:public OneCardViewAsSkill{
public:
    PZhanshen():OneCardViewAsSkill("pzhanshen"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return Slash::IsAvailable(player);
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern == "slash";
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        const Card *card = to_select->getFilteredCard();

        return (card->inherits("TrickCard") || card->inherits("EquipCard")) && !to_select->isEquipped();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getCard();
        Card *slash = new Slash(card->getSuit(), card->getNumber());
        slash->addSubcard(card->getId());
        slash->setSkillName(objectName());
        return slash;
    }
};


class PNuozhan : public TriggerSkill{
public:
    PNuozhan():TriggerSkill("pnuozhan"){
        events << DamageComplete << CardResponsed;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        if(player->getPhase() != Player::NotActive)
           return false;

        if(event == CardResponsed){
            CardStar card_star = data.value<CardStar>();
            if(!card_star->inherits("Jink"))
                return false;
        }

        if(!player->isKongcheng() && player->askForSkillInvoke("pnuozhan"))
            player->getRoom()->askForUseCard(player, "slash", "yitian-slash");

        return false;
    }
};


class LijianPass: public OneCardViewAsSkill{
public:
    LijianPass():OneCardViewAsSkill("lijian_pass"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("LijianPassCard");
    }

    virtual bool viewFilter(const CardItem *) const{
        return true;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        LijianPassCard *lijian_pass_card = new LijianPassCard;
        lijian_pass_card->addSubcard(card_item->getCard()->getId());

        return lijian_pass_card;
    }
};

LijianPassCard::LijianPassCard(){
    once = true;
}

bool LijianPassCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{

    if(targets.isEmpty() && to_select->hasSkill("kongcheng") && to_select->isKongcheng()){
        return false;
    }

    return true;
}

bool LijianPassCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    return targets.length() == 2;
}

void LijianPassCard::use(Room *room, ServerPlayer *, const QList<ServerPlayer *> &targets) const{
    room->throwCard(this);

    ServerPlayer *to = targets.at(0);
    ServerPlayer *from = targets.at(1);

    Duel *duel = new Duel(Card::NoSuit, 0);
    duel->setSkillName("lijian");
    duel->setCancelable(false);

    CardUseStruct use;
    use.from = from;
    use.to << to;
    use.card = duel;
    room->useCard(use);
}

class QingnangPass: public OneCardViewAsSkill{
public:
    QingnangPass():OneCardViewAsSkill("qingnang_pass"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("QingnangPassCard");
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return true;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        QingnangPassCard *qingnang_pass_card = new QingnangPassCard;
        qingnang_pass_card->addSubcard(card_item->getCard()->getId());

        return qingnang_pass_card;
    }
};


QingnangPassCard::QingnangPassCard(){
    once = true;
    target_fixed = true;
}

void QingnangPassCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    room->throwCard(this);
    if(source->isWounded()){
        RecoverStruct recover;
        recover.card = this;
        recover.who = source;
        source->getRoom()->recover(source, recover);
    }else{
        room->setPlayerFlag(source,"qingnang_buff");
    }
}

class QingnangBuff: public PhaseChangeSkill{
public:
    QingnangBuff():PhaseChangeSkill("#qingnang"){
    }
    virtual bool onPhaseChange(ServerPlayer *huatuo) const{
        if(huatuo->getPhase() == Player::Start && huatuo->hasFlag("qingnang_buff")){
            huatuo->setFlags("-qingnang_buff");
            if(huatuo->isWounded()){
                RecoverStruct recover;
                recover.who = huatuo;
                huatuo->getRoom()->recover(huatuo, recover);
            }
        }
        return false;
    }
};

class PXuanhu: public TriggerSkill{
public:
    PXuanhu():TriggerSkill("pxuanhu"){
        events << HpRecover ;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return ! target->hasSkill(objectName()) ;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *huatuo = room->findPlayerBySkillName(objectName());

        if(huatuo == NULL)
            return false;

        if(!player->isKongcheng()){
            LogMessage log;
            log.type = "#PXuanhu";
            log.from = huatuo;
            log.to << player;
            room->sendLog(log);
            room->moveCardTo(player->getRandomHandCard(), huatuo, Player::Hand, false);
        }
        return false;
    }
};

PassMode::PassMode(QObject *parent)
    :GameRule(parent)
{
    setObjectName("pass_mode_rule");
}

static int Restart = 1;

bool PassMode::trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
    Room *room = player->getRoom();

    switch(event){
    case GameStart:{
            if(setjmp(env) == Restart){
                room->setTag("SwapPile",0);
                room->setPlayerProperty(player, "phase", "not_active");
                room->setCurrent(room->getLord());
                foreach(ServerPlayer *player, room->getPlayers()){
                    int n = player->isLord() ? 6 : 3 ;
                    if(player->hasSkill("pt_fenjin"))
                        n ++ ;
                    player->drawCards(n);
                }
                room->getLord()->play();
            }else{
                if(player->isLord()){
                    room->setTag("Stage",1);

                    const Package *passpack = Sanguosha->findChild<const Package *>("pass_mode");
                    QList<const General *> generals = passpack->findChildren<const General *>();

                    QStringList names;
                    foreach(const General *general, generals){
                        if(general->getKingdom() == "p_hero")
                            names << general->objectName();
                    }

                    QString name = room->askForGeneral(player, names);

                    room->transfigure(player, name, true, true);
                    room->setPlayerProperty(player, "maxhp", player->getMaxHP() + 1);
                    room->setPlayerProperty(player, "hp", player->getMaxHP());
                    const General *general = Sanguosha->getGeneral(name);
                    if(player->getKingdom() != general->getKingdom())
                        room->setPlayerProperty(player, "kingdom", general->getKingdom());
                }
                if(player->getGeneralName().startsWith("sujiang"))
                    room->setPlayerProperty(player, "maxhp", 3);
                int n = player->isLord() ? 6 : 3 ;
                if(player->hasSkill("pt_fenjin"))
                    n ++ ;
                player->drawCards(n);
            }
            return false;
        }

    case Death:{
            if(player->isLord()){
                if(! room->getAlivePlayers().empty())
                    room->gameOver("rebel");
            }else{
                ServerPlayer *lord = room->getLord();
                if(lord->hasSkill("pt_fenjin")){
                    lord->drawCards(1);
                }

                DamageStar damage = data.value<DamageStar>();
                if((!damage || damage->from != lord))
                    lord->drawCards(3);

                static QMap<QString, int> exp_map;
                if(exp_map.isEmpty()){
                    exp_map.insert("p_evil", 4);
                    exp_map.insert("p_hero", 8);
                }
                int exp = exp_map.value(player->getKingdom(),10) ;

                if(lord->getKingdom() == "p_hero"){
                    int orgin_exp = lord->getMark("@exp") ;
                    lord->gainMark("@exp",exp);
                    LogMessage log;
                    log.type = "#GainExp";
                    log.from = lord;
                    log.to << player;
                    log.arg = QString::number(exp);
                    log.arg2 = QString::number(orgin_exp+exp) ;
                    room->sendLog(log);
                }else{
                    LogMessage log;
                    log.type = "#CantGainExp";
                    log.from = lord;
                    room->sendLog(log);
                }

                static QList<QString> enemy_list;
                if(enemy_list.isEmpty()){
                    enemy_list.append("p_jianwei+p_qibing+p_shizu");
                    enemy_list.append("p_huwei+p_gongshou+p_huwei");
                    enemy_list.append("p_jianwei+p_yaodao+p_jianwei");
                    enemy_list.append("p_huwei+caocao_p+p_jianwei");
                    enemy_list.append("p_gongshou+luxun_p+p_shizu");
                    enemy_list.append("p_qibing+machao_p+p_qibing");
                    enemy_list.append("p_jianwei+zhaoyun_p+p_huwei");
                    enemy_list.append("p_jianwei+zhouyu_p+p_jianwei");
                    enemy_list.append("p_yaodao+guojia_p+p_yaodao"); // 10
                    enemy_list.append("p_huwei+zhangliao_p+p_qibing");
                    enemy_list.append("p_shizu+ganning_p+p_shizu");
                    enemy_list.append("p_huwei+sunshangxiang_p+lumeng_p");
                    enemy_list.append("zhugeliang_p+huangyueying_p+p_gongshou");
                    enemy_list.append("p_qibing+xuchu_p+xiahoudun_p");
                    enemy_list.append("luxun_p+huangyueying_p+simayi_p");
                    enemy_list.append("guojia_p+caocao_p+zhenji_p");
                    enemy_list.append("zhouyu_p+sunquan_p+huanggai_p");
                    enemy_list.append("zhangfei_p+liubei_p+zhangfei_p");
                    enemy_list.append("lubu_p+diaochan_p+lubu_p");
                }

                if(room->getAlivePlayers().length() == 1){
                    player->throwAllCards();
                    int stage = room->getTag("Stage").toInt();
                    if(stage >= enemy_list.length()+1){
                        room->gameOver("lord");
                        break ;
                    }
                    stage ++ ;
                    room->setTag("Stage",stage);

                    LogMessage log;
                    log.type = "#NextPStage";
                    log.from = lord;
                    log.arg = QString::number(stage);
                    room->sendLog(log);

                    static QMap<QString, int> skill_map;
                    if(skill_map.isEmpty()){
                        skill_map.insert("mashu", 15);
                        skill_map.insert("pt_kezhi", 15);
                        skill_map.insert("pt_qingxin", 15);
                        skill_map.insert("pt_fenjin", 20);
                        skill_map.insert("pt_nuhou", 20);
                        skill_map.insert("pt_duanyan", 30);
                        skill_map.insert("niepan", 30);
                        skill_map.insert("pt_tipo" ,40);
                        skill_map.insert("feiying", 55);
                        skill_map.insert("yingzi", 60);
                    }

                    QString choice  ;
                    while(choice != "cancel"){
                        QString skill_names ;
                        foreach (QString key, skill_map.keys()) {
                            if(!lord->hasSkill(key))
                                skill_names.append(key).append("_s+") ;
                        }
                        skill_names.append("cancel") ;

                        choice = room->askForChoice(lord,"study_skill",skill_names) ;
                        QString skill_name = choice.split("_s").at(0) ;
                        int exp = lord->getMark("@exp") ;
                        int need_exp = skill_map.value(skill_name) ;
                        if(exp >= need_exp){
                            exp -= need_exp ;
                            room->setPlayerMark(lord,"@exp",exp);
                            room->acquireSkill(lord,skill_name);
                            if(skill_name == "pt_tipo"){
                                room->setPlayerProperty(lord, "maxhp", lord->getMaxHP() + 1);
                            }else if(skill_name == "pt_kezhi"){
                                room->setPlayerProperty(lord, "xueyi", 1);
                            }else if(skill_name == "niepan"){
                                lord->gainMark("@nirvana");
                            }
                        }

                        if(exp < 15)
                            break ;
                    }

                    room->setPlayerProperty(lord, "hp", lord->getMaxHP());
                    lord->throwAllCards();
                    room->setPlayerProperty(lord, "hp", lord->getMaxHP());
                    int exp = lord->getMark("@exp");
                    int nirvana = lord->getMark("@nirvana");
                    lord->throwAllMarks();
                    lord->gainMark("@exp",exp);
                    if(nirvana > 0)
                        lord->gainMark("@nirvana");
                    lord->clearFlags();
                    lord->clearHistory();


                    int i = 0 ;
                    QStringList enemys = enemy_list.at(stage-2).split("+") ;
                    foreach(ServerPlayer *player, room->getPlayers()){
                        if(player->isDead()){
                            room->transfigure(player, enemys.at(i), true, true);
                            const General *general = Sanguosha->getGeneral(enemys.at(i));
                            if(player->getKingdom() != general->getKingdom())
                                room->setPlayerProperty(player, "kingdom", general->getKingdom());
                            room->revivePlayer(player);
                            if(!player->faceUp())
                                player->turnOver();
                            if(player->isChained())
                                room->setPlayerProperty(player, "chained", false);
                            player->throwAllMarks();
                            i ++ ;
                        }
                    }

                    longjmp(env,Restart);
                    return true ;
                }
            }
        }

    default:
        break;
    }

    return GameRule::trigger(event, player, data);
}

bool PassModeScenario::exposeRoles() const{
    return true;
}

void PassModeScenario::assign(QStringList &generals, QStringList &roles) const{
    generals << lord << loyalists << rebels << renegades;

    foreach(QString general, generals){
        if(general == lord)
            roles << "lord";
        else if(loyalists.contains(general))
            roles << "loyalist";
        else if(rebels.contains(general))
            roles << "rebel";
        else
            roles << "renegade";
    }
}

int PassModeScenario::getPlayerCount() const{
    return 4;
}

void PassModeScenario::getRoles(char *roles) const{
    strcpy(roles, "ZFFF");
}

void PassModeScenario::onTagSet(Room *room, const QString &key) const{
    // dummy
}

bool PassModeScenario::generalSelection() const{
    return false;
}


class PassModeRule: public ScenarioRule{
public:
    PassModeRule(Scenario *scenario)
        :ScenarioRule(scenario)
    {
        events << GameStart << GameOverJudge << DrawNCards << Predamaged;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        switch(event){
        case DrawNCards:{
                if(player->getKingdom() == "p_evil")
                    data = data.toInt() - 1 ;
                break;
            }
        case Predamaged:{
                DamageStruct damage = data.value<DamageStruct>();
                if(damage.card != NULL && damage.card->inherits("Lightning")){
                    damage.damage -- ;
                    data = QVariant::fromValue(damage);
                }
                break;
            }

        case GameOverJudge:{
                return true;
            }
        default:
            break;
        }

        return false;
    }
};

PassModeScenario::PassModeScenario()
        :Scenario("pass_mode")
{

    lord = "liubei_p";
    rebels << "sujiang" << "sujiangf" << "sujiang";
//    rebels << "ganning_p" << "ganning_p" << "ganning_p";
    rule = new PassModeRule(this);

    skills << new PShiqi << new PQianggong << new PPojia << new PQishu << new PChenwen << new Phongzhuang << new PYaoshu << new PJitian
            << new RendePass << new JijiangPass << new PWenjiu << new PBaonu << new TiejiPass << new MashuPass << new JizhiPass << new PLonghou
            << new JianxiongPass << new PXietian << new PBadao << new PBadaoCost << new FankuiPass << new PLanggu << new GangliePass  << new PJueduan
                << new YijiPass << new PGuimou << new LuoshenPass << new LuoyiPass << new PGuantong  << new TuxiPass
            << new ZhihengPass << new FanjianPass << new KurouPass << new PZhaxiang << new KejiPass << new PBaiyi << new LianyingPass
                << new JieyinPass << new PTongji << new PJielue << new PYuyue << new PShuangxing
            << new PZhanshen << new PNuozhan << new LijianPass << new QingnangPass << new QingnangBuff << new PXuanhu
            << new Skill("pt_nuhou") << new Skill("pt_tipo") << new Skill("pt_kezhi") << new Skill("pt_fenjin") << new PQingxin << new PDuanyan
            ;

    related_skills.insertMulti("pwenjiu", "#luoyi");
    related_skills.insertMulti("qingnang_pass", "#qingnang");
    related_skills.insertMulti("pbadao", "#badao_cost");

    General *iizu, *gyip, *jjww, *qibq, *huww, *yddd;
    iizu = new General(this,"p_shizu","p_evil",3, true, true);
    iizu->addSkill("ps_shiqi");

    gyip = new General(this,"p_gongshou","p_evil",3, false, true);
    gyip->addSkill("zhengfeng");
    gyip->addSkill("ps_qianggong");

    jjww = new General(this,"p_jianwei","p_evil",3, false, true);
    jjww->addSkill("ps_pojia");

    qibq = new General(this,"p_qibing","p_evil",3, true, true);
    qibq->addSkill("ps_qishu");

    huww = new General(this,"p_huwei","p_evil",3, true, true);
    huww->addSkill("ps_chenwen");
    huww->addSkill("ps_zhongzhuang");

    yddd = new General(this,"p_yaodao","p_evil",3, true, true);
    yddd->addSkill("ps_yaoshu");
    yddd->addSkill("ps_jitian");

    General *liubei_p = new General(this,"liubei_p","p_hero",4, true, true);
    liubei_p->addSkill("rende_pass");
    liubei_p->addSkill("jijiang_pass");

    General *guanyu_p = new General(this,"guanyu_p","p_hero",4, true, true);
    guanyu_p->addSkill("wusheng");
    guanyu_p->addSkill("pwenjiu");
    guanyu_p->addSkill("#luoyi");

    General *zhangfei_p = new General(this,"zhangfei_p","p_hero",4, true, true);
    zhangfei_p->addSkill("paoxiao");
    zhangfei_p->addSkill("pbaonu");

    General *zhaoyun_p = new General(this,"zhaoyun_p","p_hero",4, true, true);
    zhaoyun_p->addSkill("longdan");
    zhaoyun_p->addSkill(new Skill("plongwei"));
    zhaoyun_p->addSkill("plonghou");

    General *machao_p = new General(this,"machao_p","p_hero",4, true, true);
    machao_p->addSkill("tieji_pass");
    machao_p->addSkill("mashu_pass");

    General *zhugeliang_p = new General(this,"zhugeliang_p","p_hero",3, true, true);
    zhugeliang_p->addSkill("super_guanxing");
    zhugeliang_p->addSkill("kongcheng");

    General *huangyueying_p = new General(this,"huangyueying_p","p_hero",3, false, true);
    huangyueying_p->addSkill("jizhi_pass");
    huangyueying_p->addSkill("qicai");


    General *caocao_p = new General(this,"caocao_p","p_hero",4, true, true);
    caocao_p->addSkill("jianxiong_pass");
    caocao_p->addSkill("pxietian");
    caocao_p->addSkill("pbadao");
    caocao_p->addSkill("#badao_cost");

    General *simayi_p = new General(this,"simayi_p","p_hero",3, true, true);
    simayi_p->addSkill("guicai");
    simayi_p->addSkill("fankui_pass");
    simayi_p->addSkill("planggu");

    General *xiahoudun_p = new General(this,"xiahoudun_p","p_hero",4, true, true);
    xiahoudun_p->addSkill("ganglie_pass");
    xiahoudun_p->addSkill("pjueduan");

    General *guojia_p = new General(this,"guojia_p","p_hero",3, true, true);
    guojia_p->addSkill("yiji_pass");
    guojia_p->addSkill("pguimou");

    General *zhenji_p = new General(this,"zhenji_p","p_hero",3, false, true);
    zhenji_p->addSkill("luoshen_pass");
    zhenji_p->addSkill("qingguo");

    General *xuchu_p = new General(this,"xuchu_p","p_hero",4, true, true);
    xuchu_p->addSkill("luoyi_pass");
    xuchu_p->addSkill("pguantong");
    xuchu_p->addSkill("#luoyi");

    General *zhangliao_p = new General(this,"zhangliao_p","p_hero",4, true, true);
    zhangliao_p->addSkill("tuxi_pass");

    General *sunquan_p = new General(this,"sunquan_p","p_hero",4, true, true);
    sunquan_p->addSkill("zhiheng_pass");

    General *zhouyu_p = new General(this, "zhouyu_p", "p_hero", 3, true, true);
    zhouyu_p->addSkill("yingzi");
    zhouyu_p->addSkill("fanjian_pass");

    General *lumeng_p = new General(this, "lumeng_p", "p_hero", 4 ,true, true);
    lumeng_p->addSkill("keji_pass");
    lumeng_p->addSkill("pbaiyi");

    General *luxun_p = new General(this, "luxun_p", "p_hero", 3, true, true);
    luxun_p->addSkill("qianxun");
    luxun_p->addSkill("lianying_pass");

    General *ganning_p = new General(this, "ganning_p", "p_hero", 4 ,true, true);
    ganning_p->addSkill("qixi");
    ganning_p->addSkill("ptongji");
    ganning_p->addSkill("pjielue");

    General *huanggai_p = new General(this, "huanggai_p", "p_hero", 4 ,true, true);
    huanggai_p->addSkill("kurou_pass");
    huanggai_p->addSkill("pzhaxiang");

    General *daqiao_p = new General(this, "daqiao_p", "p_hero", 3, false, true);
    daqiao_p->addSkill("guose");
    daqiao_p->addSkill("pyuyue");
    daqiao_p->addSkill("pshuangxing");

    General *sunshangxiang_p = new General(this, "sunshangxiang_p", "p_hero", 3, false, true);
    sunshangxiang_p->addSkill("jieyin_pass");
    sunshangxiang_p->addSkill("xiaoji");


    General *lubu_p = new General(this, "lubu_p", "p_hero", 4, true, true);
    lubu_p->addSkill("wushuang");
    lubu_p->addSkill("pzhanshen");
    lubu_p->addSkill("pnuozhan");

    General *huatuo_p = new General(this, "huatuo_p", "p_hero", 3, true, true);
    huatuo_p->addSkill("qingnang_pass");
    huatuo_p->addSkill("#qingnang");
    huatuo_p->addSkill("jijiu");
    huatuo_p->addSkill("pxuanhu");

    General *diaochan_p = new General(this, "diaochan_p", "p_hero", 3, false, true);
    diaochan_p->addSkill("lijian_pass");
    diaochan_p->addSkill("biyue");

    addMetaObject<PDuanyanCard>();
    addMetaObject<PYaoshuCard>();
    addMetaObject<RendePassCard>();
    addMetaObject<JijiangPassCard>();
    addMetaObject<LuoyiPassCard>();
    addMetaObject<TuxiPassCard>();
    addMetaObject<ZhihengPassCard>();
    addMetaObject<FanjianPassCard>();
    addMetaObject<KurouPassCard>();
    addMetaObject<PZhaxiangCard>();
    addMetaObject<JieyinPassCard>();
    addMetaObject<PYuyueCard>();
    addMetaObject<LijianPassCard>();
    addMetaObject<QingnangPassCard>();
}

ADD_SCENARIO(PassMode)
