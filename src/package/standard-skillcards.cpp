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
    once = true;
}

void ShexianCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    room->drawCards(source, 3);
    room->loseHp(source);
}

CheatCard::CheatCard(){
    target_fixed = true;
    will_throw = false;
}

void CheatCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    if(Config.FreeChoose)
        room->obtainCard(source, subcards.first());
}

//mc
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

HeiyiCard::HeiyiCard(){
    target_fixed = true;
}

void HeiyiCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    room->setPlayerProperty(source, "role", "renegade");
    room->setPlayerProperty(source, "draw3", true);
    LogMessage log;
    log.type = "#Heiyi";
    log.from = source;
    log.arg = "renegade";
    room->sendLog(log);
    room->detachSkillFromPlayer(source, "heiyi");
}
