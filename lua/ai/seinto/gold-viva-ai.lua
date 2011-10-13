-- xiufu
sgs.ai_skill_invoke["xiufu"] = function(self, data)
	local dying = data:toDying()
	return self:isFriend(dying.who)
end
-- 这段有功能问题，修复用牌总是黑桃7的杀
local xiufu_skill={}
xiufu_skill.name = "xiufu"
table.insert(sgs.ai_skills, xiufu_skill)
xiufu_skill.getTurnUseCard=function(self)
    if self.player:hasUsed("XiufuCard") or not self.player:isWounded() then return end
	local cards = self.player:getCards("he")	
    cards = sgs.QList2Table(cards)
	local red_card
--	self:sortByUseValue(cards, true)
	for _, card in ipairs(cards) do
		if card:getSuitString() == "heart" then
			red_card = card
			break
		end
	end
	if red_card then
		card = sgs.Card_Parse("@XiufuCard=." .. red_card:getEffectiveId()) 
		return card
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

--xingmie(未生效)
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
	if self:isFriend(target) or self.player == target then return end

	return sgs.Card_Parse("@XingmieCard=.")
end

sgs.ai_skill_use_func["XingmieCard"]=function(card,use,self)
	local cards = self.player:getHandcards()
	cards = sgs.QList2Table(cards)
	local need_cards = {}
	for _, card in ipairs(cards) do
		table.insert(need_cards, card)
		if #need_cards == 3 then break end
	end
	if #need_cards ~= 3 then return end
	use.card = sgs.Card_Parse("@XingmieCard=" .. table.concat(need_cards, "+"))
end

sgs.ai_skill_playerchosen["xingmie"] = function(self, targets)
	for _, player in sgs.qlist(targets) do
		if self:isEnemy(player) and 
			not (player:getArmor() and player:getArmor():objectName() == "silver_lion") then
			return player
		end
	end
end

-- haojiao
sgs.ai_skill_invoke["haojiao"] = function(self, data)
	return true
end
sgs.ai_skill_use["@@haojiao"] = function(self, prompt)
	if self.player:getHp() > 1 and self.player:getHandcardNum() < self.player:getHp()-1 
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
