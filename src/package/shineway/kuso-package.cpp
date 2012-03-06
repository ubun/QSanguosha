#include "kuso-package.h"
#include "skill.h"
#include "client.h"
#include "engine.h"
#include "carditem.h"
#include "room.h"
#include "maneuvering.h"
#include "settings.h"

class Huaxu: public TriggerSkill{
public:
    Huaxu():TriggerSkill("huaxu"){
        events << Damage << TurnStart;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        if(event == TurnStart){
            foreach(ServerPlayer *tmp, player->getRoom()->getAlivePlayers()){
                for(int i = 1; i <= 4; i++){
                    QString mark_name = "@hana" + QString::number(i);
                    if(tmp->getMark(mark_name) > 0)
                        tmp->loseAllMarks(mark_name);
                }
            }
            return false;
        }
        if(player->getPhase() == Player::NotActive)
            return false;
        DamageStruct damage = data.value<DamageStruct>();
        if(!damage.card || damage.card->getSuit() == Card::NoSuit)
            return false;
        Room *room = player->getRoom();
        if(damage.to->isDead())
            return false;
        if(player->askForSkillInvoke(objectName(), data)){
            int i = 0;
            switch(damage.card->getSuit()){
            case Card::Heart : i = 1; break;
            case Card::Diamond : i = 2; break;
            case Card::Spade : i = 3; break;
            case Card::Club : i = 4; break;
            default: break;
            }
            QString mark_name = "@hana" + QString::number(i);
            if(damage.to->getMark(mark_name) < 1)
                damage.to->gainMark(mark_name);
            LogMessage log;
            log.type = "#Gotoshit";
            log.from = player;
            log.to << damage.to;
            log.arg = Card::Suit2String(damage.card->getSuit());
            room->sendLog(log);
        }
        return false;
    }
};

class HuaxuEffect:public TriggerSkill{
public:
    HuaxuEffect():TriggerSkill("#huaxu_eft"){
        events << CardUsed;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        for(int i = 1; i <= 4; i++){
            QString mark_name = "@hana" + QString::number(i);
            if(target->getMark(mark_name) > 0)
                return true;
        }
        return false;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        if(player->getPhase() == Player::NotActive)
            return false;
        CardUseStruct use = data.value<CardUseStruct>();
        CardStar card = use.card;
        Card::Suit suit = card->getSuit();
        Room *room = player->getRoom();
        if(card && suit != Card::NoSuit){
            LogMessage log;
            log.from = player;
            log.card_str = card->getEffectIdString();
            if(suit == Card::Heart && player->getMark("@hana1") > 0){
                log.type = "$HuaxuDamage1";
                room->sendLog(log);
                DamageStruct damage;
                damage.from = damage.to = player;
                damage.card = card;
                //damage.damage = player->getMark("@hana1");
                damage.nature = DamageStruct::Fire;
                room->throwCard(card);
                room->damage(damage);
            }
            else if(suit == Card::Diamond && player->getMark("@hana2") > 0){
                log.type = "$HuaxuDamage2";
                room->sendLog(log);
                DamageStruct damage;
                damage.from = damage.to = player;
                damage.card = card;
                //damage.damage = player->getMark("@hana2");
                room->throwCard(card);
                room->damage(damage);
            }
            else if(suit == Card::Spade && player->getMark("@hana3") > 0){
                log.type = "$HuaxuDamage3";
                room->sendLog(log);
                //room->loseHp(player, player->getMark("@hana3"));
                room->throwCard(card);
                room->loseHp(player);
            }
            else if(suit == Card::Club && player->getMark("@hana4") > 0){
                log.type = "$HuaxuDamage4";
                room->sendLog(log);
                DamageStruct damage;
                damage.from = damage.to = player;
                damage.card = card;
                //damage.damage = player->getMark("@hana4");
                damage.nature = DamageStruct::Thunder;
                room->throwCard(card);
                room->damage(damage);
            }
            if(player->isDead())
                return true;
        }
        return false;
    }
};

class Liaoting:public ViewAsSkill{
public:
    Liaoting():ViewAsSkill("liaoting"){
        frequency = Limited;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getMark("@liaot") >= 1;
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(selected.length() > 4)
            return false;
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() != 4)
            return NULL;

        LiaotingCard *card = new LiaotingCard;
        card->addSubcards(cards);

        return card;
    }
};

LiaotingCard::LiaotingCard(){
    target_fixed = true;
}

void LiaotingCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    QList<int> subs = this->getSubcards();
    source->loseMark("@liaot");
    foreach(int tmp, subs){
        if(!Sanguosha->getCard(tmp)->inherits("Shit")){
            room->throwCard(this);
            return;
        }
    }
    if(source->isLord() || source->getRole() == "loyalist"){
        foreach(ServerPlayer *tmp, room->getAlivePlayers())
            if(tmp->getRole() == "renegade" || tmp->getRole() == "rebel")
                room->killPlayer(tmp);
    }
    else if(source->getRole() == "renegade"){
        source->drawCards(5, false);
        foreach(ServerPlayer *tmp, room->getAlivePlayers())
            if(tmp->getRole() == "loyalist" || tmp->getRole() == "rebel")
                room->killPlayer(tmp);
    }
    else if(source->getRole() == "rebel")
        room->killPlayer(room->getLord());

    room->throwCard(this);
    if(source->isAlive())
        room->setPlayerProperty(source, "hp", source->getMaxHP());
}

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

class EatDeath: public TriggerSkill{
public:
    EatDeath():TriggerSkill("eatdeath"){
        events << Death;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *tenkei = room->findPlayerBySkillName(objectName());
        if(!tenkei)
            return false;
        QVariantList eatdeath_skills = tenkei->tag["EatDeath"].toList();
        if(room->askForSkillInvoke(tenkei, objectName(), data)){
            QStringList eatdeaths;
            foreach(QVariant tmp, eatdeath_skills)
                eatdeaths << tmp.toString();
            if(!eatdeaths.isEmpty()){
                QString choice = room->askForChoice(tenkei, objectName(), eatdeaths.join("+"));
                room->detachSkillFromPlayer(tenkei, choice);
                eatdeath_skills.removeOne(choice);
            }
            room->loseMaxHp(tenkei);
            QList<const Skill *> skills = player->getVisibleSkillList();
            foreach(const Skill *skill, skills){
                if(skill->parent()){
                    QString sk = skill->objectName();
                    room->acquireSkill(tenkei, sk);
                    eatdeath_skills << sk;
                }
            }
            tenkei->tag["EatDeath"] = eatdeath_skills;
        }

        return false;
    }
};

class SuperJuejing: public TriggerSkill{
public:
    SuperJuejing():TriggerSkill("super_juejing"){
        events << GameStart << PhaseChange
                << CardLostDone << CardDrawnDone << CardGotDone;
        frequency = Compulsory;
    }

    virtual int getPriority() const{
        return 2;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        if(event == GameStart){
            player->getRoom()->setPlayerMark(player, "SuperMan", 1);
            if(player->getMaxHP() < 1)
                player->getRoom()->setPlayerProperty(player, "maxhp", 1);
            return false;
        }
        Room *room = player->getRoom();
        if(event == PhaseChange){
            if(player->getPhase() == Player::Draw)
                return true;
            return false;
        }

        int handcard = player->getHandcardNum();
        LogMessage log;
        log.type = "#TriggerSkill";
        log.from = player;
        log.arg = objectName();
        room->sendLog(log);

        if(handcard < 4)
            player->drawCards(4 - handcard);
        else if(handcard > 4)
            room->askForDiscard(player, objectName(), handcard - 4);
        return false;
    }
};

class Duojian: public PhaseChangeSkill{
public:
    Duojian():PhaseChangeSkill("duojian"){
    }

    virtual bool onPhaseChange(ServerPlayer *zilong) const{
        if(zilong->getPhase() == Player::Start){
            Room *room = zilong->getRoom();
            const Card *qinggang = NULL;
            foreach(ServerPlayer *tmp, room->getAllPlayers()){
                if(!tmp->getJudgingArea().isEmpty()){
                    foreach(const Card *card, tmp->getJudgingArea()){
                        if(card->inherits("QinggangSword")){
                            qinggang = card;
                            break;
                        }
                    }
                }
                if(qinggang)
                    break;
                if(tmp != zilong && tmp->getWeapon() && tmp->getWeapon()->inherits("QinggangSword")){
                    qinggang = tmp->getWeapon();
                    break;
                }
            }
            if(qinggang && zilong->askForSkillInvoke(objectName())){
                room->throwCard(zilong->getWeapon());
                room->moveCardTo(qinggang, zilong, Player::Equip);
            }
        }
        return false;
    }
};

class Jieao: public PhaseChangeSkill{
public:
    Jieao():PhaseChangeSkill("jieao"){
        frequency = Compulsory;
    }

    virtual bool onPhaseChange(ServerPlayer *miheng) const{
        if(miheng->getPhase() == Player::Start && miheng->getHp() > miheng->getHandcardNum()){
            LogMessage log;
            log.type = "#TriggerSkill";
            log.from = miheng;
            log.arg = objectName();
            miheng->getRoom()->sendLog(log);
            miheng->drawCards(2);
        }
        return false;
    }
};

YuluCard::YuluCard(){
    target_fixed = true;
    will_throw = false;
}

void YuluCard::use(Room *, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    foreach(int word_id, this->getSubcards()){
        source->addToPile("word", word_id);
    }
}

class Yulu: public ViewAsSkill{
public:
    Yulu():ViewAsSkill("yulu"){
    }

    virtual bool viewFilter(const QList<CardItem *> &, const CardItem *to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.isEmpty())
            return NULL;
        YuluCard *yulu_card = new YuluCard;
        yulu_card->addSubcards(cards);
        return yulu_card;
    }
};

ViewMyWordsCard::ViewMyWordsCard(){
    target_fixed = true;
}

void ViewMyWordsCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    QList<int> words = source->getPile("word");
    if(words.isEmpty())
        return;
    room->fillAG(words, source);
    int card_id = room->askForAG(source, words, true, "viewmywords");
    if(card_id != -1){
        words.removeOne(card_id);
        room->moveCardTo(Sanguosha->getCard(card_id), source, Player::Hand, false);
    }
    source->invoke("clearAG");
    words.clear();
}

class ViewMyWords: public ZeroCardViewAsSkill{
public:
    ViewMyWords():ZeroCardViewAsSkill("numa"){
    }
    virtual const Card *viewAs() const{
        return new ViewMyWordsCard;
    }
};

class Numa: public PhaseChangeSkill{
public:
    Numa():PhaseChangeSkill("numa"){
        view_as_skill = new ViewMyWords;
    }

    virtual bool onPhaseChange(ServerPlayer *miheng) const{
        Room *room = miheng->getRoom();
        if(miheng->getPhase() == Player::Finish &&
           !miheng->getPile("word").isEmpty() &&
           room->askForSkillInvoke(miheng, objectName())){
            Room *room = miheng->getRoom();
            QString c,word;
            foreach(int i, miheng->getPile("word")){
                c = Sanguosha->getCard(i)->getSuitString().left(1);

                LogMessage log;
                log.type = "#Numasingle";
                log.from = miheng;
                log.arg = objectName() + c;
                room->sendLog(log);

                word = word + c;
            }

            LogMessage gitlog;
            gitlog.type = "#Numa_" + word;
            gitlog.from = miheng;
            gitlog.arg = "numa_notice";
            if(word == "hc"){
                room->sendLog(gitlog);
                //womei:recover self
                RecoverStruct womei;
                womei.card = NULL;
                womei.who = miheng;
                room->recover(miheng, womei);
            }
            else if(word == "dc"){
                room->sendLog(gitlog);
                //nimei:throw single player 2 cards
                QList<ServerPlayer *> players;
                foreach(ServerPlayer *tmp, room->getAlivePlayers()){
                    if(tmp->getHandcardNum() < 2){
                        if(tmp->hasSkill("lianying"))
                            players << tmp;
                        else if(tmp->hasSkill("shangshi") && tmp->isWounded())
                            players << tmp;
                        else
                            continue;
                    }
                    else
                        players << tmp;
                }
                room->askForDiscard(room->askForPlayerChosen(miheng, players, objectName()), objectName(), 2);
            }
            else if(word == "cc"){
                room->sendLog(gitlog);
                //meimei:clear single player's all judge_area
                QList<ServerPlayer *> players;
                foreach(ServerPlayer *tmp, room->getAlivePlayers()){
                    if(!tmp->getJudgingArea().isEmpty())
                        players << tmp;
                }
                if(!players.isEmpty()){
                    ServerPlayer *target = room->askForPlayerChosen(miheng, players, objectName());
                    foreach(const Card *c, target->getJudgingArea()){
                        room->throwCard(c->getId());
                    }
                }
            }
            else if(word == "sd"){
                room->sendLog(gitlog);
                //rini:let single player tribute a card and recover 1 hp
                QList<ServerPlayer *> players;
                foreach(ServerPlayer *tmp, room->getOtherPlayers(miheng)){
                    if(tmp->isWounded() && !tmp->isKongcheng())
                        players << tmp;
                }
                if(!players.isEmpty()){
                    ServerPlayer *target = room->askForPlayerChosen(miheng, players, objectName());
                    const Card *card = room->askForCardShow(target, miheng, objectName());
                    miheng->obtainCard(card);
                    RecoverStruct rini;
                    rini.card = card;
                    rini.who = miheng;
                    room->recover(target, rini);
                }
            }
            else if(word == "hs"){
                room->sendLog(gitlog);
                //wori:get skill fanchun
                JudgeStruct judge;
                judge.pattern = QRegExp("(Peach|GodSalvation):(.*):(.*)");
                judge.good = true;
                judge.reason = objectName();
                judge.who = miheng;
                room->judge(judge);
                if(judge.isGood())
                    room->acquireSkill(miheng, "fanchun");
            }
            else if(word == "hsc" || word == "hsd"){
                room->sendLog(gitlog);
                //worimei&worini:recover hp with a girl or a boy
                QList<ServerPlayer *> players;
                foreach(ServerPlayer *tmp, room->getOtherPlayers(miheng)){
                    if(((word == "hsc" && tmp->getGeneral()->isFemale()) ||
                       (word == "hsd" && tmp->getGeneral()->isMale())) && tmp->isWounded())
                        players << tmp;
                }
                if(!players.isEmpty()){
                    ServerPlayer *target = room->askForPlayerChosen(miheng, players, objectName());
                    RecoverStruct worimei;
                    worimei.card = NULL;
                    worimei.who = miheng;
                    room->recover(target, worimei);
                    room->recover(miheng, worimei);
                }
            }
            else if(word == "dsh"){
                room->sendLog(gitlog);
                //niriwo:call slash me! or taking away all his cards
                QList<ServerPlayer *> players;
                foreach(ServerPlayer *tmp, room->getAlivePlayers()){
                    if(tmp->canSlash(miheng))
                        players << tmp;
                }
                ServerPlayer *target = room->askForPlayerChosen(miheng, players, objectName());
                const Card *slash = room->askForCard(target, "slash", objectName());
                if(slash){
                    CardUseStruct niriwo;
                    niriwo.card = slash;
                    niriwo.to << miheng;
                    niriwo.from = target;
                    room->useCard(niriwo);
                }else if(!target->isNude()){
                    QList<const Card *> cards = target->getCards("hej");
                    foreach(const Card *tmp, cards)
                        room->moveCardTo(tmp, miheng, Player::Hand, false);
                }
            }
            else if(word == "shc"){
                room->sendLog(gitlog);
                //riwomei:let single player damage myself and recover himself
                DamageStruct riwmei;
                ServerPlayer *target = room->askForPlayerChosen(miheng, room->getAlivePlayers(), objectName());
                riwmei.from = target;
                riwmei.to = miheng;
                room->damage(riwmei);

                RecoverStruct riwomei;
                riwomei.card = NULL;
                riwomei.who = miheng;
                room->recover(target, riwomei);
            }
            else if(word == "hhh"){
                room->sendLog(gitlog);
                //wowowo:the same to Jushou
                miheng->turnOver();
                miheng->drawCards(3);
            }
            else if(word == "sss"){
                room->sendLog(gitlog);
                //ririri:the same to Fangzhu
                ServerPlayer *target = room->askForPlayerChosen(miheng, room->getAlivePlayers(), objectName());
                target->turnOver();
                target->drawCards(miheng->getMaxHP() - miheng->getHp());
            }
            else if(word == "ddd"){
                room->sendLog(gitlog);
                //ninini:let a player obtain word-card
                ServerPlayer *target = room->askForPlayerChosen(miheng, room->getAlivePlayers(), objectName());
                foreach(int i, miheng->getPile("word"))
                    room->moveCardTo(Sanguosha->getCard(i), target, Player::Hand);
            }
            else if(word == "ccc"){
                room->sendLog(gitlog);
                //meimeimei:clear single player's all equip_area
                ServerPlayer *target = room->askForPlayerChosen(miheng, room->getAlivePlayers(), objectName());
                target->throwAllEquips();
            }
            else if(word == "dcdc"){
                room->sendLog(gitlog);
                //nimeinimei:make a extra turn
                ServerPlayer *target = room->askForPlayerChosen(miheng, room->getAlivePlayers(), objectName());
                foreach(int i, miheng->getPile("word"))
                    room->throwCard(i);
                room->setCurrent(target);
                room->getThread()->trigger(TurnStart, target);
                room->setCurrent(miheng);
            }
            else if(word == "sdc" || word == "hsdc"){
                room->sendLog(gitlog);
                //rinimei:slash
                //worinimei:drank and slash
                if(word == "hsdc")
                    room->setPlayerFlag(miheng, "drank");

                QList<ServerPlayer *> players;
                foreach(ServerPlayer *tmp, room->getAlivePlayers()){
                    if(tmp->hasSkill("kongcheng") && tmp->isKongcheng())
                        continue;
                    players << tmp;
                }
                ServerPlayer *target = room->askForPlayerChosen(miheng, players, objectName());

                int slashtype = Sanguosha->getCard(miheng->getPile("word").first())->getNumber();

                if(!players.isEmpty()){
                    CardUseStruct worinimei;
                    Card *card;
                    if(word == "sdc" && slashtype < 5)
                        card = new ThunderSlash(Card::NoSuit, 0);
                    else if(word == "sdc" && slashtype >9)
                        card = new FireSlash(Card::NoSuit, 0);
                    else
                        card = new Slash(Card::NoSuit, 0);
                    card->setSkillName(objectName());
                    worinimei.card = card;
                    worinimei.from = miheng;
                    worinimei.to << target;
                    room->useCard(worinimei);
                }
            }
            else if(word == "ccsh"){
                room->sendLog(gitlog);
                //nimeiriwo:hp full
                room->setPlayerProperty(miheng, "hp", miheng->getMaxHP());
            }
            else if(word == "dsdc"){
                room->sendLog(gitlog);
                //nimeiriwo:show one player's handcard to other one
                ServerPlayer *source = room->askForPlayerChosen(miheng, room->getAlivePlayers(), objectName());
                ServerPlayer *target = room->askForPlayerChosen(miheng, room->getAlivePlayers(), objectName());

                LogMessage log;
                log.type = "#Info_dsdc";
                log.from = source;
                log.to << target;
                room->sendLog(log);

                room->showAllCards(target, source);
            }
            else if(word == "dshc"){
                room->sendLog(gitlog);
                //niriwomei:kill-self
                /*if(Config.FreeChoose && room->askForChoice(miheng, "numat", "kno+kyes") == "kno"){
                    gitlog.type = "#Numa_tequan";
                    gitlog.from = miheng;
                    room->sendLog(gitlog);
                }
                else{*/
                    room->killPlayer(miheng);
                //}
            }
            else if(word == "hhhhh"){
                room->sendLog(gitlog);
                //niriwomei:goto dengai
                room->transfigure(miheng, "dengai", true);
                foreach(int i, miheng->getPile("word")){
                    room->throwCard(i);
                    miheng->addToPile("field", i);
                }
            }
            else if(word == "dshcc"){
                room->sendLog(gitlog);
                //niriwomeimei:throw other 4 card and make 2 damage to self
                ServerPlayer *target = room->askForPlayerChosen(miheng, room->getAlivePlayers(), objectName());
                for(int i = 4; i > 0; i--)
                    room->throwCard(room->askForCardChosen(miheng, target, "he", objectName()));
                DamageStruct niriwomm;
                niriwomm.from = miheng;
                niriwomm.to = miheng;
                niriwomm.damage = 2;
                room->damage(niriwomm);
            }
            else if(word == "hsdcc" && miheng->getMark("hsdcc") == 0){
                room->sendLog(gitlog);
                //worinimeimei:Limited-Skill, like GreatYeyan
                ServerPlayer *target = room->askForPlayerChosen(miheng, room->getAlivePlayers(), objectName());
                DamageStruct worinimm;
                worinimm.from = miheng;
                worinimm.to = target;
                worinimm.nature = DamageStruct::Thunder;
                room->damage(worinimm);
                worinimm.nature = DamageStruct::Fire;
                room->damage(worinimm);
                worinimm.nature = DamageStruct::Normal;
                room->damage(worinimm);
                room->loseHp(miheng, 2);
                miheng->addMark("hsdcc");
            }
            else if(word == "dcshc" && miheng->getMark("dcshc") == 0){
                room->sendLog(gitlog);
                //worinimeimei:Limited-Skill, like Guixin
                room->loseHp(miheng);
                foreach(ServerPlayer *player, room->getAllPlayers()){
                    if(!player->isKongcheng()){
                        int card_id = room->askForCardChosen(miheng, player, "h", objectName());
                        if(room->getCardPlace(card_id) == Player::Hand)
                            room->moveCardTo(Sanguosha->getCard(card_id), miheng, Player::Hand, false);
                        else
                            room->obtainCard(miheng, card_id);
                    }
                }
                miheng->turnOver();
                miheng->addMark("dcshc");
            }
            else if(word == "ssdcc" && miheng->getMark("ssdcc") == 0){
                room->sendLog(gitlog);
                //ririnimeimei:lightning
                QList<ServerPlayer *> players;
                foreach(ServerPlayer *tmp, room->getAlivePlayers()){
                    foreach(const Card *lightning, tmp->getJudgingArea()){
                        if(lightning->objectName() == "lightning"){
                            players << tmp;
                            break;
                        }
                    }
                }
                if(!players.isEmpty()){
                    ServerPlayer *target = room->askForPlayerChosen(miheng, players, objectName());
                    foreach(const Card *lightning, target->getJudgingArea()){
                        if(lightning->objectName() == "lightning"){
                            room->throwCard(lightning->getId());
                            break;
                        }
                    }
                    DamageStruct damage;
                    damage.from = NULL;
                    damage.to = target;
                    damage.nature = DamageStruct::Thunder;
                    damage.damage = 3;
                    room->damage(damage);

                    miheng->addMark("ssdcc");
                }
            }
            else if(word == "ssscc" && miheng->getMark("ssscc") == 0){
                room->sendLog(gitlog);
                //riririmeimei:let single player acquire benghuai or wumou
                QList<ServerPlayer *> players;
                foreach(ServerPlayer *tmp, room->getOtherPlayers(miheng)){
                    if(tmp->getMaxHP() > miheng->getMaxHP())
                        players << tmp;
                }
                if(!players.isEmpty()){
                    ServerPlayer *target = room->askForPlayerChosen(miheng, players, objectName());
                    QString choice = room->askForChoice(target, objectName(), "bthx+wump");
                    if(choice == "bthx"){
                        room->setPlayerProperty(target, "maxhp", target->getMaxHP() + 2);
                        room->acquireSkill(target, "benghuai");
                    }
                    else{
                        room->setPlayerProperty(target, "maxhp", target->getMaxHP() + 1);
                        room->acquireSkill(target, "wumou");
                    }
                    miheng->addMark("ssscc");
                }
            }
            else if(word.length() == 4){
                gitlog.type = "#Numa_4wd";
                gitlog.from = miheng;
                room->sendLog(gitlog);
                //worinimeimei:Wake-Skill, lost all skills
                /*if(Config.FreeChoose && room->askForChoice(miheng, "numat", "suno+suyes") == "suno"){
                    gitlog.type = "#Numa_tequan";
                    gitlog.from = miheng;
                    room->sendLog(gitlog);
                }
                else{*/
                    QList<const Skill *> skills = miheng->getVisibleSkillList();
                    foreach(const Skill *skill, skills)
                        room->detachSkillFromPlayer(miheng, skill->objectName());
                    room->setPlayerProperty(miheng, "general", "sujiang");
                    room->setPlayerProperty(miheng, "general2", "sujiangf");
                    room->setPlayerProperty(miheng, "maxhp", miheng->getMaxHP() + 2);
                //}
            }
            else if(word.length() == 5 && miheng->getMark("fivewd") == 0){
                gitlog.type = "#Numa_5wd";
                gitlog.from = miheng;
                room->sendLog(gitlog);
                //worinimeimei:Wake-Skill, learn longhun
                /*if(Config.FreeChoose && room->askForChoice(miheng, "numat", "lhno+lhyes") == "lhno"){
                    gitlog.type = "#Numa_tequan";
                    gitlog.from = miheng;
                    room->sendLog(gitlog);
                }
                else{*/
                    room->loseMaxHp(miheng);
                    if(miheng->isAlive()){
                        room->acquireSkill(miheng, "longhun");
                        miheng->addMark("fivewd");
                    }
                //}
            }
            else if(word.length() > 5 && miheng->getMark("othwd") == 0){
                gitlog.type = "#Numa_wds";
                gitlog.from = miheng;
                room->sendLog(gitlog);
                //worinimeimei:Wake-Skill, learn wuyan and buqu
                room->loseMaxHp(miheng, 2);
                if(miheng->isAlive()){
                    room->acquireSkill(miheng, "wuyan");
                    room->acquireSkill(miheng, "buqu");
                    miheng->addMark("othwd");
                }
            }
            else{
                gitlog.type = "#Numa_git";
                gitlog.from = miheng;
                room->sendLog(gitlog);
            }
            foreach(int i, miheng->getPile("word"))
                room->throwCard(i);
        }
        return false;
    }
};

class Fanchun:public MasochismSkill{
public:
    Fanchun():MasochismSkill("fanchun"){
    }
    virtual void onDamaged(ServerPlayer *mh, const DamageStruct &damage) const{
        Room *room = mh->getRoom();
        const Card *card = damage.card;
        if(!room->obtainable(card, mh))
            return;
        QVariant data = QVariant::fromValue(card);
        if(room->askForSkillInvoke(mh, objectName(), data)){
            if(!card->getSubcards().isEmpty())
                foreach(int cd, card->getSubcards())
                    mh->addToPile("word", cd);
            else
                mh->addToPile("word", card->getId());
        }
    }
};

KusoPackage::KusoPackage()
    :Package("kuso")
{
    General *kusoking = new General(this, "kusoking", "god", 4, false);
    kusoking->addSkill(new Huaxu);
    kusoking->addSkill(new HuaxuEffect);
    related_skills.insertMulti("huaxu", "#huaxu_eft");
    kusoking->addSkill(new Liaoting);
    kusoking->addSkill(new MarkAssignSkill("@liaot", 1));
    related_skills.insertMulti("liaoting", "#@liaot-1");

    General *tianyin = new General(this, "tianyin", "god", 3);
    tianyin->addSkill(new Skydao);
    tianyin->addSkill(new Noqing);

    General *tenkei = new General(this, "tenkei", "god", 5, false);
    tenkei->addSkill(new EatDeath);

    General *shenzilong = new General(this, "shenzilong", "god", 1, true, true);
    shenzilong->addSkill(new SuperJuejing);
    shenzilong->addSkill("longhun");
    shenzilong->addSkill(new Duojian);

    General *miheng = new General(this, "miheng", "god", 3);
    miheng->addSkill(new Yulu);
    miheng->addSkill(new Numa);
    miheng->addSkill(new Jieao);

    skills << new Fanchun;
    addMetaObject<LiaotingCard>();
    addMetaObject<YuluCard>();
    addMetaObject<ViewMyWordsCard>();
}

//cards

Sacrifice::Sacrifice(Suit suit, int number)
    :SingleTargetTrick(suit, number, false) {
    setObjectName("sacrifice");
}

bool Sacrifice::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;

    if(!to_select->isWounded())
        return false;

    return true;
}

void Sacrifice::onEffect(const CardEffectStruct &effect) const{
    if(!effect.to->isWounded())
        return;

    Room *room = effect.to->getRoom();
    room->loseHp(effect.from);

    RecoverStruct recover;
    recover.card = this;
    recover.who = effect.from;
    room->recover(effect.to, recover, true);
}

class ClearShirtSkill: public ArmorSkill{
public:
    ClearShirtSkill():ArmorSkill("clear_shirt"){
        events << Predamage;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.to->getNextAlive() == damage.from){
            LogMessage log;
            log.type = "#CSProtect";
            log.from = damage.to;
            log.arg = QString::number(damage.damage);
            log.arg2 = objectName();
            player->getRoom()->sendLog(log);
            return true;
        }
        return false;
    }
};

ClearShirt::ClearShirt(Suit suit, int number) :Armor(suit, number){
    setObjectName("clear_shirt");
    skill = new ClearShirtSkill;
}

void ClearShirt::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    ServerPlayer *master = targets.isEmpty() ?  source->getNextAlive() : targets.first();
    if(master->getArmor())
        room->throwCard(master->getArmor());
    room->moveCardTo(this, master, Player::Equip, true);
}

class KawaiiDressSkill: public ArmorSkill{
public:
    KawaiiDressSkill():ArmorSkill("kawaii_dress"){
        events << Predamaged;
    }

    virtual int getPriority() const{
        return 2;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(player && player == damage.to && player->getHp() == 1){
            LogMessage log;
            log.type = "#KawaiiDressProtect";
            log.from = player;
            log.arg = QString::number(damage.damage);
            if(damage.nature == DamageStruct::Normal)
                log.arg2 = "normal_nature";
            else if(damage.nature == DamageStruct::Fire)
                log.arg2 = "fire_nature";
            else
                log.arg2 = "thunder_nature";
            player->getRoom()->sendLog(log);

            return true;
        }
        return false;
    }
};

KawaiiDress::KawaiiDress(Suit suit, int number) :Armor(suit, number){
    setObjectName("kawaii_dress");
    skill = new KawaiiDressSkill;
}

void KawaiiDress::onInstall(ServerPlayer *player) const{
    EquipCard::onInstall(player);
    if(!player->getGeneral()->isFemale()){
        LogMessage log;
        Room *room = player->getRoom();
        log.from = player;
        log.arg = objectName();
        log.arg2 = "dongzhuo";
        if(player->hasSkill("jiuchi") && player->getHp() <= 2){
            log.type = "#KawaiiAngry";
            room->sendLog(log);
            room->killPlayer(player);
        }
        else{
            log.type = "#KawaiiHurt";
            room->sendLog(log);
            room->loseHp(player);
        }
    }
}

void KawaiiDress::onUninstall(ServerPlayer *player) const{
    if(player->isAlive() && player->getMark("qinggang") == 0){
        player->drawCards(2);
    }
}

class FivelineSkill: public ArmorSkill{
public:
    FivelineSkill():ArmorSkill("fiveline"){
        events << HpChanged;
    }

    virtual int getPriority() const{
        return -1;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &) const{
        int hp = player->getHp();
        if(player->isDead() || hp < 1)
            return false;
        Room *room = player->getRoom();
        QStringList skills;
        skills << "rende" << "jizhi" << "jieyin" << "guose" << "kurou" << "keji";
        QVariantList has_skills = player->tag["fiveline"].toList();
        foreach(QString str, skills){
            if(has_skills.contains(str))
                continue;
            else{
                room->detachSkillFromPlayer(player, str);
                player->loseSkill(str);
            }
        }
        if(hp <= 6)
            room->acquireSkill(player, skills.at(hp - 1));

        return false;
    }
};

Fiveline::Fiveline(Suit suit, int number) :Armor(suit, number){
    setObjectName("fiveline");
    skill = new FivelineSkill;
}

void Fiveline::onInstall(ServerPlayer *player) const{
    EquipCard::onInstall(player);
    QVariantList skills;
    QStringList fiveskill;
    fiveskill << "rende" << "jizhi" << "jieyin" << "guose" << "kurou" << "keji";
    foreach(QString str, fiveskill){
        if(player->hasSkill(str))
            skills << str;
    }
    player->tag["fiveline"] = skills;
    player->getRoom()->setPlayerProperty(player, "hp", player->getHp());
}

void Fiveline::onUninstall(ServerPlayer *player) const{
    if(player->isDead())
        return;
    QStringList skills;
    skills << "rende" << "jizhi" << "jieyin" << "guose" << "kurou" << "keji";
    QVariantList has_skills = player->tag["fiveline"].toList();
    foreach(QString str, skills){
        if(has_skills.contains(str))
            continue;
        else{
            player->getRoom()->detachSkillFromPlayer(player, str);
            player->loseSkill(str);
        }
    }
}

Emigration::Emigration(Suit suit, int number)
    :DelayedTrick(suit, number)
{
    setObjectName("emigration");
    target_fixed = false;

    judge.pattern = QRegExp("(.*):(spade|club):(.*)");
    judge.good = false;
    judge.reason = objectName();
}

bool Emigration::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
    if(!targets.isEmpty())
        return false;

    if(to_select->containsTrick(objectName()))
        return false;

    return true;
}

void Emigration::takeEffect(ServerPlayer *target) const{
    target->skip(Player::Discard);
}

Locust::Locust(Card::Suit suit, int number)
    :Disaster(suit, number)
{
    setObjectName("locust");

    judge.pattern = QRegExp("(.*):(.*):([JQ])");
    judge.good = true;
    judge.reason = objectName();
}

void Locust::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    room->moveCardTo(this, source->getNextAlive(), Player::Judging);
}

void Locust::takeEffect(ServerPlayer *target) const{
    Room *room = target->getRoom();
    if(target->isKongcheng())
        room->loseHp(target);
    else room->askForDiscard(target,objectName(),1);
    onNullified(target);
//    room->moveCardTo(this, target->getNextAlive(), Player::Judging);
}

void Locust::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();

    LogMessage log;
    log.from = effect.to;
    log.type = "#DelayedTrick";
    log.arg = effect.card->objectName();
    room->sendLog(log);

    JudgeStruct judge_struct = judge;
    judge_struct.who = effect.to;
    room->judge(judge_struct);

    if(judge_struct.isBad()){
        takeEffect(effect.to);
    }else {
        if(room->askForChoice(effect.to,objectName(),"move+throw")=="throw")
            room->throwCard(this);
        else
//        room->moveCardTo(this, effect.to->getNextAlive(), Player::Judging);
        onNullified(effect.to);
    }
}

class UFOSkill: public ArmorSkill{
public:
    UFOSkill():ArmorSkill("ufo"){
       events << PhaseChange;
    }

    virtual int getPriority() const{
        return 3;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *player, QVariant &) const{
        if(player->getPhase() != Player::Start)
            return false;
        Room *room = player->getRoom();
        int max = qMax(player->getMaxCards(), player->getMaxHP());
        int x = max - player->getHandcardNum();
        LogMessage log;
        log.from = player;
        log.arg = objectName();
        if(x < 1){
            /*QList<ServerPlayer *> targets;
            targets << player;
            const EquipCard *equipped = qobject_cast<const EquipCard *>(player->getArmor());
            equipped->use(room,player->getNextAlive(),targets);*/
            ServerPlayer *next = player->getNextAlive();
            if(next->getArmor())
                room->throwCard(next->getArmor()->getId());
            room->moveCardTo(player->getArmor(), next, Player::Equip);
            log.type = "#UFOMoving";
            log.to << next;
            room->sendLog(log);
        }else{
            log.type = "#UFODraw";
            room->sendLog(log);
            player->drawCards(x);
        }
        return false;
    }
};

UFO::UFO(Suit suit, int number):Armor(suit, number){
    setObjectName("ufo");
    skill = new UFOSkill;
}

//http://tieba.baidu.com/p/1267132334
/*
Castrate::Castrate(Suit suit, int number)
    :BasicCard(suit, number) {
    setObjectName("castrate");
}

QString Castrate::getSubtype() const{
    return "attack_card";
}

void Castrate::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    room->throwCard(this);

    if(targets.isEmpty())
        room->cardEffect(this, source, source);
    else
        foreach(ServerPlayer *tmp, targets)
            room->cardEffect(this, source, tmp);
}

void Castrate::onEffect(const CardEffectStruct &effect) const{
    //Room *room = effect.to->getRoom();

    effect.to->getGeneral()->setNeuter();
}

bool Castrate::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    return !targets.isEmpty();
}

bool Castrate::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return Self->distanceTo(to_select) < 2;
}
*/

KusoCardPackage::KusoCardPackage()
    :Package("kuso_cards")
{
    QList<Card *> kusos;
    kusos
            << new Sacrifice(Card::Diamond, 7)
            << new ClearShirt(Card::Diamond, 3)
            << new KawaiiDress(Card::Spade, 2)
            << new Fiveline(Card::Heart, 5)
            << new Emigration(Card::Spade, 9)
            << new Emigration(Card::Heart, 13)
            << new Locust(Card::Club, 2)
            << new UFO(Card::Club, 11)
            /*<< new Castrate(Card::Heart, 2)*/;

    foreach(Card *kuso, kusos)
        kuso->setParent(this);

    type = CardPack;
}

ADD_PACKAGE(Kuso)
ADD_PACKAGE(KusoCard)
