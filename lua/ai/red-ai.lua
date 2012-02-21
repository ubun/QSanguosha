-- RedPackage's AI by Ubun.

-- tianhui
-- jifeng

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
sgs.ai_view_as.baichu = function(card, player, card_place)
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	local jis = player:getPile("ji")
	jis = sgs.QList2Table(jis)
	local baichu_card = sgs.Sanguosha:getCard(jis[1])
	if card:getNumber() >= baichu_card:getNumber() then
		return ("jink:baichu[%s:%s]=%d"):format(suit, number, card_id)
	end
end

-- tonglu-response
sgs.ai_skill_choice["tonglu"] = function(self, choices)
	local hejin = self.room:findPlayerBySkillName("tonglu")
	if hejin and self:isFriend(hejin) then
		if self.player:getHp() > 2 and self.player:getHandcardNum() > 2 then
			return "agree"
		elseif self:isEquip("Axe", hejin) then
			return "agree"
		else
			return "deny"
		end
	end
end

-- liehou
sgs.ai_skill_choice["liehou"] = function(self, choices)
	for _, player in sgs.qlist(self.room:getAllPlayers()) do
		if not player:faceUp() and self:isFriend(player) then
			if choices == "get+draw+cancel" then return "get" end
			return "draw"
		end
	end
	return "cancel"
end
sgs.ai_skill_playerchosen["liehou"] = function(self, targets)
	for _, player in sgs.qlist(targets) do
		if self:isFriend(player) and player:getHandcardNum() < 2 then
			return player
		end
	end
	return self.friends[1]
end

-- xiefang
sgs.ai_skill_invoke["xiefang"] = true
local xiefang_skill = {}
xiefang_skill.name = "xiefang"
table.insert(sgs.ai_skills, xiefang_skill)
xiefang_skill.getTurnUseCard = function(self)
	if not self:slashIsAvailable(self.player) then return end
	for _, enemy in ipairs(self.enemies) do
		local weapon = enemy:getWeapon()
		if weapon then
			return sgs.Card_Parse("@XiefangCard=.")
		end
	end
end
sgs.ai_skill_use_func["XiefangCard"] = function(card, use, self)
	self:sort(self.enemies, "threat")

	for _, friend in ipairs(self.friends_noself) do
		if friend:getWeapon() and self:hasSkills(sgs.lose_equip_skill, friend) then
			for _, enemy in ipairs(self.enemies) do
				if self.player:canSlash(enemy) then
					use.card = card
					if use.to then
						use.to:append(friend)
						use.to:append(enemy)
					end
					return
				end
			end
		end
	end
	for _, enemy in ipairs(self.enemies) do
		if not self.room:isProhibited(self.player, enemy, card)
			and not self:hasSkill(sgs.lose_equip_skill, enemy)
			and enemy:getWeapon() then

			local enemies = self.enemies
			self:sort(enemies, "handcard")
			for _, enemy2 in ipairs(enemies) do
				if self.player:canSlash(enemy2) then
					use.card = card
					if use.to then
						use.to:append(enemy)
						if enemy ~= enemy2 then
							use.to:append(enemy2)
						end
					end
					return
				end
			end
		end
	end
	return "."
end

-- xiefang-slash&jink
sgs.ai_skill_invoke["xiefang"] = function(self, data)
	local asked = data:toString()
	for _, enemy in ipairs(self.enemies) do
		local weapon = enemy:getWeapon()
		local armor = enemy:getArmor() or enemy:getDefensiveHorse() or enemy:getOffensiveHorse()
		if (asked == "slash" and weapon) or (asked == "jink" and armor) then
			return true
		end
	end
	return false
end
sgs.ai_skill_playerchosen["xiefang"] = function(self, targets)
	for _, player in sgs.qlist(targets) do
		if self:isEnemy(player) and not self:hasSkills(sgs.lose_equip_skill, player) then
			return player
		elseif self:isFriend(player) and self:hasSkills(sgs.lose_equip_skill, player) then
			return player
		end
	end
end
sgs.ai_skill_cardchosen["xiefang"] = function(self, who)
	local ecards = who:getCards("e")
	ecards = sgs.QList2Table(ecards)
	for _, unweapon in ipairs(ecards) do
		if not unweapon:inherits("Weapon") then
			return unweapon
		end
	end
end

-- yanyun
sgs.ai_skill_cardask["@yanyun-slash"] = function(self, data, pattern, target)
	local cards = self.player:getHandcards()
	for _, card in sgs.qlist(cards) do
		if card:inherits("Slash") then
			return card:getEffectiveId()
		end
	end
	return "."
end

-- zhubing
sgs.ai_skill_invoke["zhubing"] = sgs.ai_skill_invoke["zaiqi"]
sgs.ai_skill_invoke["super_zaiqi"] = sgs.ai_skill_invoke["zaiqi"]

-- jielue
sgs.ai_skill_invoke["jielue"] = function(self, data)
	local effect = data:toSlashEffect()
	return self:isEnemy(effect.to)
end

-- xujiu
sgs.ai_skill_invoke["xujiu"] = function(self, data)
	local damage = data:toDamage()
	return self:isEnemy(damage.to)
end
sgs.ai_skill_cardask["@xujiu-ask"] = function(self, data, pattern, target)
	local jiu = self.player:getPile("niangA"):length() + self.player:getPile("niangB"):length()
	if jiu < 3 then
		local cards = self.player:getHandcards()
		for _, card in sgs.qlist(cards) do
			if card:isBlack() then
				return card:getEffectiveId()
			end
		end
	end
	return "."
end

-- goulian
sgs.ai_skill_invoke["goulian"] = true
local goulian_skill = {}
goulian_skill.name = "goulian"
table.insert(sgs.ai_skills, goulian_skill)
goulian_skill.getTurnUseCard = function(self)
	if self.player:hasUsed("GoulianCard") or self.player:isKongcheng() then return end
	local card = self.player:getRandomHandCard()
	return sgs.Card_Parse("@GoulianCard=" .. card:getEffectiveId())
end
sgs.ai_skill_use_func["GoulianCard"] = function(card, use, self)
	for _, player in ipairs (self.friends_noself) do
		if player:getGeneral():isMale() then
			if use.to then use.to:append(player) end
			use.card = card
			return
		end
	end
end
sgs.ai_skill_choice["goulian"] = function(self, choices)
	if self.player:isWounded() then
		return "a"
	end
	local luban = self.room:findPlayerBySkillName("goulian")
	if luban and self:isFriend(luban) then
		return "b"
	end
	return "a"
end

