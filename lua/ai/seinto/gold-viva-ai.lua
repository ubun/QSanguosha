-- xiufu
sgs.ai_skill_invoke["xiufu"] = function(self, data)
	local dying = data:toDying()
	local cards = self.player:getHandcards()
	cards = sgs.QList2Table(cards)
	for _, card in ipairs(cards) do
		if card:getSuitString() == "heart" then
			return self:isFriend(dying.who)
		end
	end
	return false
end

local xiufu_skill={}
xiufu_skill.name = "xiufu"
table.insert(sgs.ai_skills, xiufu_skill)
xiufu_skill.getTurnUseCard=function(self)
    if self.player:hasUsed("XiufuCard") or not self.player:isWounded() then return end
	local cards = self.player:getCards("he")	
    cards = sgs.QList2Table(cards)
	local red_card
	self:sortByUseValue(cards, true)
	for _, card in ipairs(cards) do
		if card:getSuitString() == "heart" then
			red_card = card
			break
		end
	end
	if red_card then
		return sgs.Card_Parse("@XiufuCard=" .. red_card:getEffectiveId()) 
	end
	return nil
end

sgs.ai_skill_use_func["XiufuCard"]=function(card,use,self)
	use.card = card
end

-- jingqiang
sgs.ai_skill_invoke["jingqiang"] = function(self, data)
	return true
end
sgs.ai_skill_playerchosen["jingqiang"] = function(self, targets)
	local target = self.player
	for _, player in sgs.qlist(targets) do
		if self:isFriend(player) then
			if player:getHp() < target:getHp() and player:getMark("@jq") == 0 then
				target = player
			end
		end
	end
	return target
end

--xingmie
local xingmie_skill={}
xingmie_skill.name = "xingmie"
table.insert(sgs.ai_skills, xingmie_skill)
xingmie_skill.getTurnUseCard=function(self)
    if self.player:hasUsed("XingmieCard") or self.player:getMark("@xm") < 1 then return end

	local cardnummin = self.player:getHandcardNum() - 3
	local players=sgs.QList2Table(self.room:getAllPlayers())
	local target = self.player
	for _, player in ipairs(players) do
		if player:getHandcardNum() < cardnummin then
			target = player
			cardnummin = target:getHandcardNum()
		end
	end
	if cardnummin <= 2 then return end
	local targets = {}
	for _, player in ipairs(players) do
		if player:getHandcardNum() == cardnummin then
			table.insert(targets, player)
		end
	end
	for _, target in ipairs(targets) do
		if not self:isFriend(target) and not self.player == target then
			return sgs.Card_Parse("@XingmieCard=.")
		end
	end
end

sgs.ai_skill_use_func["XingmieCard"]=function(card,use,self)
	local cards = self.player:getHandcards()
	cards = sgs.QList2Table(cards)
	local need_cards = {}
	for _, card in ipairs(cards) do
		table.insert(need_cards, card:getEffectiveId())
		if #need_cards == 3 then break end
	end
	if #need_cards ~= 3 then return end
	use.card = sgs.Card_Parse("@XingmieCard=" .. table.concat(need_cards, "+"))
end

sgs.ai_skill_playerchosen["xingmie"] = function(self, targets)
	for _, player in sgs.qlist(targets) do
		if self:isEnemy(player) and 
			not (player:getArmor() and player:getArmor():objectName() == "silver_lion") and
			player:getHandcardNum() > 1	then
			return player
		end
	end
end

-- haojiao
sgs.ai_skill_invoke["haojiao"] = function(self, data)
	return true
end
sgs.ai_skill_use["@@haojiao"] = function(self, prompt)
	if self.player:getHp() > 1 and self.player:getHandcardNum() < self.player:getHp() - 1 
		and self.player:faceUp() then return "." end
	local first, second
	self:sort(self.enemies)

	for _, enemy in ipairs(self.enemies) do
		if self.player:canSlash(enemy, not no_distance) then
			if not first then 
				first = enemy
			elseif enemy:getHandcardNum() < 2 then 
				first = enemy
			end
		end
		if first and self.player:canSlash(enemy, not no_distance) and enemy ~= first then
			if not second then 
				second = enemy
			elseif enemy:getHandcardNum() < 2 then 
				second = enemy
			end
		end
	end

	if first and second then
		self:log(first:getGeneralName() .. "+" .. second:getGeneralName())
		local first_o = first:objectName()
		local second_o = second:objectName()
		return ("@HaojiaoCard=.->%s+%s"):format(first_o, second_o)
	elseif first and not second then
		self:log(first:getGeneralName())
		return ("@HaojiaoCard=.->%s"):format(first:objectName())
	else
		return "."
	end
end

-- huanlong
local huanlong_skill={}
huanlong_skill.name = "huanlong"
table.insert(sgs.ai_skills, huanlong_skill)
huanlong_skill.getTurnUseCard=function(self)
	if self.player:getHandcardNum() > 1 then
		if self.player:hasUsed("HuanlongCard") then return end
		local max_card = self:getMaxCard()
		if max_card and max_card:getNumber() > 11 then
			return sgs.Card_Parse("@HuanlongCard=" .. max_card:getId())
		end
	end
end

sgs.ai_skill_use_func["HuanlongCard"] = function(card, use, self)
	self:sort(self.enemies, "handcard")
	for _, enemy in ipairs(self.enemies) do
		if enemy:getHandcardNum() > 1 and use.to then
			use.to:append(enemy)
			break
		end
	end
	use.card = card
	return
end

-- shiqi
local shiqi_skill={}
shiqi_skill.name = "shiqi"
table.insert(sgs.ai_skills, shiqi_skill)
shiqi_skill.getTurnUseCard=function(self)
	if not self.player:hasUsed("ShiqiCard") and self.player:getHandcardNum() > 1 then
		local givecard = {}
		local index = 0
		local cards = self.player:getHandcards()
		cards = sgs.QList2Table(cards)
		for _, fcard in ipairs(cards) do 
			if not fcard:inherits("Shit") then
				table.insert(givecard, fcard:getId())
				index = index + 1
			end
			if index == 2 then break end
		end
		if index < 2 then return end
		return sgs.Card_Parse("@ShiqiCard=" .. table.concat(givecard, "+"))
	end
end

sgs.ai_skill_use_func["ShiqiCard"] = function(card, use, self)
	self:sort(self.friends_noself, "handcard")
	if use.to then use.to:append(self.friends_noself[1]) end
	use.card = card
	return
end

sgs.ai_skill_choice["shiqi"] = function(self, choices)
	return "draw"
end

-- diansu
sgs.ai_skill_use["@@diansu"] = function(self, prompt)
	local others=self.room:getOtherPlayers(self.player)
	others=sgs.QList2Table(others)
	for _, enemy in ipairs(self.enemies) do
		if self.player:canSlash(enemy,true) then
            local cards = self.player:getCards("h")
			for _, card in sgs.qlist(cards) do
				if card:isBlack() and card:inherits("BasicCard") then
					local suit = card:getSuitString()
					local number = card:getNumberString()
					local card_id = card:getEffectiveId()
					return ("thunder_slash:diansu[%s:%s]=%d->%s"):format(suit, number, card_id, enemy:objectName())
				end
			end
		end
	end
	return "."
end

-- longba
sgs.ai_skill_invoke["longba"] = function(self, data)
	return true
end

-- hongzhen
sgs.ai_skill_choice["hongzhen15"] = function(self, choices)
	local players = sgs.QList2Table(self.room:getOtherPlayers(self.player))
	for _, player in sgs.qlist(players) do
		if self:isEnemy(player) and player:getHp() <= 3 then
			return "shoot"
		end
	end
	return "later"
end
sgs.ai_skill_playerchosen["hongzhen15"] = function(self, targets)
	for _, player in sgs.qlist(targets) do
		if self:isEnemy(player) and player:getHp() <= 3 then
			return player
		end
	end
end

-- shanguang
local shanguang_skill={}
shanguang_skill.name = "shanguang"
table.insert(sgs.ai_skills, shanguang_skill)
shanguang_skill.getTurnUseCard=function(self)
	if self:slashIsAvailable(self.player) then
		local cards = self.player:getHandcards()
		cards = sgs.QList2Table(cards)
		for _, hcard in ipairs(cards) do 
			if hcard:inherits("Jink") or hcard:inherits("Slash") or hcard:inherits("TrickCard") then
				return sgs.Card_Parse("@ShanguangCard=" .. hcard:getId())
			end
		end
	end
end

sgs.ai_skill_use_func["ShanguangCard"] = function(card, use, self)
	self:sort(self.enemies, "handcard")
	for _, enemy in ipairs(self.enemies) do
		if enemy and use.to then
			use.to:append(enemy)
			break
		end
	end
	use.card = card
	return
end

-- shengjian
sgs.ai_skill_use["@@shengjian"] = function(self, prompt)
	self:sort(self.enemies, "handcard")
	local x = self.player:getMaxHP() - self.player:getHp()
	for _, enemy in ipairs(self.enemies) do
		if self.player:distanceTo(enemy) <= x then
			return ("@ShengjianCard=.->%s"):format(enemy:objectName())
		end
	end
	local card_id = self:getCardId("Slash")
	if card_id then
		for _, enemy in ipairs(self.enemies) do
			if self.player:distanceTo(enemy) > x then
				return ("@ShengjianCard=%d->%s"):format(card_id, enemy:objectName())
			end
		end
	end
	return "."
end

-- binghuan
sgs.ai_skill_invoke["binghuan"] = function(self, data)
	return true
end

-- meigui
sgs.ai_skill_invoke["meigui"] = function(self, data)
	local effect = data:toSlashEffect()
	if effect.slash:isRed() then
		return true
	elseif effect.slash:isBlack() then
		return true
	end
end
