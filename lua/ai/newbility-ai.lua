-- Newbility's AI by Ubun.

-- niubi-armor
function SmartAI:useNiubi(card, use)
	use.card = card
	return
end

-- 香蕉
sgs.ai_skill_playerchosen["banana"] = function(self, targets)
	targets = sgs.QList2Table(targets)
	self:sort(targets, "hp")
	for _, enemy in ipairs(targets) do
		if self:isEnemy(enemy) then
			return enemy
		end
	end
end

-- 励志照片
sgs.ai_skill_invoke["madamfeng"] = function(self, data)
	self:sort(self.friends, "hp")
	return self.friends[1]:isWounded()
end
sgs.ai_skill_playerchosen["madamfeng"] = function(self, targets)
	targets = sgs.QList2Table(targets)
	self:sort(targets, "hp")
	for _, friend in ipairs(targets) do
		if self:isFriend(friend) then
			return friend
		end
	end
end

-- 垃圾桶
local dustbin_skill = {}
dustbin_skill.name = "dustbin"
table.insert(sgs.ai_skills, dustbin_skill)
dustbin_skill.getTurnUseCard = function(self)
	if self.player:hasFlag("dust") then return end
	if self.player:hasSkill("lianying") and self.player:getHandcardNum() == 1 then
		local cards = self.player:getHandcards()
		lastcard = sgs.QList2Table(cards)[1]
		return sgs.Card_Parse("$" .. lastcard:getId())
	end
end

-- 禽兽图
local animals_skill={}
animals_skill.name = "animals"
table.insert(sgs.ai_skills, animals_skill)
animals_skill.getTurnUseCard = function(self)
	local cards = self.player:getCards("h")
	cards=sgs.QList2Table(cards)
	local card
	for _,acard in ipairs(cards) do
		if acard:inherits("TrickCard") and acard:getSuit() == sgs.Card_Spade then
			card = acard
			break
		end
	end
	if not card then return end
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	local card_str = ("duel:animals[%s:%s]=%d"):format(suit, number, card_id)
	local skillcard = sgs.Card_Parse(card_str)
	assert(skillcard)
	return skillcard
end

-- 擀面杖
local rollingpin_skill={}
rollingpin_skill.name = "rollingpin"
table.insert(sgs.ai_skills, rollingpin_skill)
rollingpin_skill.getTurnUseCard = function(self)
	if self.player:hasUsed("RollingpinCard") or self.player:getHandcardNum()<2 then return end
	if self.player:getLostHp() < 2 then return end
	local cards = self.player:getHandcards()
	cards=sgs.QList2Table(cards)
	local first, second
	self:sortByUseValue(cards,true)
	for _, card in ipairs(cards) do
		if card:getTypeId() ~= sgs.Card_Equip then
			if not first then first  = cards[1]:getEffectiveId()
			else second = cards[2]:getEffectiveId()
			end
		end
		if second then break end
	end
	if not second then return end
	local card_str = ("@RollingpinCard=%d+%d"):format(first, second)
	assert(card_str)
	return sgs.Card_Parse(card_str)
end
sgs.ai_skill_use_func["RollingpinCard"] = function(card,use,self)
	use.card = card
end

-- 麻醉枪
sgs.ai_skill_invoke["tranqgun"] = function(self, data)
	if not self:isEnemy(data:toPlayer()) then return false end
	local cards = self.player:getHandcards()
	for _, card in sgs.qlist(cards) do
		if card:getSuit() == sgs.Card_Club then
			return true
		end
	end
	return false
end

-- 大圣旗
local wookon_skill={}
wookon_skill.name = "wookon"
table.insert(sgs.ai_skills, wookon_skill)
wookon_skill.getTurnUseCard = function(self)
	if self.player:hasUsed("WookonCard") or self.player:getHandcardNum() < 2 then return end
	local card_id = self:getCardRandomly(self.player, "h")
	local card_str = ("@WookonCard=" .. card_id)
	assert(card_str)
	return sgs.Card_Parse(card_str)
end
sgs.ai_skill_use_func["WookonCard"] = function(card,use,self)
	use.card = card
end

-- clearShirt
function SmartAI:useClearShirt(card, use)
	local next_player = self.player:getNextAlive()
	if self:isFriend(next_player) then
		if self:isEquip("GaleShell", next_player) or
			(self:isEquip("SilverLion", next_player) and next_player:isWounded()) then
			use.card = card
		end
	elseif self:isEnemy(next_player) then
		if not self:isEquip("GaleShell", next_player) then
			use.card = card
		end
	end
	return
end

-- sacrifice
function SmartAI:useCardSacrifice(sacrifice, use)
	if self.player:hasSkill("wuyan") then return end
    if self.player:getHp() <= 2 then return end
	self:sort(self.friends_noself, "hp")
	for _, friend in ipairs(self.friends_noself) do
		if friend:isWounded() then
			use.card = sacrifice
			if use.to then
				use.to:append(friend)
			end
			break
		end
	end
end

-- yabian
sgs.ai_skill_invoke["yabian"] = function(self, data)
	local damage = data:toDamage()
	return self:isEnemy(damage.to)
end

-- wutian
sgs.ai_skill_use["@@wutian"] = function(self, prompt)
	local first, second
	self:sort(self.friends_noself, "hp")
	for _, friend in ipairs(self.friends_noself) do
		if not friend:isAllNude() and friend:getPile("wall"):length() < 2 then
			if not first then
				first = friend
			else
				second = friend
			end
		end
	end
	if first and second then
		self:log(first:getGeneralName() .. "+" .. second:getGeneralName())
		local first_o = first:objectName()
		local second_o = second:objectName()
		return ("@WutianCard=.->%s+%s"):format(first_o, second_o)
	elseif first and not second then
		self:log(first:getGeneralName())
		return ("@WutianCard=.->%s"):format(first:objectName())
	else
		return "."
	end
end

-- wuzheng
sgs.ai_skill_invoke["wuzheng"] = true

-- diezhi
local diezhi_skill = {}
diezhi_skill.name = "diezhi"
table.insert(sgs.ai_skills, diezhi_skill)
diezhi_skill.getTurnUseCard = function(self)
	if self.player:isLord() or self.player:getMark("@drig") < 1 then return end
	return sgs.Card_Parse("@DiezhiCard=.")
end
sgs.ai_skill_use_func["DiezhiCard"] = function(card, use, self)
	use.card = card
end
-- diezhi respond
sgs.ai_skill_choice["drig-guess"] = function(self, choices)
	local players = self.room:getOtherPlayers(self.player)
	players = sgs.QList2Table(players)
	local kanze
	for _, other in ipairs(players) do
		if other:hasSkill("diezhi") then kanze = other break end
	end
	if kanze:isLord() then return "lord"
	else
		local r = math.random(0, 1)
		if r == 0 then
			return "loyalist"
		else
			return "rebel"
		end
	end
end

-- rangli
sgs.ai_skill_invoke["rangli"] = function(self, data)
	local players = sgs.QList2Table(self.room:getAllPlayers())
	for _, player in ipairs(players) do
		if not player:isKongcheng() then
			return true
		end
	end
	return false
end
sgs.ai_skill_playerchosen["rangli"] = function(self, targets)
	local next_player = self.player:getNextAlive()
	targets = sgs.QList2Table(targets)
	self:sort(targets, "hp")
	for _, target in ipairs(targets) do
		if next_player == target and self:isFriend(target) and target:getHandcardNum() > 1 then
			return target
		end
		if self:isEnemy(target) and target:getHp() < 2 and target:getHandcardNum() > 1 and
			not target:hasSkill("lianying") and
			not target:hasSkill("shangshi") and
			not target:hasSkill("tuntian") then
			return target
		end
	end
	return targets[1]
end

-- yuren
sgs.ai_skill_playerchosen["yuren"] = function(self, targets)
	targets = sgs.QList2Table(targets)
	self:sort(targets, "hp")
	for _, friend in ipairs(targets) do
		if self:isFriend(friend) and friend:hasSkill("rende") and self.player.isWounded() then
			return friend
		end
	end
	for _, friend in ipairs(targets) do
		if self:isFriend(friend) then
			return friend
		end
	end
	return self.friends[1]
end

-- zhenlie
sgs.ai_skill_invoke["zhenlie"] = function(self, data)
	return self.player:getRole() ~= "renegade"
end
sgs.ai_skill_playerchosen["zhenlie"] = function(self, targets)
	targets = sgs.QList2Table(targets)
	self:sort(targets, "handcard")
	for _, friend in ipairs(targets) do
		if self:isFriend(friend) then
			return friend
		end
	end
	return self.friends[1]
end

-- baichu
sgs.ai_skill_invoke["baichu"] = true
local baichu_skill = {}
baichu_skill.name = "baichu"
table.insert(sgs.ai_skills, baichu_skill)
baichu_skill.getTurnUseCard = function(self)
	if self.player:hasUsed("BaichuCard") then return end
	local cards = self.player:getHandcards()
	cards = sgs.QList2Table(cards)
	for _, card in ipairs(cards) do
		if card:getNumber() > 5 and card:getNumber() < 9 then
			return sgs.Card_Parse("@BaichuCard=" .. card:getEffectiveId())
		end
	end
end
sgs.ai_skill_use_func["BaichuCard"] = function(card, use, self)
	use.card = card
end

-- nongquan
sgs.ai_skill_invoke["nongquan"] = true

-- jielue
sgs.ai_skill_invoke["jielue"] = function(self, data)
	local effect = data:toSlashEffect()
	return self:isEnemy(effect.to)
end
