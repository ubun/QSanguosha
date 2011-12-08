#include "standard.h"
#include "general.h"
#include "engine.h"
#include "client.h"
#include "room.h"
#include "carditem.h"

Slash::Slash(Suit suit, int number): BasicCard(suit, number, false){
    setObjectName("slash");
}

bool Slash::IsAvailable(const Player *player){
    return player->canSlashWithoutCrossbow();
}

bool Slash::isAvailable(const Player *player) const{
    return IsAvailable(player);
}

QString Slash::getSubtype() const{
    return "attack_card";
}

void Slash::onEffect(const CardEffectStruct &card_effect) const{
    Room *room = card_effect.from->getRoom();

    SlashEffectStruct effect;
    effect.from = card_effect.from;
    effect.slash = this;
    effect.to = card_effect.to;

    room->slashEffect(effect);
}

bool Slash::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    return !targets.isEmpty();
}

bool Slash::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    int slash_targets = 1;
    if(targets.length() >= slash_targets)
        return false;

    return Self->canSlash(to_select);
}

Jink::Jink(Suit suit, int number):BasicCard(suit, number, false){
    setObjectName("jink");
    target_fixed = true;
}

QString Jink::getSubtype() const{
    return "defense_card";
}

bool Jink::isAvailable(const Player *) const{
    return false;
}

Ingenarg::Ingenarg(Suit suit, int number)
    :Jink(suit, number) {
    setObjectName("ingenarg");
}

Peach::Peach(Suit suit, int number):BasicCard(suit, number, false){
    setObjectName("peach");
}

QString Peach::getSubtype() const{
    return "recover_card";
}

QString Peach::getEffectPath(bool is_male) const{
    return Card::getEffectPath();
}

bool Peach::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select->isWounded();
}

void Peach::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    room->throwCard(this);

    if(targets.isEmpty())
        room->cardEffect(this, source, source);
    else
        foreach(ServerPlayer *tmp, targets)
            room->cardEffect(this, source, tmp);
}

void Peach::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();

    // do animation
    room->broadcastInvoke("animate", QString("peach:%1:%2")
                          .arg(effect.from->objectName())
                          .arg(effect.to->objectName()));

    // recover hp
    RecoverStruct recover;
    recover.card = this;
    recover.who = effect.from;

    room->recover(effect.to, recover);
}

bool Peach::isAvailable(const Player *player) const{
    return player->isWounded();
}

Concludence::Concludence(Suit suit, int number)
    :SingleTargetTrick(suit, number, false) {
    setObjectName("concludence");
}

bool Concludence::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;
    return true;
}

void Concludence::onEffect(const CardEffectStruct &effect) const{
    DamageStruct dmaa;
    dmaa.from = effect.from;
    dmaa.to = effect.to;
    dmaa.card = this;
    effect.from->getRoom()->damage(dmaa);
}

AmazingGrace::AmazingGrace(Suit suit, int number)
    :GlobalEffect(suit, number)
{
    setObjectName("amazing_grace");
}

void AmazingGrace::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    room->throwCard(this);

    QList<ServerPlayer *> players = targets.isEmpty() ? room->getAllPlayers() : targets;
    QList<int> card_ids = room->getNCards(players.length() + 1);
    room->fillAG(card_ids);

    QVariantList ag_list;
    foreach(int card_id, card_ids)
        ag_list << card_id;
    room->setTag("AmazingGrace", ag_list);

    GlobalEffect::use(room, source, players);

    ag_list = room->getTag("AmazingGrace").toList();

    // throw the rest cards
    foreach(QVariant card_id, ag_list){
        room->takeAG(NULL, card_id.toInt());
    }

    room->broadcastInvoke("clearAG");
}

void AmazingGrace::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    QVariantList ag_list = room->getTag("AmazingGrace").toList();
    QList<int> card_ids;
    foreach(QVariant card_id, ag_list)
        card_ids << card_id.toInt();

    int card_id = room->askForAG(effect.to, card_ids, false, objectName());
    card_ids.removeOne(card_id);

    room->takeAG(effect.to, card_id);
    ag_list.removeOne(card_id);

    room->setTag("AmazingGrace", ag_list);
}

ArcheryAttack::ArcheryAttack(Card::Suit suit, int number)
    :AOE(suit, number)
{
    setObjectName("archery_attack");
}

void ArcheryAttack::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();
    const Card *jink = room->askForCard(effect.to, "jink", "archery-attack-jink:" + effect.from->objectName());
    if(jink)
        room->setEmotion(effect.to, "jink");
    else{
        DamageStruct damage;
        damage.card = this;
        damage.damage = 1;
        damage.from = effect.from;
        damage.to = effect.to;
        damage.nature = DamageStruct::Normal;

        room->damage(damage);
    }
}

void SingleTargetTrick::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    room->throwCard(this);

    CardEffectStruct effect;
    effect.card = this;
    effect.from = source;
    if(!targets.isEmpty()){
        foreach(ServerPlayer *tmp, targets){
            effect.to = tmp;
            room->cardEffect(effect);
        }
    }
    else{
        effect.to = source;
        room->cardEffect(effect);
    }
}

Nullification::Nullification(Suit suit, int number)
    :SingleTargetTrick(suit, number, false)
{
    setObjectName("nullification");
}

void Nullification::use(Room *room, ServerPlayer *, const QList<ServerPlayer *> &) const{
    // does nothing, just throw it
    room->throwCard(this);
}

bool Nullification::isAvailable(const Player *) const{
    return false;
}

Ignore::Ignore(Suit suit, int number)
    :SingleTargetTrick(suit, number, false)
{
    setObjectName("ignore");
    will_throw = false;
    target_fixed = true;
}

void Ignore::onEffect(const CardEffectStruct &effect) const{
    if(!effect.to->getPile("ignore").isEmpty())
        effect.from->getRoom()->throwCard(this);
    effect.to->addToPile("ignore", effect.card->getId());
}

bool Ignore::isAvailable(const Player *Self) const{
    return Self->getPile("ignore").isEmpty();
}

ExNihilo::ExNihilo(Suit suit, int number)
    :SingleTargetTrick(suit, number, false)
{
    setObjectName("ex_nihilo");
    target_fixed = true;
}

void ExNihilo::onEffect(const CardEffectStruct &effect) const{
    effect.to->drawCards(2);
}

Duel::Duel(Suit suit, int number)
    :SingleTargetTrick(suit, number, true)
{
    setObjectName("duel");
}

bool Duel::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(to_select->hasSkill("kongcheng") && to_select->isKongcheng())
        return false;

    if(to_select == Self)
        return false;

    return targets.isEmpty();
}

void Duel::onEffect(const CardEffectStruct &effect) const{
    ServerPlayer *first = effect.to;
    ServerPlayer *second = effect.from;
    Room *room = first->getRoom();

    room->setEmotion(first, "duel-a");
    room->setEmotion(second, "duel-b");

    forever{
        if(second->hasSkill("wushuang")){
            room->playSkillEffect("wushuang");
            const Card *slash = room->askForCard(first, "slash", "@wushuang-slash-1:" + second->objectName());
            if(slash == NULL)
                break;

            slash = room->askForCard(first, "slash", "@wushuang-slash-2:" + second->objectName());
            if(slash == NULL)
                break;

        }else{
            const Card *slash = room->askForCard(first, "slash", "duel-slash:" + second->objectName());
            if(slash == NULL)
                break;
        }

        qSwap(first, second);
    }

    DamageStruct damage;
    damage.card = this;
    damage.from = second;
    damage.to = first;

    room->damage(damage);
}

Dismantlement::Dismantlement(Suit suit, int number)
    :SingleTargetTrick(suit, number, false) {
    setObjectName("dismantlement");
}

bool Dismantlement::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;

    if(to_select->isKongcheng())
        return false;

    if(to_select == Self)
        return false;

    return true;
}

void Dismantlement::onEffect(const CardEffectStruct &effect) const{
    if(effect.to->isKongcheng())
        return;

    Room *room = effect.to->getRoom();
    int card_id = effect.to->getRandomHandCardId();
    room->throwCard(card_id);

    LogMessage log;
    log.type = "$Dismantlement";
    log.from = effect.to;
    log.card_str = QString::number(card_id);
    room->sendLog(log);
}

Speak::Speak(Suit suit, int number)
    :SingleTargetTrick(suit, number, false)
{
    setObjectName("speak");
    target_fixed = true;
}

void Speak::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();
    effect.from->setFlags("Speak");
    ServerPlayer *o;
    foreach(ServerPlayer *tmp, room->getAllPlayers()){
        if(tmp->containsTrick("microphone")){
            o = tmp;
            room->setTag("McOwner", QVariant::fromValue(o));
            break;
        }
    }
    room->moveMc(o, effect.to);
}

bool Speak::isAvailable(const Player *Self) const{
    return !Self->containsTrick("microphone");
}

Microphone::Microphone(Suit suit, int number)
    :DelayedTrick(suit, number)
{
    setObjectName("microphone");
    target_fixed = true;
}

bool Microphone::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
    if(!targets.isEmpty())
        return false;

    if(to_select->containsTrick(objectName()))
        return false;

    return true;
}

class HorseSkill: public DistanceSkill{
public:
    HorseSkill():DistanceSkill("horse"){

    }

    virtual int getCorrect(const Player *from, const Player *to) const{
        int correct = 0;
        if(from->getOffensiveHorse())
            correct += from->getOffensiveHorse()->getCorrect();
        if(to->getDefensiveHorse())
            correct += to->getDefensiveHorse()->getCorrect();

        return correct;
    }
};

StandardCardPackage::StandardCardPackage()
    :Package("standard_cards")
{
    type = Package::CardPack;

    QList<Card*> cards;

    cards
            << new Microphone(Card::Club, 4)

            << new Slash(Card::Club, 1)
            << new Duel(Card::Club, 2)
            << new Jink(Card::Diamond, 3)
            << new Ignore(Card::Club, 4)
            << new Slash(Card::Club, 5)
            << new Dismantlement(Card::Club, 6)
            << new Slash(Card::Club, 7)
            << new Jink(Card::Club, 8)
            << new Dismantlement(Card::Club, 9)
            << new AmazingGrace(Card::Club, 10)
            << new Slash(Card::Club, 12)
            << new Ingenarg(Card::Club, 13)

            << new Ignore(Card::Heart, 1)
            << new ExNihilo(Card::Heart, 2)
            << new ExNihilo(Card::Heart, 3)
            << new Slash(Card::Heart, 4)
            << new Concludence(Card::Heart, 5)
            << new Ingenarg(Card::Heart, 6)
            << new Jink(Card::Heart, 7)
            << new Slash(Card::Heart, 8)
            << new Peach(Card::Heart, 9)
            << new Dismantlement(Card::Heart, 10)
            << new Slash(Card::Heart, 11)
            << new Slash(Card::Heart, 12)

            << new ArcheryAttack(Card::Spade, 1)
            << new Slash(Card::Spade, 2)
            << new Slash(Card::Spade, 3)
            << new Duel(Card::Spade, 4)
            << new Peach(Card::Spade, 5)
            << new Jink(Card::Diamond, 6)
            << new Ignore(Card::Spade, 7)
            << new Slash(Card::Spade, 8)
            << new Ingenarg(Card::Spade, 9)
            << new Slash(Card::Spade, 10)
            << new Dismantlement(Card::Spade, 11)
            << new AmazingGrace(Card::Spade, 12)
            << new Nullification(Card::Spade, 13)

            << new Duel(Card::Diamond, 1)
            << new Slash(Card::Diamond, 2)
            << new Peach(Card::Diamond, 3)
            << new Jink(Card::Diamond, 4)
            << new Slash(Card::Diamond, 5)
            << new Ingenarg(Card::Diamond, 6)
            << new Slash(Card::Diamond, 7)
            << new Slash(Card::Diamond, 8)
            << new Dismantlement(Card::Diamond, 9)
            << new ExNihilo(Card::Diamond, 10)
            << new Nullification(Card::Diamond, 12)
            << new ArcheryAttack(Card::Diamond, 13)

            << new Slash(Card::Heart, 1)
            << new Slash(Card::Spade, 2)
            << new Slash(Card::Diamond, 3)
            << new Slash(Card::Club, 4)
            << new Jink(Card::Heart, 5)
            << new Speak(Card::Spade, 6)
            << new Peach(Card::Diamond, 7)
            << new Ignore(Card::Club, 8)
            << new ExNihilo(Card::Heart, 9)
            << new AmazingGrace(Card::Spade, 10)
            << new Speak(Card::Diamond, 11);

    foreach(Card *card, cards)
        card->setParent(this);
}

ADD_PACKAGE(StandardCard)
