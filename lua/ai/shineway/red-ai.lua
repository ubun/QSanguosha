-- RedPackage's AI by Ubun.

-- tongmou
sgs.ai_skill_invoke["tongmou"] = true
sgs.ai_skill_playerchosen["tongmou_tie"] = function(self, targets)
	for _, player in sgs.qlist(targets) do
		if self:isEnemy(player) and player:getHandcardNum() > 2 then
			return player
		end
	end
	return self.enemies[1]
end
sgs.ai_skill_invoke["tongmou"] = true
local tongmou_skill = {}
tongmou_skill.name = "tongmou"
table.insert(sgs.ai_skills, tongmou_skill)
tongmou_skill.getTurnUseCard = function(self)
	if self.player:hasUsed("TongmouCard") then return end
	return sgs.Card_Parse("@TongmouCard=.")
end
sgs.ai_skill_use_func["TongmouCard"] = function(card, use, self)
	use.card = card
end

-- xianhai
sgs.ai_skill_invoke["xianhai"] = true
local xianhai_skill = {}
xianhai_skill.name = "xianhai"
table.insert(sgs.ai_skills, xianhai_skill)
xianhai_skill.getTurnUseCard = function(self)
	local cards = self.player:getHandcards()
	cards = sgs.QList2Table(cards)
	for _, card in ipairs(cards) do
		if card:inherits("Disaster") then
			return sgs.Card_Parse("@XianhaiCard=" .. card:getEffectiveId())
		end
	end
end
sgs.ai_skill_use_func["XianhaiCard"] = function(card, use, self)
	use.card = card
	if use.to then use.to:append(self.enemies[1]) end
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

-- tonglu-response
sgs.ai_skill_choice["tonglu"] = function(self, choices)
	local hejin = self.room:findPlayerBySkillName("tonglu")
	if hejin and self:isFriend(hejin) and self.player:getHp() > 2 and self.player:getHandcardNum() > 2 then
		return "agree"
	else
		return "deny"
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
--	self:sort(self.enemies, "handcard")
	self:sort(self.enemies, "threat")
	
	for _, friend in ipairs(self.friends_noself) do
		if friend:getWeapon() and self:hasSkills(sgs.lose_equip_skill, friend) then
			for _, enemy in ipairs(self.enemies) do
				if self.player:canSlash(enemy) then
					use.card = card
				end
				if use.to then
					use.to:append(friend)
					use.to:append(enemy)
				end
				return
			end
		end
	end

	local n = nil 
	local final_enemy = nil
	for _, enemy in ipairs(self.enemies) do
		if not self.room:isProhibited(self.player, enemy, card)
			and not self:hasSkill(sgs.lose_equip_skill, enemy)
			and enemy:getWeapon() then

			for _, enemy2 in ipairs(self.enemies) do
				if self.player:canSlash(enemy2) then
					if enemy:getHandcardNum() == 0 then
						use.card = card
						if enemy == enemy2 then
							if use.to then use.to:append(enemy) end
							return
						end
						if use.to then
							use.to:append(enemy)
							use.to:append(enemy2)
						end
						return
					else
						n = 1;
						final_enemy = enemy2
					end
				end
			end
			if n then use.card = card end
			if enemy == final_enemy then
				if use.to and self.player:canSlash(enemy) then use.to:append(enemy) end
				return
			end
			if use.to then
				use.to:append(enemy)
				use.to:append(final_enemy)
			end
			return
		end
		n = nil
	end
end

-- xiefang-slash&jink
sgs.ai_skill_invoke["xiefang"] = function(self, data)
	local asked = data:toString()
	for _, enemy in ipairs(self.enemies) do
		local weapon = enemy:getWeapon()
		local armor = enemy:getArmor() or enemy:getDefensiveHorse() or enemy:getOffensiveHorse()
		if asked == "slash" then
			return weapon
		elseif asked == "jink" then
			return armor
		end
	end
end
sgs.ai_skill_playerchosen["xiefang"] = function(self, targets)
	for _, player in sgs.qlist(targets) do
		if self:isEnemy(player) then
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
sgs.ai_skill_invoke["slash"] = function(self, prompt, data)
	if prompt ~= "yanyun-slash" then return end
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
sgs.ai_skill_invoke[".black"]=function(self, prompt, data)
	if prompt ~= "xujiu_ask" then return end
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

