-- RedPackage's AI by Ubun.

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

-- zhubing
sgs.ai_skill_invoke["zhubing"] = function(self, data)
	return self.player:getLostHp() > 1
end
sgs.ai_skill_invoke["super_zaiqi"] = sgs.ai_skill_invoke["zhubing"]

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

dofile "lua/ai/shineway/kuso-ai.lua"
