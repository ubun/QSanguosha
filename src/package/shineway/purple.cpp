#include "purple.h"
#include "skill.h"
#include "client.h"
#include "engine.h"
#include "carditem.h"
#include "room.h"

class Zhuosu:public MasochismSkill{
public:
    Zhuosu():MasochismSkill("zhuosu"){
    }

    virtual void onDamaged(ServerPlayer *yelai, const DamageStruct &damage) const{
        Room *room = yelai->getRoom();
        if(yelai->askForSkillInvoke(objectName())){
            ServerPlayer *player = room->askForPlayerChosen(yelai, room->getOtherPlayers(yelai), objectName());
            if(yelai->faceUp())
                player->gainAnExtraTurn();
            else
                player->turnOver();
            yelai->turnOver();
        }
    }
};

class Difu: public TriggerSkill{
public:
    Difu():TriggerSkill("difu"){
        events << Dying;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *pp, QVariant &data) const{
        if(pp->isNude())
            return false;
        Room *room = pp->getRoom();
        if(pp->askForSkillInvoke(objectName())){
            ServerPlayer *taregt = room->askForPlayerChosen(pp, room->getOtherPlayers(pp), objectName());
            taregt->drawCards(pp->getCardCount(true));
            DyingStruct dying_data = data.value<DyingStruct>();
            room->killPlayer(pp, dying_data.damage);
            return true;
        }
        return false;
    }
};

class Bui: public TriggerSkill{
public:
    Bui():TriggerSkill("bui"){
        events << Death;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        DamageStar damage = data.value<DamageStar>();
        ServerPlayer *killer = damage ? damage->from : NULL;

        if(killer && killer != player)
            killer->gainMark("@noequip");
        return false;
    }
};

class Wuxian: public TriggerSkill{
public:
    Wuxian():TriggerSkill("wuxian"){
        events << CardGotDone << CardDrawnDone << PhaseChange;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent e, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        if(e == PhaseChange){
            if(!room->getTag("Wuxian").toBool() && player->getPhase() == Player::Judge)
                room->setTag("Wuxian", true);
            return false;
        }
        if(!room->getTag("Wuxian").toBool())
            return false;
        ServerPlayer *nichang = room->findPlayerBySkillName(objectName());
        if(nichang == player){
            if(nichang->getHandcardNum() > 5){
                int n = nichang->getHandcardNum() - nichang->getMaxHP();
                if(n > 0)
                    room->askForDiscard(nichang, objectName(), n);
            }
            return false;
        }
        if(room->getCurrent() == player)
            return false;
        if(nichang && nichang->askForSkillInvoke(objectName()))
            nichang->drawCards(1);
        return false;
    }
};

class Xianming: public TriggerSkill{
public:
    Xianming():TriggerSkill("xianming"){
        events << CardEffected << CardEffect;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *miug, QVariant &data) const{
        //Room *room = miug->getRoom();
        CardEffectStruct effect = data.value<CardEffectStruct>();
        if(!effect.card->inherits("Snatch") && !effect.card->inherits("Dismantlement"))
            return false;
        if(miug->askForSkillInvoke(objectName(), QVariant::fromValue(effect.card->objectName()))){
            Card::Suit suit = effect.card->getSuit();
            int num = effect.card->getNumber();
            if(effect.card->inherits("Snatch")){
                Dismantlement *d = new Dismantlement(suit, num);
                d->setSkillName(objectName());
                d->addSubcard(effect.card);
                effect.card = d;
            }
            else{
                Snatch *s = new Snatch(suit, num);
                s->setSkillName(objectName());
                s->addSubcard(effect.card);
                effect.card = s;
            }
            data = QVariant::fromValue(effect);
        }
        return false;
    }
};

class Xiannei: public TriggerSkill{
public:
    Xiannei():TriggerSkill("xiannei"){
        events << CardLost;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getHandcardNum() < target->getMaxCards();
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        CardMoveStar move = data.value<CardMoveStar>();
        Room *room = player->getRoom();
        ServerPlayer *current = room->getCurrent();
        ServerPlayer *xing = room->findPlayerBySkillName(objectName());
        if(player->isDead() || !xing || move->to == player || player->hasFlag("nie"))
            return false;
        if(move->from_place == Player::Hand || move->from_place == Player::Equip){
            if(!current->hasFlag("nei") && !xing->isNude() && room->askForSkillInvoke(xing, objectName(), QVariant::fromValue((PlayerStar)player))){
                room->playSkillEffect(objectName());
                xing->setFlags("nie");
                if(room->askForDiscard(xing, objectName(), 1, false, true)){
                    QString ch = !player->isWounded()? "draw":
                                 room->askForChoice(xing, objectName(), "draw+hp");
                    if(ch == "draw")
                        player->drawCards(2);
                    else{
                        RecoverStruct r;
                        r.who = xing;
                        room->recover(player, r);
                    }
                    room->setPlayerFlag(current, "nei");
                }
                xing->setFlags("-nie");
            }
        }
        return false;
    }
};

class Shangjue: public TriggerSkill{
public:
    Shangjue():TriggerSkill("shangjue"){
        events << CardLost;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *zous, QVariant &data) const{
        if(zous->getPhase() == Player::NotActive && zous->isKongcheng()){
            CardMoveStar move = data.value<CardMoveStar>();
            if(move->from_place == Player::Hand){
                Room *room = zous->getRoom();
                if(room->askForSkillInvoke(zous, objectName())){
                    QString chs = room->askForChoice(zous, objectName(), "draw+damage");
                    room->playSkillEffect(objectName());
                    if(chs == "draw")
                        zous->drawCards(2);
                    else{
                        DamageStruct damage;
                        damage.from = zous;
                        damage.to = room->askForPlayerChosen(zous, room->getAllPlayers(), objectName());
                        room->damage(damage);
                    }
                }
            }
        }
        return false;
    }
};

class Quling: public PhaseChangeSkill{
public:
    Quling():PhaseChangeSkill("quling"){
        frequency = Wake;
    }

    static QList<ServerPlayer *> getFriends(PlayerStar me, QString kingdom){
        Room *room = me->getRoom();
        QList<ServerPlayer *> targets;
        foreach(ServerPlayer *tmp, room->getOtherPlayers(me)){
            if(tmp->getKingdom() == kingdom)
                targets << tmp;
        }
        return targets;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target)
                && target->getMark("quling") == 0
                && target->getPhase() == Player::Start
                && getFriends((PlayerStar)target, "qun").isEmpty();
    }

    virtual bool onPhaseChange(ServerPlayer *zou) const{
        Room *room = zou->getRoom();

        LogMessage log;
        log.type = "#QulingWake";
        log.from = zou;
        room->sendLog(log);

        room->loseMaxHp(zou);
        room->setPlayerProperty(zou, "kingdom", "wei");
        room->acquireSkill(zou, "huoshui");
        room->setPlayerMark(zou, "quling", 1);

        return false;
    }
};

class Huoshui: public TriggerSkill{
public:
    Huoshui():TriggerSkill("huoshui"){
        events << Dying;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *pp, QVariant &data) const{
        Room *room = pp->getRoom();
        if(pp->askForSkillInvoke(objectName())){
            foreach(ServerPlayer *tmp, room->getAllPlayers())
                if(tmp->getGeneral()->isMale())
                    room->loseHp(tmp);
            DyingStruct dying_data = data.value<DyingStruct>();
            room->killPlayer(pp, dying_data.damage);
            return true;
        }
        return false;
    }
};

class PoxieBuff: public TriggerSkill{
public:
    PoxieBuff():TriggerSkill("#poxie"){
        events << CardEffect;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasFlag("poxie") && target->isAlive();
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *duanmt, QVariant &data) const{
        CardEffectStruct effect = data.value<CardEffectStruct>();
        if(!effect.card->inherits("BasicCard") && !effect.card->inherits("TrickCard"))
            return false;
        PlayerStar target = duanmt->tag["PoxieTarget"].value<PlayerStar>();
        if(!target || duanmt->hasFlag("Poxie"))
            return false;
        Room *room = duanmt->getRoom();
        QString suit_str = effect.card->getSuitString();
        QString pattern = QString(".%1").arg(suit_str.at(0).toUpper());
        QString prompt = QString("@poxie:%1::%2").arg(duanmt->getGeneralName()).arg(suit_str);
        if(!room->askForCard(target, pattern, prompt)){
            DamageStruct dmg;
            dmg.from = duanmt;
            dmg.to = target;
            room->damage(dmg);
        }
        duanmt->setFlags("Poxie");
        duanmt->tag.remove("PoxieTarget");
        return false;
    }
};

class Poxie: public DrawCardsSkill{
public:
    Poxie():DrawCardsSkill("poxie"){
    }

    virtual int getDrawNum(ServerPlayer *duanmt, int n) const{
        Room *room = duanmt->getRoom();
        if(room->askForSkillInvoke(duanmt, objectName())){
            room->playSkillEffect(objectName());
            PlayerStar target = room->askForPlayerChosen(duanmt, room->getAllPlayers(), objectName());
            duanmt->tag["PoxieTarget"] = QVariant::fromValue(target);
            duanmt->setFlags(objectName());
            return n - 1;
        }else
            return n;
    }
};

class Xiangfeng: public TriggerSkill{
public:
    Xiangfeng():TriggerSkill("xiangfeng"){
        events << SlashMissed;
        frequency = Frequent;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *dm = room->findPlayerBySkillName(objectName());
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        if(dm == effect.to && dm->askForSkillInvoke(objectName())){
            dm->obtainCard(effect.slash);
        }
        return false;
    }
};

class Xiayi: public TriggerSkill{
public:
    Xiayi():TriggerSkill("xiayi"){
        events << Predamaged;
    }

    virtual int getPriority() const{
        return 2;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getGeneral()->isMale();
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        DamageStruct damage = data.value<DamageStruct>();
        ServerPlayer *bao3niang = room->findPlayerBySkillName(objectName());
        if(!bao3niang || !damage.from || damage.from == damage.to)
            return false;
        if(room->askForCard(bao3niang, "slash", "@xiayi", QVariant::fromValue((PlayerStar)damage.from))){
            Slash *slash = new Slash(Card::NoSuit, 0);
            slash->setSkillName(objectName());
            CardUseStruct use;
            use.card = slash;
            use.from = damage.to;
            use.to << damage.from;

            room->useCard(use);
        }
        return false;
    }
};

class Chongguan: public TriggerSkill{
public:
    Chongguan():TriggerSkill("chongguan"){
        events << FinishJudge;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual int getPriority() const{
        return -1;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *bulianshi = room->findPlayerBySkillName(objectName());
        if(!bulianshi)
            return false;
        JudgeStar judge = data.value<JudgeStar>();
        const Card *card = judge->card;
        if(card->getSuit() == Card::Diamond &&
           bulianshi->askForSkillInvoke(objectName())){
            Indulgence *indulgence = new Indulgence(card->getSuit(), card->getNumber());
            indulgence->setSkillName(objectName());
            indulgence->addSubcard(card);
            QList<ServerPlayer *> targets;
            foreach(ServerPlayer *p, room->getAlivePlayers()){
                if(bulianshi->isProhibited(p, indulgence))
                    continue;
                if(p->containsTrick("indulgence"))
                    continue;
                targets << p;
            }
            if(targets.isEmpty())
                return false;

            ServerPlayer *target = room->askForPlayerChosen(bulianshi, targets, objectName());
            CardUseStruct use;
            use.card = indulgence;
            use.from = bulianshi;
            use.to << target;
            room->playSkillEffect(objectName());
            room->useCard(use);
        }
        if(judge->reason == "indulgence" && bulianshi->askForSkillInvoke(objectName())){
            room->setPlayerFlag(bulianshi, "fre");
            ServerPlayer *target = room->askForPlayerChosen(bulianshi, room->getAllPlayers(), objectName());
            room->setPlayerFlag(bulianshi, "-fre");
            if(target->getGeneral()->isMale()){
                QString chost = room->askForChoice(bulianshi, objectName(), "draw+playy");
                QList<Player::Phase> phase;
                if(chost == "draw")
                    phase << Player::Draw;
                else
                    phase << Player::Play;
                target->play(phase);
            }
        }
        return false;
    }
};

class Xianhou: public PhaseChangeSkill{
public:
    Xianhou():PhaseChangeSkill("xianhou"){
    }

    virtual bool onPhaseChange(ServerPlayer *ayumi) const{
        if(ayumi->getPhase() == Player::Draw ||
           ayumi->getPhase() == Player::Play){
            Room *room = ayumi->getRoom();
            if(!ayumi->hasFlag("xianh") && room->askForSkillInvoke(ayumi, objectName())){
                ServerPlayer *target = room->askForPlayerChosen(ayumi, room->getAllPlayers(), objectName());
                QString chost = room->askForChoice(ayumi, objectName(), "draw+hp");
                if(chost == "draw")
                    target->drawCards(2);
                else{
                    RecoverStruct d;
                    d.who = ayumi;
                    room->recover(target, d);
                }
                ayumi->setFlags("xianh");
                return true;
            }
        }
        return false;
    }
};

class Nvwang: public FilterSkill{
public:
    Nvwang():FilterSkill("nvwang"){
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getCard()->inherits("Slash");
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *c = card_item->getCard();
        Peach *peach = new Peach(c->getSuit(), c->getNumber());
        peach->setSkillName(objectName());
        peach->addSubcard(card_item->getCard());
        return peach;
    }
};

ShouguoCard::ShouguoCard(){
    once = true;
}

bool ShouguoCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;
    return to_select->getGeneral()->isMale() && to_select->isWounded() &&
            to_select->getHandcardNum() > 1;
}

void ShouguoCard::onEffect(const CardEffectStruct &effect) const{
    effect.from->obtainCard(effect.to->getRandomHandCard());
    effect.from->obtainCard(effect.to->getRandomHandCard());
    RecoverStruct rev;
    rev.card = this;
    rev.who = effect.from;
    effect.from->getRoom()->recover(effect.to, rev);
}

class Shouguo: public ZeroCardViewAsSkill{
public:
    Shouguo():ZeroCardViewAsSkill("shouguo"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("ShouguoCard");
    }

    virtual const Card *viewAs() const{
        return new ShouguoCard;
    }
};

YaofaCard::YaofaCard(){
}

bool YaofaCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;
    return to_select != Self;
}

void YaofaCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    bool ichi, me;

    JudgeStruct judge;
    judge.reason = "yaofa1";
    judge.who = effect.to;
    room->judge(judge);
    ichi = judge.card->isRed();

    judge.reason = "yaofa2";
    judge.pattern = ichi ? QRegExp("(.*):(heart|diamond):(.*)"): QRegExp("(.*):(club|spade):(.*)");
    judge.good = false;
    room->judge(judge);
    me = judge.card->isRed();

    DamageStruct damage;
    damage.from = effect.from;
    damage.to = effect.to;
    if(ichi && me){
        damage.nature = DamageStruct::Fire;
        room->damage(damage);
    }
    else if(!ichi && !me){
        damage.nature = DamageStruct::Thunder;
        room->damage(damage);
    }
}

class YaofaViewAsSkill: public ZeroCardViewAsSkill{
public:
    YaofaViewAsSkill():ZeroCardViewAsSkill("yaofa"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "@@yaofa";
    }

    virtual const Card *viewAs() const{
        return new YaofaCard;
    }
};

class Yaofa: public MasochismSkill{
public:
    Yaofa():MasochismSkill("yaofa"){
        view_as_skill = new YaofaViewAsSkill;
    }

    virtual void onDamaged(ServerPlayer *qiaodaoq, const DamageStruct &) const{
        qiaodaoq->getRoom()->askForUseCard(qiaodaoq, "@@yaofa", "@yaofa");
    }
};

class Cizhen: public TriggerSkill{
public:
    Cizhen():TriggerSkill("cizhen"){
        events << CardUsed;
    }

    virtual bool triggerable(const ServerPlayer *sb) const{
        return !sb->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        CardUseStruct use = data.value<CardUseStruct>();
        if(!use.card->inherits("Peach"))
            return false;
        Room *room = player->getRoom();
        ServerPlayer *guos = room->findPlayerBySkillName(objectName());
        if(guos && !guos->isNude() && guos->askForSkillInvoke(objectName(), data)){
            room->askForDiscard(guos, objectName(), 1, false, true);

            JudgeStruct judge;
            judge.pattern = QRegExp("(.*):(club|spade):(.*)");
            judge.good = true;
            judge.reason = objectName();
            judge.who = guos;

            room->judge(judge);
            if(judge.isGood())
                return true;
        }
        return false;
    }
};

class Woxuan: public MasochismSkill{
public:
    Woxuan():MasochismSkill("woxuan"){
    }

    virtual void onDamaged(ServerPlayer *guonvwang, const DamageStruct &damage) const{
        if(!damage.from)
            return;
        Room *room = guonvwang->getRoom();
        const QString patl = qAbs(guonvwang->getHandcardNum() - damage.from->getHandcardNum()) <= guonvwang->getLostHp() ?
                   "woturn+woexcha+wocancel" : "woturn+wocancel";
        QString choice = room->askForChoice(guonvwang, objectName(), patl);
        if(choice == "wocancel")
            return;
        LogMessage log;
        log.from = guonvwang;
        log.type = "#Woxuan";
        log.arg = objectName();
        log.arg2 = choice;
        room->sendLog(log);
        if(choice == "woturn"){
            damage.from->turnOver();
            damage.to->turnOver();
        }
        else{
            int n1 = damage.from->getHandcardNum();
            int n2 = damage.to->getHandcardNum();

            DummyCard *card1 = damage.from->wholeHandCards();
            DummyCard *card2 = damage.to->wholeHandCards();

            if(card1){
                room->moveCardTo(card1, damage.to, Player::Hand, false);
                delete card1;
            }
            room->getThread()->delay();

            if(card2){
                room->moveCardTo(card2, damage.from, Player::Hand, false);
                delete card2;
            }
            LogMessage log;
            log.type = "#Dimeng";
            log.from = damage.from;
            log.to << damage.to;
            log.arg = QString::number(n1);
            log.arg2 = QString::number(n2);
            room->sendLog(log);
        }
    }
};

PurplePackage::PurplePackage()
    :Package("Purple")
{
    General *purpleyelai = new General(this, "purpleyelai", "wei", 3, false);
    purpleyelai->addSkill(new Zhuosu);
    purpleyelai->addSkill(new Difu);
    purpleyelai->addSkill(new Bui);

    General *purplesunnichang = new General(this, "purplesunnichang", "wu", 3, false);
    purplesunnichang->addSkill(new Wuxian);
    purplesunnichang->addSkill(new Xianming);

    General *purplexingcai = new General(this, "purplexingcai", "shu", 4, false);
    purplexingcai->addSkill(new Xiannei);

    General *purpleguozhao = new General(this, "purpleguozhao", "wei", 3, false);
    purpleguozhao->addSkill(new Woxuan);
    purpleguozhao->addSkill(new Cizhen);

    General *purpleduanmeng = new General(this, "purpleduanmeng", "shu", 3, false);
    purpleduanmeng->addSkill(new Poxie);
    purpleduanmeng->addSkill(new PoxieBuff);
    related_skills.insertMulti("poxie", "#poxie");
    purpleduanmeng->addSkill(new Xiangfeng);
    purpleduanmeng->addSkill(new Skill("fuchou", Skill::Compulsory));

    General *purplezoushi = new General(this, "purplezoushi", "qun", 4, false);
    purplezoushi->addSkill(new Shangjue);
    purplezoushi->addSkill(new Quling);
    skills << new Huoshui;

    General *purplesunbushi = new General(this, "purplesunbushi", "wu", 3, false);
    purplesunbushi->addSkill(new Chongguan);
    purplesunbushi->addSkill(new Xianhou);

    General *purplebao3niang = new General(this, "purplebao3niang", "shu", 4, false);
    purplebao3niang->addSkill(new Xiayi);

    General *purplebeimihu = new General(this, "purplebeimihu", "qun", 3, false, true);
    //purplebeimihu->addSkill(new Nvwang);
    purplebeimihu->addSkill(new Shouguo);
    purplebeimihu->addSkill(new Yaofa);

    addMetaObject<ShouguoCard>();
    addMetaObject<YaofaCard>();
}

ADD_PACKAGE(Purple)
