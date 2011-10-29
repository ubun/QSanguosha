--maozhua
local maozhua_skill={}
maozhua_skill.name="maozhua"
table.insert(sgs.ai_skills,maozhua_skill)
maozhua_skill.getTurnUseCard=function(self,inclusive)
    local cards = self.player:getCards("he")	
    cards=sgs.QList2Table(cards)
	
	local red_card
	
	self:sortByUseValue(cards,true)
	
	for _,card in ipairs(cards)  do
		if card:getSuit() == sgs.card_Club and not card:inherits("Slash") -- no peach in clubs
			and ((self:getUseValue(card)<sgs.ai_use_value["Slash"]) or inclusive) then
			red_card = card
			break
		end
	end

	if red_card then		
		local suit = red_card:getSuitString()
    	local number = red_card:getNumberString()
		local card_id = red_card:getEffectiveId()
		local card_str = ("slash:maozhua[%s:%s]=%d"):format(suit, number, card_id)
		local slash = sgs.Card_Parse(card_str)
		
		assert(slash)
        
        return slash
	end
end --Ã¨×¦£º·ÂÎäÊ¥

--duihuan
local duihuan_skill={}
duihuan_skill.name = "duihuan"
table.insert(sgs.ai_skills, duihuan_skill)
duihuan_skill.getTurnUseCard=function(self)
	if self.player:getMark("@lywater") < 2 then return end
	return sgs.Card_Parse("@duihuanCard=" .. self:getGeneralName())
end