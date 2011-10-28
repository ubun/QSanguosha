#include "OETanA.h"
#include "standard-skillcards.h"
#include "general.h"
#include "skill.h"
#include "engine.h"
#include "standard.h"
#include "carditem.h"
#include "generaloverview.h"
#include "clientplayer.h"
#include "client.h"
#include "maneuvering.h"
#include "room.h"
#include "ai.h"

//OE3112 圆月-潜水，上浮，MOD
/*潜水：锁定技，【决斗】和红色的【杀】对你无效。
  防御技，毅重的效果比这个强一些，不过毅重会让防具几乎变成废牌，此乃毅重比八阵坑爹的地方：防具都是黑的，卧龙可以拿来当无懈*/
class Qianshui: public TriggerSkill{
public:
    Qianshui():TriggerSkill("qianshui"){
        events << CardEffect << CardEffected;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        CardEffectStruct effect = data.value<CardEffectStruct>();
        if(effect.card->inherits("Duel") || (effect.card->inherits("Slash") && effect.card->isRed())){
            if(effect.to->hasSkill(objectName()) && effect.from){
                if(effect.to->getMark("shangfu"))
                    return false;
                /*上浮的设计成功表明，断肠是可以不变素将的，不过比空城还蛋疼。此法不宜推广*/
                /*待补*/
                //Room *room = player->getRoom();
                /*
                LogMessage log;
                log.type = "#QianshuiNullify";
                log.from = effect.to;
                log.to << effect.from;
                log.arg = effect.card->objectName();

                room->sendLog(log);*/
                return true;
            }
        }
        return false;
    }
};

/*上浮：觉醒技，回合开始阶段，若你装备区的装备达到2个或更多，你须减1点体力上限，失去技能【潜水】，并永久获得技能【MOD】。
  装备意指做MOD用到的Qt等一堆东西。如果有意保存实力，可以不急着觉醒*/
class Shangfu: public PhaseChangeSkill{
public:
    Shangfu():PhaseChangeSkill("shangfu"){
        frequency = Wake;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target)
                && target->getMark("shangfu") == 0
                && target->getPhase() == Player::Start
                && target->getEquips().length() >= 2;
    }

    virtual bool onPhaseChange(ServerPlayer *ibicdlcod) const{
        Room *room = ibicdlcod->getRoom();

        LogMessage log;
        log.type = "#shangfuWake";
        log.from = ibicdlcod;
        room->sendLog(log);

        room->loseMaxHp(ibicdlcod);

        room->acquireSkill(ibicdlcod, "mod");
        room->setPlayerMark(ibicdlcod, "shangfu", 1);
        ibicdlcod->gainMark("shangfu");
        return false;
    }
};

/*MOD：回合开始和结束阶段，你可分别声明并获得场上存活角色的两个技能直至你再次声明为止。
  Er...这个设计的不很成功，想想蛋疼的同将模式吧。。。
  待修复的BUG：技能按钮不会消失，即使你已经声明了其他技能*/
class Mod: public PhaseChangeSkill{
public:
    Mod():PhaseChangeSkill("mod"){
        frequency = Compulsory;
    }

    virtual int getPriority() const{
        return 3;
    }

    virtual bool onPhaseChange(ServerPlayer *ibicdlcod) const{
        if((ibicdlcod->getPhase() != Player::NotActive) && (ibicdlcod->getPhase() != Player::Start))
            return false;

        Room *room = ibicdlcod->getRoom();int i;
        if(!room->askForSkillInvoke(ibicdlcod, objectName()))
            return false;
        {
            ibicdlcod->loseAllSkills();
            room->acquireSkill(ibicdlcod, "mod");
            QList<ServerPlayer *> players = room->getAlivePlayers();
            for(i=0;i<2;i++){
            QStringList lord_skills;//魏武帝遗留，这里只是一般技能

            foreach(ServerPlayer *pp, players){
                QString lord = pp->getGeneralName();
                const General *general = Sanguosha->getGeneral(lord);
                QList<const Skill *> skills = general->findChildren<const Skill *>();
                foreach(const Skill *skill, skills){
                    if(!ibicdlcod->hasSkill(skill->objectName()) && skill->isVisible())
                        lord_skills << skill->objectName();
                }
            }

            if(!lord_skills.isEmpty()){
                QString skill_name = room->askForChoice(ibicdlcod, objectName(), lord_skills.join("+"));

                const Skill *skill = Sanguosha->getSkill(skill_name);
                room->acquireSkill(ibicdlcod, skill);

                if(skill->inherits("GameStartSkill")){
                    const GameStartSkill *game_start_skill = qobject_cast<const GameStartSkill *>(skill);
                    game_start_skill->onGameStart(ibicdlcod);
                }
            }
            }//for
        }
        return false;
    }
};

//OE3103 宇文$-顶座（牛逼），妹控，卖萌$
/*牛逼：出牌阶段前，你可弃X张牌，若如此做，回合结束阶段，你可任意安排场上角色的位置。X为场上存活角色数/2（向上取整）
  好吧我承认修改这个技能是我严重偷懒。。。
  不过原设定成X张不同花色牌会导致9人局或以上严重白板且嘲讽较高，改了也好。
  灵感来源是牛逼神装的屁*/
class Niubi: public PhaseChangeSkill{
public:
    Niubi():PhaseChangeSkill("niubi"){

    }

    virtual int getPriority() const{
        return 3;
    }

    virtual bool onPhaseChange(ServerPlayer *ubun) const{
        switch(ubun->getPhase()){
        case Player::Play: {
                Room *room = ubun->getRoom();
                bool invoked = ubun->askForSkillInvoke(objectName());
                if(invoked){
                    room->askForDiscard(ubun, "niubi", (room->getAlivePlayers().length()+1)/2, false, true);
                    ubun->gainMark("niubi");
                }//if
                return false;
            }//case
        case Player::Finish: {
                if(ubun->getMark("niubi")){
                    Room *room = ubun->getRoom();
                    while(ubun->askForSkillInvoke(objectName())){
                        ServerPlayer *player1 = room->askForPlayerChosen(ubun, room->getOtherPlayers(ubun), objectName());
                        ServerPlayer *player2 = room->askForPlayerChosen(ubun, room->getOtherPlayers(ubun), objectName());
                        room->swapSeat(player1,player2);
                        ubun->loseAllMarks("niubi");
                    }
                }//if
                break;
            }//case
        default:
            break;
        }
        return false;
    }
};

/*妹控：联动技，锁定技，你对天启造成伤害时，防止此伤害，且天启回复与伤害等量的体力。
  不解释。。。我怎么想到了刮骨疗毒。。。M神马的（话说某日没事搜了一下Masochism什么意思，瞎了）*/
class Meikong: public TriggerSkill{
public:
    Meikong():TriggerSkill("meikong"){
        events << Predamage;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.to->getGeneralName() == "tenkei"){
            LogMessage log;
            log.type = "#MeikongSolace";
            log.from = player;
            log.to << damage.to;
            log.arg = QString::number(damage.damage);
            player->getRoom()->sendLog(log);

            RecoverStruct recover;
            recover.who = player;
            recover.recover = damage.damage;
            player->getRoom()->recover(damage.to, recover);
            return true;
        }
        return false;
    }
};

/*卖萌：主公技，坛势力角色在他们各自的出牌阶段可展示并给你两张同花色手牌，若如此做，你回复1点体力。
  DIY区无存在感，随便想的，设计得不好，求更好设计
  注意送出的牌是正面朝上的，并且每回合限一次（技能表述已经隐含，无需另外说明）*/
MaimengCard::MaimengCard(){
}

void MaimengCard::use(Room *room, ServerPlayer *player, const QList<ServerPlayer *> &targets) const{
    ServerPlayer *ubun = targets.first();
    if(ubun->hasSkill("maimeng")){
        ubun->obtainCard(this);
        room->setEmotion(ubun, "good");
        RecoverStruct recover;
        recover.who = player;
        player->getRoom()->recover(ubun, recover);
    }
}

bool MaimengCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select->hasLordSkill("maimeng") && to_select != Self;
}

class MaimengViewAsSkill: public ViewAsSkill{
public:
    MaimengViewAsSkill():ViewAsSkill("maimengv"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("MaimengCard") && player->getKingdom() == "tan";
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(selected.isEmpty())
            return !to_select->isEquipped();
        else if(selected.length() == 1){
            const Card *card = selected.first()->getFilteredCard();
            return !to_select->isEquipped() && to_select->getFilteredCard()->getSuit() == card->getSuit();
        }else
            return false;
    }

    virtual const Card *viewAs(const QList<CardItem*> &cards) const{
        MaimengCard *card = new MaimengCard;
        card->addSubcards(cards);
        card->setSkillName(objectName());
        return card;
    }
};

class Maimeng: public GameStartSkill{
public:
    Maimeng():GameStartSkill("maimeng$"){

    }

    virtual void onGameStart(ServerPlayer *ubun) const{
        Room *room = ubun->getRoom();
        QList<ServerPlayer *> players = room->getAlivePlayers();
        foreach(ServerPlayer *player, players){
            room->attachSkillToPlayer(player, "maimengv");
        }
    }
};

//OE3136 天启-重炮，龟速，萌化
/*重炮：锁定技，你使用的【杀】伤害+1。
  许褚：555.。。。。。*/
class Zhongpao: public TriggerSkill{
public:
    Zhongpao():TriggerSkill("zhongpao"){
        events << Predamage;
        frequency = Compulsory;
    }

    virtual int getPriority() const{
        return -2;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *tenkei, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();

        const Card *reason = damage.card;
        if(reason == NULL)
            return false;

        if(reason->inherits("Slash")){

            LogMessage log;
            log.type = "#zhongpao";
            log.from = tenkei;
            log.to << damage.to;
            log.arg = QString::number(damage.damage);
            log.arg2 = QString::number(damage.damage + 1);
            tenkei->getRoom()->sendLog(log);

            damage.damage ++;
            data = QVariant::fromValue(damage);
        }
        return false;
    }
};

/*龟速：锁定技，你使用【杀】时须弃掉X张牌，否则【杀】不能生效。X为你到目标角色的距离-1.
  本来加了一句“至少为0”，后来发现我烧饼了。。。
  另外本来想弄成弃手牌的，好吧我又偷懒了。。。。*/
class Guisu: public TriggerSkill{
public:
    Guisu():TriggerSkill("guisu"){
        events << CardEffect;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        CardEffectStruct effect = data.value<CardEffectStruct>();

        if(effect.card->inherits("Slash")){
            Room *room = player->getRoom();
            /*
            room->playSkillEffect(objectName());

            LogMessage log;
            log.type = "#Guisu";
            log.from = effect.from;
            log.to << effect.to;
            room->sendLog(log);
            */
            if(player->distanceTo(effect.to) > 1)
                return !room->askForDiscard(player, "guisu", player->distanceTo(effect.to)-1,true,true);
            else return false;
        }
        return false;
    }
};

/*萌化：你可以防止其他角色对你造成的伤害。每回合开始至下一回合开始限一次。*/
class Menghua: public TriggerSkill{
public:
    Menghua():TriggerSkill("menghua"){
        events << Predamaged << PhaseChange;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        if(event == Predamaged){
            if(!player->getMark("menghua")) return false;
            DamageStruct damage = data.value<DamageStruct>();
            if((player->askForSkillInvoke(objectName(),data)) && (damage.from == player || damage.to == player))
            {
            LogMessage log;
            log.type = "#menghuaPrevent";
            log.from = player;
            log.arg = QString::number(damage.damage);
            player->getRoom()->sendLog(log);
                player->loseMark("menghua");
                return true;
            }
            return false;
        }else if(player->hasSkill(objectName()) && event == PhaseChange && player->getPhase() == Player::Start){
            player->loseAllMarks("menghua");
            player->gainMark("menghua");
        }
        return false;
    }
};

//OE3118 猫猫-烧饼，猫爪，爱情
/*烧饼：出牌阶段，你可以自减1点体力，令任一角色增加1点体力上限。每回合限用一次。
  主技能，跟某武将配合或双将极为强力，你懂的。。。
  好吧一开始我烧饼了，居然设成不能对自己使用，其实原来技能表述没这个，估计是写的那会我在OL上测试孙坚呢*/
ShaobingCard::ShaobingCard(){
    once = true;
}

bool ShaobingCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty();
}

void ShaobingCard::use(Room *room, ServerPlayer *OEhyk, const QList<ServerPlayer *> &targets) const{
    room->loseHp(OEhyk);
    room->setPlayerProperty(targets.first(), "maxhp", targets.first()->getMaxHP()+1);
}

class Shaobing: public ZeroCardViewAsSkill{
public:
    Shaobing():ZeroCardViewAsSkill("shaobing"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("ShaobingCard");
    }

    virtual const Card *viewAs() const{
        return new ShaobingCard;
    }
};

/*猫爪：你可以将你的任意一张梅花牌当【杀】使用或打出。
  跟武圣比起来不知弱了多少，因为我记得梅花牌一堆杀。。。。*/
class Maozhua:public OneCardViewAsSkill{
public:
    Maozhua():OneCardViewAsSkill("maozhua"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return Slash::IsAvailable(player);
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern == "slash";
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        const Card *card = to_select->getFilteredCard();

        if(card->getSuit() != Card::Club)
            return false;

        if(card == Self->getWeapon() && card->objectName() == "crossbow")
            return Self->canSlashWithoutCrossbow();
        else
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

/*爱情：异性角色对你造成伤害时，你可进行一次判定，若判定结果为红桃，则防止此伤害。
  发动频率太低，纯一娱乐技能。后两个技能都太弱，第一个还降低防御，以至于不得不4血，真囧*/
class Aiqing: public TriggerSkill{
public:
    Aiqing():TriggerSkill("aiqing"){
        events << Predamaged;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        if(event == Predamaged){
            DamageStruct damage = data.value<DamageStruct>();
            if(damage.from->getGeneral()->isMale() != player->getGeneral()->isMale()){
                if(!player->askForSkillInvoke(objectName()))
                    return false;
                JudgeStruct judge;
                judge.pattern = QRegExp("(.*):(heart):(.*)");
                judge.good = true;
                judge.reason = "aiqing";
                judge.who = player;

                room->judge(judge);

                if(judge.isBad()) return false;

                LogMessage log;
                log.type = "#AiqingProtect";
                log.to << player;
                log.from = damage.from;
                room->sendLog(log);

                return true;
            }
        }
        return false;
    }
};

//OE3119 黄金似夜流月-水魂，幻身
/*水魂：你可将同花色的X张牌按下列规则使用/打出：
  红桃当【无中生有】，方片当【万箭齐发】，黑桃当【南蛮入侵】，梅花当【无懈可击】。X为你当前体力值和1中较大数。
  吐槽：神赵的描述（甚至上溯到诸葛亮的描述）“X为你当前的体力值且至少为1”是非常烧饼的，可以理解为神赵不会濒死吗？？？
  好吧变种神赵的原因是当初漏抄了2，后来审查技能描述的时候不知道是几张就写成X了。。。。
  注意可以水魂装备*/
class Shuihun: public ViewAsSkill{
public:
    Shuihun():ViewAsSkill("shuihun"){

    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "nullification";
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return true;
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        const Card *card = to_select->getFilteredCard();
        int n = qMax(1, Self->getHp());

        if(selected.length() >= n)
            return false;

        if(n > 1 && !selected.isEmpty()){
            Card::Suit suit = selected.first()->getFilteredCard()->getSuit();
            return card->getSuit() == suit;
        }

        switch(ClientInstance->getStatus()){
        case Client::Playing:{
                if(card->getSuit() != Card::Club)
                    return true;
            }
        case Client::Responsing:{
                QString pattern = ClientInstance->getPattern();
                if(pattern == "nullification")
                    return card->getSuit() == Card::Club;
                /*吐槽：所有跟无懈有关的都尼玛是系统技*/
            }
        default:
            break;
        }
        return false;
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        int n = qMax(1, Self->getHp());

        if(cards.length() != n)
            return NULL;

        const Card *card = cards.first()->getFilteredCard();
        Card *new_card = NULL;

        Card::Suit suit = card->getSuit();
        int number = cards.length() > 1 ? 0 : card->getNumber();
        switch(card->getSuit()){
        case Card::Spade:{
                new_card = new SavageAssault(suit, number);
                break;
            }

        case Card::Heart:{
                new_card = new ExNihilo(suit, number);
                break;
            }

        case Card::Club:{
                new_card = new Nullification(suit, number);
                break;
            }

        case Card::Diamond:{
                new_card = new ArcheryAttack(suit, number);
                break;
            }
        default:
            break;
        }

        if(new_card){
            new_card->setSkillName(objectName());
            new_card->addSubcards(cards);
        }

        return new_card;
    }
};

/*幻身：锁定技，属性伤害对你无效，你的手牌上限始终为你的体力上限。
  其实挺牛逼一技能，雷击啊业炎啊闪电啊这些神赵的致命威胁对他无效，另外藤甲是这货神器？
  手牌上限始终为2其实有点烧饼（忒脆了点），不过给他3血我真的有点嫌多。。。*/
class Huanshen: public TriggerSkill{
public:
    Huanshen():TriggerSkill("huanshen"){
        events << Predamaged;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        //Room *room = player->getRoom();
        if(event == Predamaged){
            DamageStruct damage = data.value<DamageStruct>();
            if(damage.nature != DamageStruct::Normal){
                /*
                LogMessage log;
                log.type = "#HuanshenProtect";
                log.to << player;
                log.from = damage.from;
                room->sendLog(log);
                */
                return true;
            }
        }
        return false;
    }
};

//OE3120 白银似夜流月-际遇，兑换
/*际遇：锁定技，你每造成或受到1点伤害，获得1枚【水】标记。
  其实就是开局不拿标记的【狂暴】*/
class Jiyu: public TriggerSkill{
public:
    Jiyu():TriggerSkill("jiyu"){
        events << Damage << Damaged;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        /*
        LogMessage log;
        log.type = event == Damage ? "#JiyuDamage" : "#JiyuDamaged";
        log.from = player;
        log.arg = QString::number(damage.damage);
        player->getRoom()->sendLog(log);
        */
        player->gainMark("@lywater", damage.damage);
        //player->getRoom()->playSkillEffect(objectName());

        return false;
    }
};

/*兑换：出牌阶段，可弃2枚【水】标记，选择下列两项中的一项执行：1.对任意一名其他角色造成1点伤害。2.摸两张牌。
  话说原先是1枚标记，然后目测极强，给了2血。测试的时候才看出来问题：卧槽直接无限伤害啊！！！（流月你淫了。。。）
  其实这货仍然可以靠AOE获得极为强大的爆发，比典韦牛逼多了，2血3血的噩梦（除卖血流及某武将以外）*/
DuihuanCard::DuihuanCard(){

}

bool DuihuanCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;

    return true;
}

void DuihuanCard::use(Room *room, ServerPlayer *OEsilvlryr, const QList<ServerPlayer *> &targets) const{
    OEsilvlryr->loseMark("@lywater", 2);
    if(targets.first() == OEsilvlryr)
        OEsilvlryr->drawCards(2);
    else{
        DamageStruct damage;
        damage.from = OEsilvlryr;
        damage.to = targets.first();
        damage.card = NULL;

        room->damage(damage);
    }
}

class Duihuan: public ZeroCardViewAsSkill{
public:
    Duihuan():ZeroCardViewAsSkill("duihuan"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getMark("@lywater") >= 2;
    }

    virtual const Card *viewAs() const{
        return new DuihuanCard;
    }
};

//OE3123 侍魂-酱油
/*酱油：锁定技，场上每出现一点【杀】造成的伤害，你摸一张牌；场上每出现1点属性伤害，你回复1点体力。
  吐槽在弑魂武将牌里*/
class Jiangyou: public TriggerSkill{
public:
    Jiangyou():TriggerSkill("jiangyou"){
        events << Damaged;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        Room *room = player->getRoom();
        ServerPlayer *OEsrhrsr = room->findPlayerBySkillName(objectName());
        if(damage.nature != DamageStruct::Normal){
            RecoverStruct recover;
            recover.who = damage.from;
            recover.recover = damage.damage;
            room->recover(OEsrhrsr, recover, true);
        }
        if(damage.card && damage.card->inherits("Slash"))
            OEsrhrsr->drawCards(damage.damage);
        return false;
    }
};

//OE3124 卧槽孔明-百合，发呆
/*百合：出牌阶段，你可以弃一张手牌，指定两名女性角色各回复1点体力。
  其实她自己就是女性角色。。。当然仍然有点观赏性，你不一定能有女将队友*/
BaiheCard::BaiheCard(){
}

bool BaiheCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(targets.isEmpty())
        return true;
    if(targets.length() == 1)
        return to_select->getGeneral()->isFemale() && targets.first()->getGeneral()->isFemale();
    return false;
}

void BaiheCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    ServerPlayer *a = targets.at(0);
    ServerPlayer *b = targets.at(1);

    room->throwCard(this);

    RecoverStruct recover;
    recover.card = this;
    recover.who = source;
    room->recover(a, recover, true);
    room->recover(b, recover, true);

}

class Baihe: public ViewAsSkill{
public:
    Baihe():ViewAsSkill("baihe"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("BaiheCard");
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(selected.length() > 1)
            return false;

        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() != 1)
            return NULL;

        BaiheCard *baihe_card = new BaiheCard();
        baihe_card->addSubcards(cards);

        return baihe_card;
    }
};

/*回合结束阶段，你可以弃置所有牌，则直至你的下回合开始，防止你受到的除【南蛮入侵】和【万箭齐发】造成外的一切伤害。
  发呆锁定技，谁用谁知道。对付这货的方法。。。攒AOE？？？*/
class Fadai: public TriggerSkill{
public:
    Fadai():TriggerSkill("fadai"){
        events << PhaseChange << Predamaged;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *OEwbolir = room->findPlayerBySkillName(objectName());
        if(OEwbolir->getPhase() == Player::Finish){
            if(room->askForSkillInvoke(OEwbolir, objectName())){
                OEwbolir->gainMark("fadai");
                //room->playSkillEffect(objectName());
                OEwbolir->throwAllCards();
            }
            return false;
        }
        else if(OEwbolir->getPhase() == Player::Start){
            OEwbolir->loseAllMarks("fadai");
            return false;
        }
        else if(event == Predamaged){
            DamageStruct damage = data.value<DamageStruct>();
            if(OEwbolir->getMark("fadai") && damage.to == OEwbolir
               && !damage.card->inherits("SavageAssault") && !damage.card->inherits("ArcheryAttack"))
                return true;
        }
        return false;
    }
};

OETanAPackage::OETanAPackage()
    :Package("OEtanA")
{
    General *OEibicdlcod, *OEubun, *OEtenkei;
    OEibicdlcod = new General(this, "ibicdlcod", "tan", 4, true);
    OEibicdlcod->addSkill(new Qianshui);
    OEibicdlcod->addSkill(new Shangfu);

    OEubun = new General(this, "ubun$", "tan", 4, true);
    OEubun->addSkill(new Niubi);
    OEubun->addSkill(new Meikong);
    OEubun->addSkill(new Maimeng);

    OEtenkei = new General(this, "tenkei", "tan", 3, false);
    OEtenkei->addSkill(new Zhongpao);
    OEtenkei->addSkill(new Guisu);
    OEtenkei->addSkill(new Menghua);

    General *OEhyk, *OEgoldlryr, *OEsilvlryr, *OEsrhrsr, *OEwbolir;
    OEhyk = new General(this, "hyk3374", "tan", 4, true);
    OEhyk->addSkill(new Shaobing);
    OEhyk->addSkill(new Maozhua);
    OEhyk->addSkill(new Aiqing);

    OEgoldlryr = new General(this, "goldsiyeliuyue", "tan", 2, true);
    OEgoldlryr->addSkill(new Shuihun);
    OEgoldlryr->addSkill(new Huanshen);

    OEsilvlryr = new General(this, "silversiyeliuyue", "tan", 3, false);
    OEsilvlryr->addSkill(new Jiyu);
    OEsilvlryr->addSkill(new Duihuan);

    OEsrhrsr = new General(this, "j_shihunzhishi_l", "tan", 2, true);
    OEsrhrsr->addSkill(new Jiangyou);

    OEwbolir = new General(this, "wocaokongming", "tan", 2, false);
    OEwbolir->addSkill(new Baihe);
    OEwbolir->addSkill(new Fadai);

    addMetaObject<MaimengCard>();
    addMetaObject<ShaobingCard>();
    addMetaObject<DuihuanCard>();
    addMetaObject<BaiheCard>();
    skills << new Mod << new MaimengViewAsSkill;
}

ADD_PACKAGE(OETanA)
