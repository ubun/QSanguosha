#include "standard.h"
#include "skill.h"
#include "gold-seinto-pre.h"
#include "client.h"
#include "carditem.h"
#include "engine.h"
#include "maneuvering.h"
#include "ai.h"

class  Jingqiang: public TriggerSkill{
public:
    Jingqiang(): TriggerSkill("jingqiang"){
        events << PhaseChange;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *target = NULL;

        if(player->getPhase() == Player::Start){
            foreach(ServerPlayer *p, room->getAlivePlayers()){
                if(p->getMark("@jq") > 0){
                    p->loseAllMarks("@jq");
                    target = p;
                    break;
                }
            }
            if(target){
                foreach(ServerPlayer *p, room->getOtherPlayers(target)){
                    room->setFixedDistance(p, target, -1);
                }
            }
        }
        else if(player->getPhase() == Player::Finish){
            if(!room->askForSkillInvoke(player, objectName()))
                return false;

            target = room->askForPlayerChosen(player, room->getAlivePlayers(), objectName());
            foreach(ServerPlayer *p, room->getOtherPlayers(target)){
                room->setFixedDistance(p, target, target->distanceTo(p)+1);
            }
            target->gainMark("@jq");
        }

        return false;
    }
};

class JingqiangClear: public TriggerSkill{
public:
    JingqiangClear():TriggerSkill("#jingqiang-clear"){
        events << Death;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &) const{
        Room *room = player->getRoom();
        QList<ServerPlayer *> players = room->getAllPlayers();
        ServerPlayer *target = NULL;
        foreach(ServerPlayer *p, players){
            if(p->getMark("@jq") > 0){
                p->loseAllMarks("@jq");
                target = p;
                break;
            }
        }
        if(target){
            foreach(ServerPlayer *p, room->getOtherPlayers(target)){
                room->setFixedDistance(p, target, -1);
            }
        }
        return false;
    }
};

class Zhuyi: public OneCardViewAsSkill{
public:
    Zhuyi():OneCardViewAsSkill("zhuyi"){
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped() && to_select->getCard()->inherits("EquipCard");
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        ZhuyiCard *card = new ZhuyiCard;
        card->addSubcard(card_item->getCard()->getId());

        return card;
    }
};

ZhuyiCard::ZhuyiCard(){
}
void ZhuyiCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    room->throwCard(this);
    effect.from->drawCards(1);
    effect.to->drawCards(1);
}

class Zhuanling: public PhaseChangeSkill{
public:
    Zhuanling():PhaseChangeSkill("zhuanling$"){
        frequency = Wake;
        default_choice = "draw";
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target)
                && target->getPhase() == Player::Start
                && target->getMark("zhuanling") == 0
                && target->getHandcardNum() == 0;
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        Room *room = player->getRoom();

        LogMessage log;
        log.type = "#ZhuanlingWake";
        log.from = player;
        log.arg = QString::number(player->getHandcardNum());
        room->sendLog(log);

        QString result = room->askForChoice(player ,objectName(), "recover+draw");
        if(result == "recover"){
            RecoverStruct recover;
            recover.card = NULL;
            recover.who = NULL;
            room->recover(player, recover);
        }
        else if(result == "draw")
            player->drawCards(2);

        player->setMark("zhuanling", 1);

        room->loseMaxHp(player);
        room->acquireSkill(player, "jiaohuang");

        return false;
    }
};

JiaohuangCard::JiaohuangCard(){
    once = true;
}

void JiaohuangCard::use(Room *room, ServerPlayer *, const QList<ServerPlayer *> &targets) const{
    ServerPlayer *header = targets.first();
    if(header->hasSkill("jiaohuang")){
        header->obtainCard(this);
        room->setEmotion(header, "good");
    }
}

bool JiaohuangCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select->hasLordSkill("jiaohuang") && to_select != Self;
}

class JiaohuangViewAsSkill: public OneCardViewAsSkill{
public:
    JiaohuangViewAsSkill():OneCardViewAsSkill("jiaohuangv"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("JiaohuangCard") && player->getKingdom() == "st";
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        const Card *card = to_select->getCard();
        return !to_select->isEquipped() && card->inherits("EquipCard");
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        JiaohuangCard *card = new JiaohuangCard;
        card->addSubcard(card_item->getFilteredCard());

        return card;
    }
};

class Jiaohuang: public TriggerSkill{
public:
    Jiaohuang():TriggerSkill("jiaohuang"){
        events << GameStart << TurnStart;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        ServerPlayer *aries = target->getRoom()->findPlayerBySkillName(objectName());
        if(aries && aries->getMark("jhv") == 0)
            return true;
        else return false;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *aries, QVariant &) const{
        Room *room = aries->getRoom();
        QList<ServerPlayer *> players = room->getAlivePlayers();
        foreach(ServerPlayer *player, players){
            room->attachSkillToPlayer(player, "jiaohuangv");
        }
        aries->setMark("jhv", 1);

        return false;
    }
};

HaojiaoCard::HaojiaoCard(){
    once = true;
}

bool HaojiaoCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(targets.length() < 2)
        return Self->inMyAttackRange(to_select) && !Self->isProhibited(to_select, new Slash(Card::NoSuit, 0));
    else
        return false;
}

void HaojiaoCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    Slash *slash = new Slash(Card::NoSuit, 0);
    CardUseStruct use;
    use.card = slash;
    use.from = source;
    use.to = targets;
    room->useCard(use, false);
}

class HaojiaoViewAsSkill: public ZeroCardViewAsSkill{
public:
    HaojiaoViewAsSkill(): ZeroCardViewAsSkill("haojiao"){

    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "@haojiao-card";
    }

    virtual const Card *viewAs() const{
        HaojiaoCard *card = new HaojiaoCard;
        card->setSkillName("haojiao");
        return card;
    }
};

class Haojiao: public TriggerSkill{
public:
    Haojiao(): TriggerSkill("haojiao"){
        events << PhaseChange << Damaged;

        view_as_skill = new HaojiaoViewAsSkill;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();

        if(event == PhaseChange){
            if(player->getPhase() != Player::Finish)
                return false;

            if(room->askForUseCard(player, "@haojiao-card", "@@haojiao"))
                player->turnOver();
        }
        else if(event == Damaged){
            if(player->faceUp() || !room->askForSkillInvoke(player, objectName()))
                return false;

            player->turnOver();
        }

        return false;
    }
};

class Disui: public TriggerSkill{
public:
    Disui():TriggerSkill("disui"){
        events << SlashProceed;
        frequency = Frequent;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        if(player->getPhase() != Player::Play || effect.slash->getNature() == DamageStruct::Normal)
            return false;

        Room *room = player->getRoom();
        QString slasher = player->objectName();

        const Card *first_jink = NULL, *second_jink = NULL;
        first_jink = room->askForCard(effect.to, "jink", "@disui-jink-1:" + slasher);
        if(first_jink)
            second_jink = room->askForCard(effect.to, "jink", "@disui-jink-2:" + slasher);

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

class Ciyuan: public TriggerSkill{
public:
    Ciyuan(): TriggerSkill("ciyuan"){
        events << DamageComplete;
        frequency = Wake;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        ServerPlayer *gemini = target->getRoom()->findPlayerBySkillName(objectName());
        return gemini && gemini->getMark("ciyuan") == 0;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.nature != DamageStruct::Fire)
            return false;
        Room *room = player->getRoom();
        ServerPlayer *gemini = room->findPlayerBySkillName(objectName());
        if(!gemini)
            return false;
        LogMessage log;
        log.type = "#CiyuanWake";
        log.from = gemini;
        room->sendLog(log);

        room->loseMaxHp(gemini);
        QList<int> cards_id = room->getNCards(7);
        foreach(int card_id, cards_id){
            gemini->obtainCard(Sanguosha->getCard(card_id));
            gemini->addToPile("stars", card_id, false);
        }

        room->acquireSkill(gemini, "qixing");
        gemini->gainMark("@star", 7);
        room->acquireSkill(gemini, "dawu");

        gemini->addMark("ciyuan");

        return false;
    }
};

ShiqiCard::ShiqiCard(){
    once = true;
}

bool ShiqiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty();
}

void ShiqiCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    room->throwCard(this);

    QString choice = room->askForChoice(source, "shiqi", "start+judge+draw+discard+finish");
    QVariant data = QVariant::fromValue(choice);
    room->setTag(targets.first()->objectName(), data);

    targets.first()->gainMark("@shiqi");
}

class Shiqi: public ViewAsSkill{
public:
    Shiqi(): ViewAsSkill("shiqi"){

    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(selected.length() < 2)
            return true;
        else
            return false;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("ShiqiCard");
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        ShiqiCard *card = new ShiqiCard;
        card->addSubcards(cards);
        card->setSkillName(objectName());
        return card;
    }
};

class ShiqiEffect: public TriggerSkill{
public:
    ShiqiEffect(): TriggerSkill("#shiqi-effect"){
        events << PhaseChange;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        if(player->getPhase() != Player::NotActive)
            return false;

        Room*room = player->getRoom();
        ServerPlayer *target = NULL;
        foreach(ServerPlayer *p, room->getAllPlayers()){
            if(p->getMark("@shiqi")>0){
                target = p;
                break;
            }
        }

        if(target){
            target->loseAllMarks("@shiqi");
            QString choice = room->getTag(target->objectName()).toString();
            room->removeTag(target->objectName());

            LogMessage log;
            log.type = "#ShiqiEffect";
            log.from = player;
            log.to << target;
            log.arg = "shiqi";
            log.arg2 = choice;
            room->sendLog(log);

            QList<Player::Phase> phases;
            if(choice == "start")
                phases << Player::Start;
            else if(choice == "judge")
                phases << Player::Judge;
            else if(choice == "draw")
                phases << Player::Draw;
            else if(choice == "discard")
                phases << Player::Discard;
            else if(choice == "finish")
                phases << Player::Finish;

            if(!phases.isEmpty()){
                target->play(phases);
            }
        }

        return false;
    }
};

class Leiguang: public OneCardViewAsSkill{
public:
    Leiguang():OneCardViewAsSkill("leiguang"){
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getCard()->objectName() == "slash";
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *first = card_item->getCard();
        ThunderSlash *thunderslash = new ThunderSlash(first->getSuit(), first->getNumber());
        thunderslash->addSubcard(first->getId());
        thunderslash->setSkillName(objectName());
        return thunderslash;
    }
};

class Juexiao: public PhaseChangeSkill{
public:
    Juexiao():PhaseChangeSkill("juexiao"){
        frequency = Wake;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target)
                && target->getMark("juexiao") == 0
                && target->getPhase() == Player::Start;
    }

    virtual bool onPhaseChange(ServerPlayer *leo) const{
        Room *room = leo->getRoom();

        foreach(ServerPlayer *p, room->getAllPlayers()){
            if(leo->getHp() > p->getHp())
                return false;
        }

        LogMessage log;
        log.type = "#JuexiaoWake";
        log.from = leo;
        log.arg = QString::number(leo->getHp());
        room->sendLog(log);

        room->setPlayerMark(leo, "juexiao", 1);
        room->loseMaxHp(leo);
        room->setPlayerProperty(leo, "hp", leo->getMaxHP());

        QStringList all_generals = Sanguosha->getLimitedGeneralNames();
        QStringList skill_names;
        foreach(QString name, all_generals){
            const General *general = Sanguosha->getGeneral(name);
            if(general->getKingdom() != "st" || general->getPackage() != "goldseintopre")
                continue;
            foreach(const Skill *skill, general->getVisibleSkillList()){
                if(leo->hasSkill(skill->objectName()) || skill->isLordSkill() ||
                   skill->getFrequency() == Skill::Limited
                   || skill->getFrequency() == Skill::Wake)
                    continue;
                if((skill->objectName() == "zhuyi" && !leo->hasSkill("jingqiang")) ||
                   (skill->objectName() == "wangqi" && !leo->hasSkill("jiguang")) ||
                   (skill->objectName() == "jingji" && !leo->hasSkill("mogong")) ||
                   skill->objectName() == "baolun")
                    continue;
                skill_names << skill->objectName();
            }
        }

        QString skill = room->askForChoice(leo, "jxxkudyx", skill_names.join("+"));
        room->acquireSkill(leo, skill);
        leo->addMark("jxuy");
        return false;
    }
};

class Chaoyue:public MasochismSkill{
public:
    Chaoyue():MasochismSkill("chaoyue"){
        frequency = Frequent;
    }

    virtual void onDamaged(ServerPlayer *leo, const DamageStruct &damage) const{
        if(leo->getMark("juexiao") < 1 || leo->getMark("jxuy") >= 3 || !leo->askForSkillInvoke(objectName()))
            return;
        Room *room = leo->getRoom();

        QStringList all_generals = Sanguosha->getLimitedGeneralNames();
        QStringList skill_names;
        for(int i = damage.damage; i > 0; i--){
            foreach(QString name, all_generals){
                const General *general = Sanguosha->getGeneral(name);
                if(general->getKingdom() != "st")
                    continue;
                foreach(const Skill *skill, general->getVisibleSkillList()){
                    if(leo->hasSkill(skill->objectName()) || skill->isLordSkill() ||
                       skill->getFrequency() == Skill::Limited
                       || skill->getFrequency() == Skill::Wake)
                        continue;
                    if((skill->objectName() == "zhuyi" && !leo->hasSkill("jingqiang")) ||
                       (skill->objectName() == "wangqi" && !leo->hasSkill("jiguang")) ||
                       (skill->objectName() == "jingji" && !leo->hasSkill("mogong")) ||
                       skill->objectName() == "baolun")
                        continue;
                    skill_names << skill->objectName();
                }
            }

            QString skill = room->askForChoice(leo, "jxxkudyx", skill_names.join("+"));
            room->acquireSkill(leo, skill);
            leo->addMark("jxuy");
        }
    }
};

class Tianmo: public TriggerSkill{
public:
    Tianmo():TriggerSkill("tianmo"){
        events << CardResponsed;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *virgo, QVariant &data) const{
        CardStar card_star = data.value<CardStar>();
        if(!card_star->inherits("Jink") || card_star->getSuit() == Card::NoSuit)
            return false;

        Room *room = virgo->getRoom();
        QString choice = room->askForChoice(virgo, objectName(), "draw+guo+cancel");
        if(choice == "draw")
            virgo->drawCards(1);
        else if(choice == "guo"){
            virgo->addToPile("guo", room->drawCard());
            virgo->addToPile("guo", room->drawCard());
        }

        return false;
    }
};

class Liudao: public PhaseChangeSkill{
public:
    Liudao():PhaseChangeSkill("liudao"){
        frequency = Wake;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return (PhaseChangeSkill::triggerable(target)
                && target->getPhase() == Player::Start
                && target->getMark("liudao") == 0
                && target->getPile("guo").length() >= 6) ||
                (target->getPhase() == Player::Draw
                && target->getMark("liudao") != 0
                && !target->getPile("guo").isEmpty());
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        Room *room = player->getRoom();

        if(player->getMark("liudao") != 0){
            QList<int> guoguo = player->getPile("guo");
            room->fillAG(guoguo, player);
            int card_id = room->askForAG(player, guoguo, true, "guo");
            if(card_id != -1){
                guoguo.removeOne(card_id);
                room->moveCardTo(Sanguosha->getCard(card_id), player, Player::Hand, false);
            }
            player->invoke("clearAG");
            return false;
        }
        LogMessage log;
        log.type = "#LiudaoWake";
        log.from = player;
        room->sendLog(log);

        player->setMark("liudao", 1);
        room->loseMaxHp(player);

        return false;
    }
};

class Baolun:public ZeroCardViewAsSkill{
public:
    Baolun():ZeroCardViewAsSkill("baolun"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getMark("liudao") == 0;
    }

    virtual const Card *viewAs() const{
        return new BaolunCard;
    }
};

BaolunCard::BaolunCard(){
}

bool BaolunCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && !to_select->isKongcheng()
            && Self->getPile("guo").length() >= to_select->getHandcardNum();
}

void BaolunCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    ServerPlayer *target = targets.first();
    int handlog = target->getHandcardNum();
    target->throwAllHandCards();

    QList<int> guoguo = source->getPile("guo");
    room->fillAG(guoguo, source);
    while(!guoguo.isEmpty()){
        if(handlog <= 0)
            break;
        int card_id = room->askForAG(source, guoguo, true, "guo");
        if(card_id == -1)
            break;
        guoguo.removeOne(card_id);
        handlog --;
        room->moveCardTo(Sanguosha->getCard(card_id), target, Player::Hand, false);
    }
    source->invoke("clearAG");
}

class Longba: public TriggerSkill{
public:
    Longba():TriggerSkill("longba"){
        events << Damage;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *libra, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();

        if(damage.card && damage.card->inherits("Slash") && damage.to->isAlive()
            && damage.card->isRed()
            && !damage.to->isAllNude()){
            Room *room = libra->getRoom();
            if(room->askForSkillInvoke(libra, objectName())){
                int to_throw = room->askForCardChosen(libra, damage.to, "hej", objectName());
                room->throwCard(to_throw);
            }
        }
        return false;
    }
};

class Longfei: public PhaseChangeSkill{
public:
    Longfei():PhaseChangeSkill("longfei"){
        frequency = Wake;
        default_choice = "draw";
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target)
                && target->getPhase() == Player::Start
                && target->getMark("longfei") == 0
                && target->getEquips().length() > 2;
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        Room *room = player->getRoom();

        LogMessage log;
        log.type = "#LongfeiWake";
        log.from = player;
        room->sendLog(log);

        QString result = room->askForChoice(player ,objectName(), "recover+draw");
        if(result == "recover"){
            RecoverStruct recover;
            recover.card = NULL;
            recover.who = NULL;
            room->recover(player, recover);
        }
        else if(result == "draw")
            player->drawCards(2);

        player->setMark("longfei", 1);

        room->loseMaxHp(player);
        room->acquireSkill(player, "longdan");
        room->acquireSkill(player, "zhijian");

        return false;
    }
};

class Zhiyan:public ZeroCardViewAsSkill{
public:
    Zhiyan():ZeroCardViewAsSkill("zhiyan"){
    }
/*
    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->isWounded();
    }
*/
    virtual const Card *viewAs() const{
        return new ZhiyanCard;
    }
};

ZhiyanCard::ZhiyanCard(){
}

void ZhiyanCard::onEffect(const CardEffectStruct &effect) const{
    effect.from->getRoom()->loseMaxHp(effect.from);
    DamageStruct damage;
    damage.nature = DamageStruct::Fire;
    damage.from = effect.from;
    damage.to = effect.to;
    effect.from->getRoom()->damage(damage);
}

class Jingong: public TriggerSkill{
public:
    Jingong():TriggerSkill("jingong"){
        events << PhaseChange << CardLost << CardFinished;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        if(room->getCurrent() != player)
            return false;
        if(!player->getWeapon()){
            room->setPlayerFlag(player, "oo");
        }
        else
            room->setPlayerFlag(player, "-oo");

        return false;
    }
};
/*
class Jingong: public TriggerSkill{
public:
    Jingong(): TriggerSkill("jingong"){
        events << CardLost;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        if(player->getWeapon())
            player->setAttackRange(1);
        else{
            player->setAttackRange(5);

            LogMessage log;
            log.type = "#Jingong";
            log.from = player;
            log.arg = objectName();
            player->getRoom()->sendLog(log);
        }
        return false;
    }
};
*/
class Shanyao: public PhaseChangeSkill{
public:
    Shanyao():PhaseChangeSkill("shanyao"){
        frequency = Wake;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target)
                && target->getPhase() == Player::Start
                && target->getMark("shanyao") == 0
                && target->getHp() == 1;
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        Room *room = player->getRoom();

        LogMessage log;
        log.type = "#ShanyaoWake";
        log.from = player;
        room->sendLog(log);

        player->setMark("shanyao", 1);

        room->loseMaxHp(player);
        if(player->getHandcardNum() >= player->getHp())
            room->acquireSkill(player, "liegong");
        else
            room->acquireSkill(player, "buqu");

        return false;
    }
};

class Shengjian: public OneCardViewAsSkill{
public:
    Shengjian():OneCardViewAsSkill("shengjian"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->isKongcheng() && !player->hasUsed("ShengjianCard");
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped() && to_select->getCard()->inherits("TrickCard");
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        ShengjianCard *card = new ShengjianCard;
        card->addSubcard(card_item->getCard()->getId());

        return card;
    }
};

ShengjianCard::ShengjianCard(){
    target_fixed = true;
    once = true;
}

void ShengjianCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    int sigema = Sanguosha->getCardCount();
    QStringList weapons;
    switch(Sanguosha->getCard(this->getSubcards().first())->getSuit()){
        case Spade: weapons << "blade" << "spear"; break;
        case Heart: weapons << "kylin_bow" << "ice_sword"; break;
        case Club: weapons << "qinggang_sword" << "halberd"; break;
        case Diamond: weapons << "axe" << "double_sword"; break;
        default: return;
    }

    QMutableListIterator<QString> itor(weapons);
    while(itor.hasNext()){
        itor.next();
        if(source->getMark(itor.value()) > 0)
            itor.remove();
    }

    source->addMark("stswd");
    if(weapons.length() < 1)
        return;
    QString wpch = room->askForChoice(source, "shengjian", weapons.join("+"));
    for(sigema--; sigema > -1; sigema--){
        if(Sanguosha->getCard(sigema)->objectName() == wpch){
            room->obtainCard(source, sigema);
            break;
        }
    }
    room->throwCard(this);
    if(source->getWeapon())
        room->throwCard(source->getWeapon());
    room->moveCardTo(Sanguosha->getCard(sigema), source, Player::Equip, true);

    LogMessage log;
    log.type = "$Shengjian";
    log.from = source;
    log.card_str = Sanguosha->getCard(sigema)->toString();
    room->sendLog(log);

    source->addMark(wpch);
}

class Duanbi: public PhaseChangeSkill{
public:
    Duanbi():PhaseChangeSkill("duanbi"){
        frequency = Wake;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target)
                && target->getPhase() == Player::Judge
                && target->getMark("duanbi") == 0
                && target->getMark("stswd") >= 8;
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        Room *room = player->getRoom();

        LogMessage log;
        log.type = "#DuanbiWake";
        log.from = player;
        room->sendLog(log);

        player->setMark("duanbi", 1);

        player->throwAllCards();
        room->acquireSkill(player, "paoxiao");

        return false;
    }
};

class Duanbipre: public PhaseChangeSkill{
public:
    Duanbipre():PhaseChangeSkill("#duanbip"){
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getPhase() == Player::Finish && target->getMark("stswd") == 7;
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        Room *room = player->getRoom();

        LogMessage log;
        log.type = "#DuanbiHide";
        log.from = player;
        room->sendLog(log);

        room->acquireSkill(player, "duanbi");

        return false;
    }
};

class JiguangClear: public TriggerSkill{
public:
    JiguangClear():TriggerSkill("#jiguang-clear"){
        events << Death;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill("jiguang");
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &) const{
        Room *room = player->getRoom();
        foreach(ServerPlayer *tmp, room->getAlivePlayers()){
            if(room->findPlayerBySkillName("jiguang")  == tmp)
                return false;
        }
        foreach(ServerPlayer *tmp, room->getAlivePlayers()){
            if(tmp->getPile("gas").isEmpty())
                continue;
            foreach(int i, tmp->getPile("gas"))
                room->throwCard(i);
        }
        return false;
    }
};

class Jiguang: public TriggerSkill{
public:
    Jiguang():TriggerSkill("jiguang"){
        events << Damage << Damaged;
        frequency = Frequent;
    }

    virtual int getPriority() const{
        return -1;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        DamageStruct damage = data.value<DamageStruct>();

        ServerPlayer *target = player == damage.from ? damage.to : damage.from;
        if(target->isAlive() && !target->isKongcheng() && player->askForSkillInvoke(objectName())){
            for(int i = damage.damage; i > 0; i--){
                if(target && !target->isKongcheng()){
                    int card_id = room->askForCardChosen(player, target, "h", "jiguang");
                    target->addToPile("gas", card_id);
                }
            }
        }
        return false;
    }
};

class Wangqi: public PhaseChangeSkill{
public:
    Wangqi():PhaseChangeSkill("wangqi"){
        frequency = Frequent;
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        Room *room = player->getRoom();

        if(player->getPhase() == Player::Judge && player->askForSkillInvoke(objectName())){
            JudgeStruct judge;
            judge.pattern = QRegExp("(.*):(heart|diamond):(.*)");
            judge.good = true;
            judge.reason = "wangqi";
            judge.who = player;

            room->judge(judge);

            int count = 0;
            foreach(ServerPlayer *p, room->getAllPlayers()){
                foreach(int i, p->getPile("gas")){
                    room->moveCardTo(Sanguosha->getCard(i), judge.isGood() ? player : p, Player::Hand);
                    count++;
                }
            }
            if(judge.isGood()){
                QList<int> yiji_cards = player->handCards().mid(player->getHandcardNum() - count);
                while(room->askForYiji(player, yiji_cards));
            }
        }
        return false;
    }
};

class Mogong: public TriggerSkill{
public:
    Mogong():TriggerSkill("mogong"){
        events << CardUsed;
    }

    virtual bool triggerable(const ServerPlayer *player) const{
        return !player->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        CardUseStruct use = data.value<CardUseStruct>();
        if(!use.card->inherits("Slash"))
            return false;
        Room *room = player->getRoom();
        ServerPlayer *pisces = room->findPlayerBySkillName(objectName());
        if(!pisces)
            return false;
        if(use.to.length() == 1 && use.to.contains(pisces))
            return false;
        const Card *card = room->askForCard(pisces, ".C", "mogongshow", data);
        if(card){
            pisces->obtainCard(card);
            //Show(pisces, use.from, objectName())->getSuit() == Card::Club)
            room->showCard(pisces, card->getEffectiveId());
            use.to.clear();
            use.to << pisces;
            data = QVariant::fromValue(use);
        }

        return false;
    }
};

class Jingji:public MasochismSkill{
public:
    Jingji():MasochismSkill("jingji"){
        default_choice = "draw";
    }

    virtual void onDamaged(ServerPlayer *pisces, const DamageStruct &damage) const{
        Room *room = pisces->getRoom();

        QString choice = room->askForChoice(pisces, objectName(), "draw+throw+cancel");
        if(choice == "draw"){
            int x = damage.damage, i;
            for(i=0; i<x; i++)
                pisces->drawCards(2);
        }
        else if(choice == "throw" && !damage.from->isNude()){
            room->askForDiscard(damage.from, objectName(), qMin(2, damage.from->getCardCount(true)), false, true);
        }
    }
};

GoldSeintoPrePackage::GoldSeintoPrePackage()
    :Package("goldseintopre")
{
    // xiaoqiao and yuji is not in this package
    General *aries, *taurus, *gemini, *cancer, *leo, *virgo,
    *libra, *scorpio, *sagittarius, *capricorn, *aquarius, *pisces;

    aries = new General(this, "aries$", "st");
    aries->addSkill(new Jingqiang);
    aries->addSkill(new JingqiangClear);
    aries->addSkill(new Zhuyi);
    aries->addSkill(new Zhuanling);
    related_skills.insertMulti("jingqiang", "#jingqiang-clear");

    taurus = new General(this, "taurus", "st");
    taurus->addSkill(new Haojiao);

    gemini = new General(this, "gemini", "st");
    gemini->addSkill(new Disui);
    gemini->addSkill(new Ciyuan);

    cancer = new General(this, "cancer", "st");
    cancer->addSkill(new Shiqi);
    cancer->addSkill(new ShiqiEffect);
    related_skills.insertMulti("shiqi", "#shiqi-effect");

    leo = new General(this, "leo", "st");
    leo->addSkill(new Leiguang);
    leo->addSkill(new Juexiao);
    leo->addSkill(new Chaoyue);

    virgo = new General(this, "virgo", "st");
    virgo->addSkill(new Tianmo);
    virgo->addSkill(new Liudao);
    virgo->addSkill(new Baolun);

    libra = new General(this, "libra", "st");
    libra->addSkill(new Longba);
    libra->addSkill(new Longfei);

    scorpio = new General(this, "scorpio", "st");
    scorpio->addSkill(new Zhiyan);

    sagittarius = new General(this, "sagittarius", "st");
    sagittarius->addSkill(new Jingong);
    sagittarius->addSkill(new Shanyao);

    capricorn = new General(this, "capricorn", "st");
    capricorn->addSkill(new Shengjian);
    capricorn->addSkill(new Duanbipre);
    related_skills.insertMulti("shengjian", "#duanbip");

    aquarius = new General(this, "aquarius", "st", 3);
    aquarius->addSkill(new Jiguang);
    aquarius->addSkill(new JiguangClear);
    aquarius->addSkill(new Wangqi);
    related_skills.insertMulti("jiguang", "#jiguang-clear");

    pisces = new General(this, "pisces", "st", 3);
    pisces->addSkill(new Mogong);
    pisces->addSkill(new Jingji);

    addMetaObject<ZhuyiCard>();
    addMetaObject<JiaohuangCard>();
    addMetaObject<HaojiaoCard>();
    addMetaObject<ShiqiCard>();
    addMetaObject<BaolunCard>();
    addMetaObject<ZhiyanCard>();
    addMetaObject<ShengjianCard>();

    skills << new Jiaohuang << new JiaohuangViewAsSkill << new Duanbi;
}

ADD_PACKAGE(GoldSeintoPre)
