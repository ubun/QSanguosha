-- PurplePackage's AI by Ubun.

-- zhuosu
sgs.ai_skill_invoke["zhuosu"] = true
sgs.ai_skill_playerchosen["zhuosu"] = function(self, targets)
	for _, player in sgs.qlist(targets) do
		if self.player:faceUp() and self:isFriend(player) then
			return player
		end
		if not self.player:faceUp() and self:isEnemy(player) and player:faceUp() then
			return player
		end
	end
	return self.player
end

-- difu
sgs.ai_skill_invoke["difu"] = function(self, data)
	if self:getCardsNum("Analeptic") > 0 then return false end
	local peach = 0
	for _, friend in ipairs(self.friends) do
		peach = peach + self:getCardsNum("Peach", friend)
	end
	return peach < 1
end
sgs.ai_skill_playerchosen["difu"] = function(self, targets)
	self:sort(self.friends_noself, "handcard")
	return self.friends_noself[1]
end

-- wuxian&xianming
sgs.ai_skill_invoke["wuxian"] = function(self, data)
	return self.player:getHandcardNum() < 5
end
sgs.ai_skill_invoke["xianming"] = function(self, data)
	local cardy = data:toString()
	return cardy == "dismantlement"
end

-- xiannei
sgs.ai_skill_invoke["xiannei"] = function(self, data)
	local target = data:toPlayer()
	self.xianneitarget = data
	return self.player:getHandcardNum() > 1 and self:isFriend(target)
end
sgs.ai_skill_choice["xiannei"] = function(self, choices)
	local target = self.xianneitarget:toPlayer()
	if target:isWounded() then
		return "hp"
	else
		return "draw"
	end
end

-- shangjue
sgs.ai_skill_invoke["shangjue"] = true
sgs.ai_skill_choice["shangjue"] = function(self, choices)
	if self.player:isKongcheng() then
		return "draw"
	else
		return "damage"
	end
end
sgs.ai_skill_playerchosen["shangjue"] = function(self, targets)
	self:sort(self.enemies, "hp")
	return self.enemies[1]
end

-- poxie
sgs.ai_skill_invoke["poxie"] = function(self, data)
	local r = math.random(1, 3)
	return r == 2
end
sgs.ai_skill_playerchosen["poxie"] = sgs.ai_skill_playerchosen["shangjue"]
sgs.ai_skill_invoke["xiangfeng"] = true

-- xiayi
sgs.ai_skill_cardask["@xiayi"] = function(self, data, pattern, target)
	local target = data:toPlayer()
	if self:isFriend(target) then return "." end
	local cards = self.player:getHandcards()
	for _, card in sgs.qlist(cards) do
		if card:inherits("Slash") then
			return card:getEffectiveId()
		end
	end
	return "."
end

-- chongguan
sgs.ai_skill_invoke["chongguan"] = true
sgs.ai_skill_playerchosen["chongguan"] = function(self, targets)
	if self.player:hasFlag("fre") then
		for _, friend in ipairs(self.friends) do
			if friend:getGeneral():isMale() then
				return friend
			end
		end
		return self.friends[1]
	end
	for _, player in sgs.qlist(targets) do
		if self:isEnemy(player) then
			return player
		end
	end
	return self.enemies[1]
end
sgs.ai_skill_choice["chongguan"] = "draw"

-- xianhou
sgs.ai_skill_invoke["xianhou"] = function(self, data)
	local num = self.player:getHandcardNum()
	return num > 2
end
sgs.ai_skill_playerchosen["xianhou"] = sgs.ai_skill_playerchosen["difu"]
sgs.ai_skill_choice["xianhou"] = "draw"

-- yaofa
sgs.ai_skill_use["@@yaofa"] = function(self, prompt)
	self:sort(self.enemies, "hp")
	local target = self.enemies[1]
	if target then return "@YaofaCard=.".."->"..target:objectName() end
	return "."
end

-- woxuan
sgs.ai_skill_choice["woxuan"] = function(self, choices)
	if choices == "woturn+woexcha+wocancel" then
		return "woexcha"
	else
		return "woturn"
	end
end

-- cizhen
sgs.ai_skill_invoke["cizhen"] = function(self, data)
	local use = data:toCardUse()
	return self:isEnemy(use.from)
end

