module("extensions.YKStdGeneral", package.seeall)

extension = sgs.Package("YKStdGeneral")

--101 liubei unf
Rende_Card = sgs.CreateSkillCard{
	name = "Rende_card",
	will_throw = false,
	
	on_use = function(self, room, source, targets)
		local target = nil
		if(targets:isEmpty()) then--[[
			table.foreach(room:getAlivePlayers(),
				function(player, v)
					if(player != source) then
						target = player
					break
					end
				end
			)]]
		else
			target = targets:first()

			room:moveCardTo(this, target, sgs.Player_Hand, false)

			local old_value = source:getMark("rende");
			local new_value = old_value + subcards:length();
			room:setPlayerMark(source, "rende", new_value);

			if(old_value < 2 and new_value >= 2) then
				local recover = sgs.RecoverStruct()
				recover.card = this;
				recover.who = source;
				room:recover(source, recover);
			end
		end
	end,
}

--102 Guanyu
Wusheng = sgs.CreateViewAsSkill{
	name = "wusheng",
	
	enabled_at_play=function(player)
		return sgs.Slash_IsAvailable(player)
	end
}
class Wusheng:public OneCardViewAsSkill{
public:
    Wusheng():OneCardViewAsSkill("wusheng"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return Slash::IsAvailable(player);
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern == "slash";
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        const Card *card = to_select->getFilteredCard();

        if(!card->isRed())
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

sgs.LoadTranslationTable{
	
}
