#include "green.h"
#include "skill.h"
#include "client.h"
#include "engine.h"
#include "carditem.h"
#include "room.h"

class Yabian: public TriggerSkill{
public:
    Yabian():TriggerSkill("yabian"){
        events << Damage;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *yanpeng, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();

        if(damage.card && damage.card->inherits("Slash") &&
           damage.from == yanpeng && damage.to != yanpeng){
            Room *room = yanpeng->getRoom();
            if(room->askForSkillInvoke(yanpeng, objectName(), data)){
                if(damage.to->getGeneralName() == "shenzhugeliang"){
                    foreach(int card_id, damage.to->getPile("stars"))
                        yanpeng->addToPile("stars", card_id, false);
                }
                //some special generals
                QList<QString> marks;
                marks
                        << "@chaos" /*luanwu*/
                        << "@nirvana" /*niepan*/
                        << "@flame" /*yeyan*/
                        ;
                foreach(QString mark, marks){
                    if(damage.to->getMark(mark)){
                        yanpeng->gainMark(mark, damage.to->getMark(mark));
                        damage.to->loseMark(mark, damage.to->getMark(mark));
                    }
                }
                //some other marks
                QStringList inscrire;
                inscrire << damage.to->getGeneralName() << damage.to->getKingdom();
                //QString myname = damage.from->getGeneralName();
                //QString mykindom = damage.from->getKingdom();
                if(damage.to->isAlive()){
                    room->transfigure(damage.to, damage.from->getGeneralName(), false, false);
                    room->setPlayerProperty(damage.to, "kingdom", damage.from->getKingdom());
                }
                room->transfigure(damage.from, inscrire.first(), false, false);
                room->setPlayerProperty(damage.from, "kingdom", inscrire.last());
            }
        }
        return false;
    }
};

class Ba0nu: public TriggerSkill{
public:
    Ba0nu():TriggerSkill("ba0nu"){
        events << TurnOvered;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *p, QVariant &data) const{
        Room *rom = p->getRoom();
        LogMessage log;
        log.type = "#TriggerSkill";
        log.from = p;
        log.arg = objectName();
        rom->sendLog(log);

        ServerPlayer *target = rom->askForPlayerChosen(p, rom->getAllPlayers(), objectName());
        if(!target)
            target = p;
        Slash *slash = new Slash(Card::NoSuit, 0);
        slash->setSkillName(objectName());
        CardUseStruct card_use;
        card_use.card = slash;
        card_use.from = p;
        card_use.to << target;
        rom->useCard(card_use);
        return false;
    }
};

class Zhechong: public PhaseChangeSkill{
public:
    Zhechong():PhaseChangeSkill("zhechong"){
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        //Room *room = player->getRoom();
        if(player->getPhase() == Player::Draw ||
           player->getPhase() == Player::Play ||
           player->getPhase() == Player::Discard){
            if(player->askForSkillInvoke(objectName())){
                player->turnOver();
                return true;
            }
        }
        return false;
    }
};

typedef Skill SkillClass;

YuanlvCard::YuanlvCard(){
    once = true;
}

YuanlvStruct::YuanlvStruct()
    :kingdom("wei"), generalA("guojia"), generalB("simayi"), maxhp(5), skills(NULL)
{
}

void YuanlvCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    ServerPlayer *target = targets.first();
    room->loseHp(source);
    QStringList skills;
    foreach(const SkillClass *skill, target->getVisibleSkillList()){
        QString skill_name = skill->objectName();
        if(skill_name == "spear" || skill_name == "axe")
            continue;

        skills << skill_name;
        room->detachSkillFromPlayer(target, skill_name);
    }
    YuanlvStruct yuanlv_data;
    yuanlv_data.kingdom = target->getKingdom();
    yuanlv_data.generalA = target->getGeneralName();
    yuanlv_data.maxhp = target->getMaxHP();
    QString to_transfigure = target->getGeneral()->isMale() ? "sujiang" : "sujiangf";
    //room->setPlayerProperty(target, "general", to_transfigure);
    room->transfigure(target, to_transfigure, false, false);
    room->setPlayerProperty(target, "maxhp", yuanlv_data.maxhp);
    if(target->getGeneral2()){
        yuanlv_data.generalB = target->getGeneral2Name();
        room->setPlayerProperty(target, "general2", to_transfigure);
    }
    room->setPlayerProperty(target, "kingdom", yuanlv_data.kingdom);
    yuanlv_data.skills = skills;
    target->tag["YuanlvStore"] = QVariant::fromValue(yuanlv_data);
    target->addMark("yuanlv_target");
}

class Yuanlv: public ZeroCardViewAsSkill{
public:
    Yuanlv(): ZeroCardViewAsSkill("yuanlv"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("YuanlvCard");
    }

    virtual const Card *viewAs() const{
        return new YuanlvCard;
    }
};

class YuanlvReset:public TriggerSkill{
public:
    YuanlvReset(): TriggerSkill("#yuanlv_clear"){
        events << PhaseChange;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getMark("yuanlv_target") > 0;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        if(player->getPhase() != Player::NotActive)
            return false;

        player->setMark("yuanlv_target", 0);
        Room *room = player->getRoom();
        YuanlvStruct yuanlv_data = player->tag.value("YuanlvStore").value<YuanlvStruct>();

        foreach(QString skill_name, yuanlv_data.skills){
            room->acquireSkill(player, skill_name);
        }
        room->setPlayerProperty(player, "general", yuanlv_data.generalA);
        if(player->getGeneral2()){
            room->setPlayerProperty(player, "general2", yuanlv_data.generalB);
        }
        room->setPlayerProperty(player, "kingdom", yuanlv_data.kingdom);

        player->tag.remove("YuanlvStore");
        return false;
    }
};

class Zhongjian: public TriggerSkill{
public:
    Zhongjian(): TriggerSkill("zhongjian"){
        events << PhaseChange;
        frequency = Frequent;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        ServerPlayer *target;
        Room *room = player->getRoom();
        if(player->getPhase() == Player::Discard && room->askForSkillInvoke(player, objectName(), data)){
            target = room->askForPlayerChosen(player, room->getOtherPlayers(player), objectName());
            target->gainMark("@vy");
        }
        else if(player->getPhase() == Player::Start){
            foreach(target, player->getRoom()->getAllPlayers()){
                if(target->getMark("@vy") > 0){
                    target->loseAllMarks("@vy");
                    target->tag["ZJCount"] = 0;
                }
            }
            player->tag["ZJCount"] = 0;
        }
        return false;
    }
};

class ZhongjianTarget: public TriggerSkill{
public:
    ZhongjianTarget(): TriggerSkill("#zhongjian_target"){
        events << CardUsed << PreSkillInvoke << CardResponsed;
        untriggerable_skill << "spear" << "eight_diagram";
    }

    bool untriggerSkill(QString skill_name) const{
        return untriggerable_skill.contains(skill_name);
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getRoom()->findPlayerBySkillName(objectName()) && target->getMark("@vy") > 0;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *jushou = room->findPlayerBySkillName(objectName());
        if(!jushou)
            return false;
        LogMessage log;
        if(event == CardUsed){
            CardUseStruct use = data.value<CardUseStruct>();
            if(use.card->getSkillName().isEmpty() || untriggerSkill(use.card->getSkillName()) || use.from == jushou)
                return false;
            log.arg2 = use.card->getSkillName();
            log.type = "#Zj_SkillCard";
        }
        else if(event == CardResponsed){
            CardStar star = data.value<CardStar>();
            if(star->getSkillName().isEmpty() || player == jushou)
                return false;
            log.arg2 = star->getSkillName();
            log.type = "#Zj_SkillCard";
        }
        else if(event == PreSkillInvoke){
            SkillInvokeStruct ski = data.value<SkillInvokeStruct>();
            if(!ski.invoked)
                return false;
            log.arg2 = ski.skillname;
            log.type = "#Zj_TriggerSkill";
        }

        log.from = jushou;
        log.to << player;
        log.arg = "zhongjian";
        room->sendLog(log);

        JudgeStruct judge;
        judge.who = jushou;
        judge.good = true;
        judge.reason = "zhongjian";
        room->judge(judge);

        ServerPlayer *target = jushou;
        if(judge.card->isRed())
            target = player;
        target->obtainCard(judge.card);
        target->tag["ZJCount"] = target->tag.value("ZJCount", 0).toInt() + 1;
        if(target->tag.value("ZJCount").toInt() > 3)
            target->turnOver();
        return false;
    }

private:
    QStringList untriggerable_skill;
};

DiezhiCard::DiezhiCard(){
    once = true;
    target_fixed = true;
}
/*
bool DiezhiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && !to_select->isLord();
}
*/
void DiezhiCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    QList<ServerPlayer *> players = room->getOtherPlayers(source);
    players.removeOne(room->getLord());
    if(!players.isEmpty()){
        ServerPlayer *target = room->askForPlayerChosen(source, players, "diezhi");
        //ServerPlayer *target = targets.first();
        const QString myrole = source->getRole();
        source->setRole(target->getRole());
        target->setRole(myrole);
    }
}

class DiezhiViewAsSkill: public ZeroCardViewAsSkill{
public:
    DiezhiViewAsSkill(): ZeroCardViewAsSkill("diezhi"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        if(ServerInfo.GameMode == "02_1v1" || ServerInfo.GameMode == "06_3v3")
            return false;
        if(player->getMark("@drig") == 0)
            return false;
        return !player->hasUsed("DiezhiCard");
    }

    virtual const Card *viewAs() const{
        return new DiezhiCard;
    }
};

class Diezhi: public TriggerSkill{
public:
    Diezhi():TriggerSkill("diezhi"){
        view_as_skill = new DiezhiViewAsSkill;
        events << Death;
    }

    virtual int getPriority() const{
        return -3;
    }

    virtual bool triggerable(const ServerPlayer *player) const{;
        return true;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *kanze = room->findPlayerBySkillName(objectName());
        if(kanze && kanze->isAlive()){
            kanze->loseAllMarks("@drig");
        }
        return false;
    }
};

class Fengjue: public TriggerSkill{
public:
    Fengjue():TriggerSkill("fengjue"){
        events << ToDrawNCards;
    }

    virtual bool triggerable(const ServerPlayer *player) const{;
        return player->getMark("Exception") == 0;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *kanze = room->findPlayerBySkillName(objectName());
        if(!kanze || room->getCurrent() == player)
            return false;
        DrawStruct draw_data = data.value<DrawStruct>();
        if(draw_data.draw > 0 && kanze->askForSkillInvoke(objectName())){
            room->askForDiscard(kanze, objectName(), 1, false, true);
            return true;
        }
        return false;
    }
};

DuizhengCard::DuizhengCard(){
    once = true;
}

bool DuizhengCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(to_select == Self)
        return false;

    return !to_select->isKongcheng();
}

void DuizhengCard::onEffect(const CardEffectStruct &effect) const{
    ServerPlayer *wanglang = effect.from;
    ServerPlayer *git = effect.to;
    Room *room = wanglang->getRoom();

    room->askForPindian(git, git, wanglang, "duizheng");
}

class DuizhengViewAsSkill:public ZeroCardViewAsSkill{
public:
    DuizhengViewAsSkill():ZeroCardViewAsSkill("duizheng"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->isKongcheng() && ! player->hasUsed("DuizhengCard");
    }

    virtual const Card *viewAs() const{
        return new DuizhengCard;
    }
};

class Duizheng: public TriggerSkill{
public:
    Duizheng():TriggerSkill("duizheng"){
        events << Pindian;
        view_as_skill = new DuizhengViewAsSkill;
    }

    virtual int getPriority() const{
        return -1;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        PindianStar pindian = data.value<PindianStar>();
        Room *room = player->getRoom();
        if(pindian->reason == "duizheng"){
            if(pindian->isSuccess()){
                DamageStruct damage;
                damage.from = pindian->from;
                damage.to = pindian->to;
                room->damage(damage);
            }
            else{
                int n = ceil(pindian->from->getHandcardNum());
                if(n == 0)
                    return false;
                QList<const Card *> cards = pindian->from->getCards("h");
                qShuffle(cards);
                cards = cards.mid(0, n);
                DummyCard *dummy_card = new DummyCard;
                foreach(const Card *card, cards)
                    dummy_card->addSubcard(card->getId());
                room->moveCardTo(dummy_card, pindian->to, Player::Hand);
                delete dummy_card;
            }
        }
        return false;
    }
};

class Zhuima: public TriggerSkill{
public:
    Zhuima():TriggerSkill("zhuima"){
        events << Death;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        DamageStar damage = data.value<DamageStar>();
        if(damage && damage->from){
            Room *room = player->getRoom();
            LogMessage log;
            log.type = "#ZhuimaEffect";
            log.from = player;
            log.arg = objectName();
            log.to << damage->from;
            room->sendLog(log);
            damage->from->gainMark("@zhuima");
            room->acquireSkill(damage->from, "#zhuima_effect");
        }
        return false;
    }
};

class ZhuimaEffect:public TriggerSkill{
public:
    ZhuimaEffect():TriggerSkill("#zhuima_effect"){
        events << CardUsed;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        CardUseStruct use = data.value<CardUseStruct>();
        CardStar card = use.card;
        if(card->isNDTrick()){
            Room *room = player->getRoom();
            LogMessage log;
            log.type = "#ZhuimaForbidden";
            log.from = player;
            log.arg = use.card->objectName();
            log.to = use.to;
            room->sendLog(log);
            room->throwCard(card);
            return true;
        }
        return false;
    }
};

class Qilin:public PhaseChangeSkill{
public:
    Qilin():PhaseChangeSkill("qilin"){
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        if(player->getPhase() == Player::Start && player->askForSkillInvoke(objectName())){
            Room *room = player->getRoom();
            ServerPlayer *target = room->askForPlayerChosen(player, room->getOtherPlayers(player), objectName());
            int delta = target->getHandcardNum() - player->getHandcardNum();
            if(delta > 0)
                room->askForDiscard(target, objectName(), delta);
            else if(delta < 0)
                target->drawCards(qAbs(delta));
            if(qAbs(delta) >= 3)
                player->turnOver();
        }
        return false;
    }
};

JinguoCard::JinguoCard(){
    once = true;
}

bool JinguoCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty();
}

void JinguoCard::onEffect(const CardEffectStruct &effect) const{
    effect.to->gainMark("@jin");
}

class JinguoViewAsSkill: public OneCardViewAsSkill{
public:
    JinguoViewAsSkill():OneCardViewAsSkill("jinguo"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "@@jinguo";
    }

    virtual bool viewFilter(const CardItem *) const{
        return true;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        JinguoCard *card = new JinguoCard;
        card->addSubcard(card_item->getCard()->getId());

        return card;
    }
};

class Jinguo: public PhaseChangeSkill{
public:
    Jinguo():PhaseChangeSkill("jinguo"){
        view_as_skill = new JinguoViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target)
                && target->getPhase() == Player::Start;
    }

    virtual bool onPhaseChange(ServerPlayer *lu) const{
        Room *room = lu->getRoom();
        foreach(ServerPlayer *tmp, room->getAllPlayers()){
            if(tmp->getMark("@jin") > 0)
                tmp->loseAllMarks("@jin");
        }

        room->askForUseCard(lu, "@@jinguo", "@jinguo");
        return false;
    }
};

class JinguoEffect: public TriggerSkill{
public:
    JinguoEffect():TriggerSkill("#jinguo_eft"){
        events << SlashProceed;
    }

    virtual int getPriority() const{
        return 2;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        if(effect.to->getMark("@jin") > 0){
            LogMessage log;
            log.type = "#JinguoEffect";
            log.from = effect.from;
            log.to << effect.to;
            log.arg = "jinguo";
            room->sendLog(log);

            const Card *jink = room->askForCard(effect.to, "slash", "jinguo-jink:" + effect.from->objectName());
            room->slashResult(effect, jink);

            return true;
        }
        return false;
    }
};

class Wuqi:public TriggerSkill{
public:
    Wuqi():TriggerSkill("wuqi"){
        events << FinishJudge;
    }

    virtual int getPriority() const{
        return -1;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return !target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *yunlu = room->findPlayerBySkillName(objectName());
        if(!yunlu)
            return false;
        JudgeStar judge = data.value<JudgeStar>();
        if(judge->card->isRed() && yunlu->askForSkillInvoke(objectName(), data)){
            yunlu->drawCards(1);
            yunlu->getRoom()->askForUseCard(yunlu, "slash", "@askforslash");
        }
        return false;
    }
};

QuanyiCard::QuanyiCard(){
    once = true;
}

bool QuanyiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(targets.isEmpty()){
        return false;
    }
    return true;
}

bool QuanyiCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    return targets.length() == 2;
}

void QuanyiCard::use(Room *room, ServerPlayer *s, const QList<ServerPlayer *> &targets) const{
    room->throwCard(this);

    ServerPlayer *from = targets.at(0);
    ServerPlayer *to = targets.at(1);
    from->drawCards(s->getHp());
    room->acquireSkill(from, "#qy_from");
    room->acquireSkill(to, "#qy_to");
}

class QuanyiViewAsSkill: public OneCardViewAsSkill{
public:
    QuanyiViewAsSkill():OneCardViewAsSkill("quanyi"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("QuanyiCard");
    }

    virtual bool viewFilter(const CardItem *) const{
        return true;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        QuanyiCard *card = new QuanyiCard;
        card->addSubcard(card_item->getCard()->getId());
        return card;
    }
};

class Quanyi: public PhaseChangeSkill{
public:
    Quanyi():PhaseChangeSkill("quanyi"){
        view_as_skill = new QuanyiViewAsSkill;
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        Room *room = player->getRoom();
        if(player->getPhase() == Player::Start){
            foreach(ServerPlayer *tmp, room->getAllPlayers()){
                room->detachSkillFromPlayer(tmp, "#qy_from");
                room->detachSkillFromPlayer(tmp, "#qy_to");
            }
        }
        return false;
    }
};

class QuanyiEffect: public ProhibitSkill{
public:
    QuanyiEffect():ProhibitSkill("#qy_from"){
    }

    virtual bool isProhibited(const Player *from, const Player *to, const Card *card) const{
        if(card->inherits("Slash") || card->isNDTrick())
            return to->hasSkill("#qy_to") && from->hasSkill("#qy_from");
        else
            return false;
    }
};

#include <QInputDialog>
class Zhunsuan: public PhaseChangeSkill{
public:
    Zhunsuan():PhaseChangeSkill("zhunsuan"){
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        Room *room = player->getRoom();
        if(player->getPhase() == Player::Start){
            room->setPlayerMark(player, "Zhunsuan", -100);
            return false;
        }
        if(player->getPhase() == Player::Finish && player->askForSkillInvoke(objectName())){
            bool ok;
            int number = QInputDialog::getInteger(NULL, tr("QInputDialog::getInteger()"), tr("Please input the number"), 9, -5, 15, 1, &ok);
            if(ok)
                room->setPlayerMark(player, "Zhunsuan", number);
        }
        return false;
    }
};

class ZhunsuanGet: public TriggerSkill{
public:
    ZhunsuanGet():TriggerSkill("#zhunsuan_get"){
        events << CardLost;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual int getPriority() const{
        return 2;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        CardMoveStar move = data.value<CardMoveStar>();
        if(move->to_place == Player::DiscardedPile){
            const Card *card = Sanguosha->getCard(move->card_id);
            ServerPlayer *chenqun = NULL;
            foreach(ServerPlayer *tmp, room->getAllPlayers()){
                if(tmp->getMark("Zhunsuan") > 0){
                    chenqun = tmp;
                    break;
                }
            }
            if(!chenqun)
                return false;
            if(qAbs(chenqun->getMark("Zhunsuan") - card->getNumber()) < 3)
                chenqun->obtainCard(card);
        }
        return false;
    }
};

GreenPackage::GreenPackage()
    :Package("green")
{
    General *greenyanpeng = new General(this, "greenyanpeng", "shu");
    greenyanpeng->addSkill(new Yabian);

    General *greencaozhang = new General(this, "greencaozhang", "wei");
    greencaozhang->addSkill(new Ba0nu);
    greencaozhang->addSkill(new Zhechong);

    General *greenjushou = new General(this, "greenjushou", "qun", 3);
    greenjushou->addSkill(new Yuanlv);
    greenjushou->addSkill(new YuanlvReset);
    related_skills.insertMulti("yuanlv", "#yuanlv_clear");
    greenjushou->addSkill(new Zhongjian);
    greenjushou->addSkill(new ZhongjianTarget);
    related_skills.insertMulti("zhongjian", "#zhongjian_target");

    General *greenkanze = new General(this, "greenkanze", "wu", 5);
    greenkanze->addSkill(new Diezhi);
    greenkanze->addSkill(new MarkAssignSkill("@drig", 1));
    related_skills.insertMulti("diezhi", "#@drig");
    greenkanze->addSkill(new Fengjue);

    General *greenwanglang = new General(this, "greenwanglang", "wei", 3);
    greenwanglang->addSkill(new Duizheng);
    greenwanglang->addSkill(new Zhuima);
    skills << new ZhuimaEffect;

    General *greenchenwu = new General(this, "greenchenwu", "wu");
    greenchenwu->addSkill(new Qilin);

    General *greenmayunlu = new General(this, "greenmayunlu", "shu", 3, false);
    greenmayunlu->addSkill(new Jinguo);
    greenmayunlu->addSkill(new JinguoEffect);
    related_skills.insertMulti("jinguo", "#jinguo_eft");
    greenmayunlu->addSkill(new Wuqi);

    General *greenchenqun = new General(this, "greenchenqun", "wei", 3);
    greenchenqun->addSkill(new Quanyi);
    skills << new QuanyiEffect << new Skill("#qy_to");
    greenchenqun->addSkill(new Zhunsuan);
    greenchenqun->addSkill(new ZhunsuanGet);
    related_skills.insertMulti("zhunsuan", "#zhunsuan_get");

    addMetaObject<YuanlvCard>();
    addMetaObject<DiezhiCard>();
    addMetaObject<DuizhengCard>();
    addMetaObject<JinguoCard>();
    addMetaObject<QuanyiCard>();
}

ADD_PACKAGE(Green)
