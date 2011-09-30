#include "standard.h"
#include "skill.h"
#include "gold-seinto.h"
#include "client.h"
#include "carditem.h"
#include "engine.h"
#include "maneuvering.h"
#include "ai.h"

class Jingqiang: public PhaseChangeSkill{
public:
    Jingqiang():PhaseChangeSkill("jingqiang"){
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        Room *room = player->getRoom();
        if(player->getPhase() == Player::Finish && room->askForSkillInvoke(player, objectName())){
            ServerPlayer *target = room->askForPlayerChosen(player, room->getAllPlayers(), "jq-choose");
            target->gainMark("@jq");
            room->acquireSkill(target, "jqdis");
        }
        else if(player->getPhase() == Player::Start){
            foreach(ServerPlayer *p, room->getAllPlayers()){
                if(p->getMark("@jq") > 0)
                    p->loseMark("@jq");
            }
        }

        return false;
    }
};

class Jingqiangdis: public DistanceSkill{
public:
    Jingqiangdis():DistanceSkill("#jqdis"){

    }
    virtual int getCorrect(const Player *from, const Player *to) const{
        if(to->getMark("@jq") > 0)
            return +1;
        else
            return 0;
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
        if(aries->getMark("jhv") == 0)
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

class Haojiao: public PhaseChangeSkill{
public:
    Haojiao():PhaseChangeSkill("haojiao"){
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target)
                && target->getPhase() == Player::Finish;
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        Room *room = player->getRoom();

        if(room->askForSkillInvoke(player, objectName())){
            player->turnOver();

            QList<ServerPlayer *> players = room->getOtherPlayers(player);
            foreach(ServerPlayer *p, players){
                if(!player->canSlash(p))
                    players.removeOne(p);
            }

            ServerPlayer *target1 = NULL, *target2 = NULL;
            if(players.length() > 0){
                target1 = room->askForPlayerChosen(player, players, "haojiao");
                players.removeOne(target1);
            }
            if(players.length() > 0)
                target2 = room->askForPlayerChosen(player, players, "haojiao");
            if(!target1 && !target2)
                return false;

            Slash *slash = new Slash(Card::NoSuit, 0);
            slash->setSkillName(objectName());

            CardUseStruct card_use;
            card_use.card = slash;
            card_use.from = player;
            if(target1)
                card_use.to << target1;
            if(target2)
                card_use.to << target2;
            room->useCard(card_use, false);
        }

        return false;
    }
};

class Haojiaof: public MasochismSkill{
public:
    Haojiaof():MasochismSkill("#haojiaof"){
    }
    virtual void onDamaged(ServerPlayer *taurus, const DamageStruct &) const{
        if(taurus->faceUp())
            return;
        if(!taurus->askForSkillInvoke("haojiao"))
            return;
        taurus->turnOver();
    }
};

HuanlongCard::HuanlongCard(){
    will_throw = false;
    once = true;
}

bool HuanlongCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select != Self && !to_select->isKongcheng();
}

void HuanlongCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    ServerPlayer *sunce = targets.first();
    source->pindian(sunce, "huanlong", this);
}

class HuanlongPindian: public OneCardViewAsSkill{
public:
    HuanlongPindian():OneCardViewAsSkill("huanlong_pindian"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->isKongcheng() && ! player->hasUsed("HuanlongCard");
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return ! to_select->isEquipped();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        HuanlongCard *card = new HuanlongCard;
        card->addSubcard(card_item->getFilteredCard());

        return card;
    }
};

class Huanlong: public TriggerSkill{
public:
    Huanlong():TriggerSkill("huanlong"){
        view_as_skill = new HuanlongPindian;
        events << Pindian;
    }

    virtual int getPriority() const{
        return -1;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        PindianStar pindian = data.value<PindianStar>();
        if(pindian->reason == "huanlong" && pindian->from == player){
            ServerPlayer *winner = pindian->from_card->getNumber() > pindian->to_card->getNumber() ? pindian->from : pindian->to;
            winner->obtainCard(pindian->to_card);
            winner->obtainCard(pindian->from_card);
            if(winner == player)
                player->getRoom()->showAllCards(pindian->to, player);
        }

        return false;
    }
};

class Ciyuan: public TriggerSkill{
public:
    Ciyuan():TriggerSkill("ciyuan"){
        frequency = Wake;
        events << DamageComplete;
    }

    virtual int getPriority() const{
        return -1;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        ServerPlayer *gemini = target->getRoom()->findPlayerBySkillName(objectName());
        return gemini->getMark("ciyuan") == 0;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.nature != DamageStruct::Fire)
            return false;
        Room *room = player->getRoom();
        ServerPlayer *gemini = room->findPlayerBySkillName(objectName());

        LogMessage log;
        log.type = "#CiyuanWake";
        log.from = gemini;
        room->sendLog(log);

        gemini->gainMark("@star", 5);
        gemini->drawCards(5);
        QList<int> stars = gemini->handCards().mid(0, 5);
        foreach(int card_id, stars)
            gemini->addToPile("stars", card_id, false);
/*        for(int star = 1; star < 5; star++)
            gemini->addToPile("stars", room->drawCard());
*/
        gemini->setMark("ciyuan", 1);

        room->loseMaxHp(gemini);
        room->acquireSkill(gemini, "qixing");
        room->acquireSkill(gemini, "dawu");

        return false;
    }
};

class Shiqi: public TriggerSkill{
public:
    Shiqi():TriggerSkill("shiqi"){
        events << AskForPeaches;
        default_choice = "cancel";
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        DyingStruct dying_data = data.value<DyingStruct>();

        if(player != dying_data.who)
            return false;
        Room *room = player->getRoom();
        ServerPlayer *cancer = room->findPlayerBySkillName(objectName());
        if(cancer->getCardCount(true) < 2)
            return false;
        QString choice = room->askForChoice(cancer, objectName(), "recovery+lost+cancel");
        if(choice == "recovery"){
//            dying_data.damage->damage --;
            room->askForDiscard(cancer, "shiqi", 2, false, true);
            RecoverStruct recover;
            recover.who = cancer;
            room->recover(dying_data.who, recover);
        }
        else if(choice == "lost"){
//            dying_data.damage->damage ++;
            room->askForDiscard(cancer, "shiqi", 2, false, true);
            room->loseHp(dying_data.who);
        }

        return false;
    }
};

class Dianguang: public OneCardViewAsSkill{
public:
    Dianguang():OneCardViewAsSkill("dianguang"){

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
/*
    foreach(int card, source->getPile("guo")){
        room->moveCardTo(Sanguosha->getCard(card), target, Player::Hand, false);
        handlog --;
        if(handlog <= 0)
            break;
    }
*/
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
        room->acquireSkill(player, "mashu");

        return false;
    }
};

class Zhiyan:public ZeroCardViewAsSkill{
public:
    Zhiyan():ZeroCardViewAsSkill("zhiyan"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->isWounded();
    }

    virtual const Card *viewAs() const{
        return new ZhiyanCard;
    }
};

ZhiyanCard::ZhiyanCard(){
}

bool ZhiyanCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty();
}

void ZhiyanCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    ServerPlayer *target = targets.first();
    room->loseMaxHp(source);
    DamageStruct damage;
    damage.nature = DamageStruct::Fire;
    damage.from = source;
    damage.to = target;
    room->damage(damage);
}

class Jingong: public TriggerSkill{
public:
    Jingong():TriggerSkill("jingong"){
        events << GameStart << CardLost << HpChanged;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        if(event == GameStart)
            room->setPlayerProperty(player, "atk", player->getAttackRange() + 4);
        else if(event == CardLost){
            CardMoveStar move = data.value<CardMoveStar>();
            if(move->from_place == Player::Equip){
                const Card *card = Sanguosha->getCard(move->card_id);
                const EquipCard *equip = qobject_cast<const EquipCard *>(card);
                if(equip && equip->location() == EquipCard::WeaponLocation)
                    room->setPlayerProperty(player, "atk", player->getAttackRange() + 4);
            }
        }
        return false;
    }
};

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
        room->acquireSkill(player, "liegong");
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
    foreach(QString w, weapons){
        if(source->getMark(w) > 0)
            weapons.removeOne(w);
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

class Jiguang: public TriggerSkill{
public:
    Jiguang():TriggerSkill("jiguang"){
        events << Damage << Damaged;
        frequency = Frequent;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        Room *room = player->getRoom();

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
            judge.pattern = QRegExp("(.*):(spade):(.*)");
            judge.good = false;
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
        if(use.to.length() == 1 && use.to.contains(pisces))
            return false;
        if(room->askForCard(pisces, ".C", "mogongshow", false))
            /*Show(pisces, use.from, objectName())->getSuit() == Card::Club)*/{
            use.to.clear();
            use.to << pisces;
            room->useCard(use);
            return true;
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

GoldSeintoPackage::GoldSeintoPackage()
    :Package("goldseinto")
{
    // xiaoqiao and yuji is not in this package
    General *aries, *taurus, *gemini, *cancer, *leo, *virgo,
    *libra, *scorpio, *sagittarius, *capricorn, *aquarius, *pisces;

    aries = new General(this, "aries$", "st");
    aries->addSkill(new Jingqiang);
    aries->addSkill(new Zhuyi);
    aries->addSkill(new Zhuanling);

    taurus = new General(this, "taurus", "st");
    taurus->addSkill(new Haojiao);
    taurus->addSkill(new Haojiaof);
    related_skills.insertMulti("haojiao", "#haojiaof");

    gemini = new General(this, "gemini", "st");
    gemini->addSkill(new Huanlong);
    gemini->addSkill(new Ciyuan);

    cancer = new General(this, "cancer", "st");
    cancer->addSkill(new Shiqi);

    leo = new General(this, "leo", "st");
    leo->addSkill(new Dianguang);
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
    aquarius->addSkill(new Wangqi);

    pisces = new General(this, "pisces", "st", 3);
    pisces->addSkill(new Mogong);
    pisces->addSkill(new Jingji);

    addMetaObject<ZhuyiCard>();
    addMetaObject<JiaohuangCard>();
    addMetaObject<HuanlongCard>();
    addMetaObject<BaolunCard>();
    addMetaObject<ZhiyanCard>();
    addMetaObject<ShengjianCard>();

    skills << new Jingqiangdis << new Jiaohuang << new JiaohuangViewAsSkill << new Duanbi;
}

ADD_PACKAGE(GoldSeinto)
