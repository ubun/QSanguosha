#include "standard.h"
#include "standard-skillcards.h"
#include "room.h"
#include "clientplayer.h"
#include "engine.h"
#include "client.h"
#include "settings.h"

KaituoCard::KaituoCard(){
    target_fixed = true;
    once = true;
}

void KaituoCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    room->throwCard(this);
    RecoverStruct revocer;
    revocer.card = this;
    revocer.who = source;
    room->recover(source, revocer);
}

ShexianCard::ShexianCard(){
    target_fixed = true;
}

void ShexianCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    room->drawCards(source, 3);
    room->loseHp(source);
}

JieyinCard::JieyinCard(){
    once = true;
    mute = true;
}

bool JieyinCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;

    return to_select->getGeneral()->isMale() && to_select->isWounded();
}

void JieyinCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();

    RecoverStruct recover;
    recover.card = this;
    recover.who = effect.from;

    room->recover(effect.from, recover, true);
    room->recover(effect.to, recover, true);

    int index = -1;
    if(effect.from->getGeneral()->isMale()){
        if(effect.from == effect.to)
            index = 5;
        else if(effect.from->getHp() >= effect.to->getHp())
            index = 3;
        else
            index = 4;
    }else{
        index = 1 + qrand() % 2;
    }

    room->playSkillEffect("jieyin", index);
}

LijianCard::LijianCard(){
    once = true;
}

bool LijianCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!Self->hasArmorEffect("yaiba") && !to_select->getGeneral()->isMale())
        return false;

    if(targets.isEmpty() && to_select->hasSkill("kongcheng") && to_select->isKongcheng()){
        return false;
    }

    return true;
}

bool LijianCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    return targets.length() == 2;
}

void LijianCard::use(Room *room, ServerPlayer *, const QList<ServerPlayer *> &targets) const{
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

QingnangCard::QingnangCard(){
    once = true;
}

bool QingnangCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select->isWounded();
}

bool QingnangCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    return targets.value(0, Self)->isWounded();
}

void QingnangCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    room->throwCard(this);

    ServerPlayer *target = targets.value(0, source);

    CardEffectStruct effect;
    effect.card = this;
    effect.from = source;
    effect.to = target;

    room->cardEffect(effect);
}

void QingnangCard::onEffect(const CardEffectStruct &effect) const{
    RecoverStruct recover;
    recover.card = this;
    recover.who = effect.from;
    effect.to->getRoom()->recover(effect.to, recover);
}

GuicaiCard::GuicaiCard(){
    target_fixed = true;
    will_throw = false;
}

void GuicaiCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{

}

LiuliCard::LiuliCard()
{
}


bool LiuliCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;

    if(!Self->hasArmorEffect("underwear") && to_select->hasFlag("slash_source"))
        return false;

    if(!Self->hasArmorEffect("underwear") && !Self->canSlash(to_select))
        return false;

    int card_id = subcards.first();
    if(!Self->hasArmorEffect("underwear") && Self->getWeapon() && Self->getWeapon()->getId() == card_id)
        return Self->distanceTo(to_select) <= 1;
    else
        return true;
}

void LiuliCard::onEffect(const CardEffectStruct &effect) const{
    effect.to->getRoom()->setPlayerFlag(effect.to, "liuli_target");
}

JijiangCard::JijiangCard(){

}

bool JijiangCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && Self->canSlash(to_select);
}

void JijiangCard::use(Room *room, ServerPlayer *liubei, const QList<ServerPlayer *> &targets) const{
    QList<ServerPlayer *> lieges = room->getLieges("shu", liubei);
    const Card *slash = NULL;

    QVariant tohelp = QVariant::fromValue((PlayerStar)liubei);
    foreach(ServerPlayer *liege, lieges){
        slash = room->askForCard(liege, "slash", "@jijiang-slash:" + liubei->objectName(), tohelp);
        if(slash){
            CardUseStruct card_use;
            card_use.card = slash;
            card_use.from = liubei;
            card_use.to << targets.first();

            room->useCard(card_use);
            return;
        }
    }
}

HuanzhuangCard::HuanzhuangCard(){
    target_fixed = true;
}

void HuanzhuangCard::onUse(Room *room, const CardUseStruct &card_use) const{
    ServerPlayer *diaochan = card_use.from;

    if(diaochan->getGeneralName() == "diaochan"){
        room->transfigure(diaochan, "sp_diaochan", false, false);
    }else if(diaochan->getGeneralName() == "sp_diaochan"){
        room->transfigure(diaochan, "diaochan", false, false);
    }
}

CheatCard::CheatCard(){
    target_fixed = true;
    will_throw = false;
}

void CheatCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    if(Config.FreeChoose)
        room->obtainCard(source, subcards.first());
}

Mp1Card::Mp1Card(){
}

bool Mp1Card::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;
    return true;
}

void Mp1Card::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    room->moveMicrophone(effect.from);
    Slash *slash = new Slash(Card::NoSuit, 0);
    slash->setSkillName("mp1");
    slash->setCancelable(false);
    CardUseStruct use;
    use.card = slash;
    use.from = effect.from;
    use.to << effect.to;
    room->useCard(use, false);
}

Mp2Card::Mp2Card(){
    target_fixed = true;
}

void Mp2Card::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    room->moveMicrophone(source);
    const Card *card = Sanguosha->getCard(this->getSubcards().first());
    Slash *slash = new Slash(card->getSuit(), card->getNumber());
    slash->addSubcard(card);
    slash->setSkillName("mp2");
    CardUseStruct use;
    use.card = slash;
    use.from = source;
    use.to = room->getOtherPlayers(source);
    room->useCard(use);
}

Mp3Card::Mp3Card(){
}

bool Mp3Card::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;
    return true;
}

void Mp3Card::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    room->moveMicrophone(effect.from);
    room->setPlayerFlag(effect.from, "mp3");
    room->setPlayerFlag(effect.to, "mp3");
}

Mp4Card::Mp4Card(){
    target_fixed = true;
}

void Mp4Card::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    room->moveMicrophone(source);
    const Card *card = Sanguosha->getCard(this->getSubcards().first());
    RecoverStruct reo;
    reo.card = card;
    reo.who = source;
    room->recover(source, reo);
}
