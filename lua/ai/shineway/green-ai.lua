-- GreenPackage's AI by Ubun.

-- yabian
sgs.ai_skill_invoke["yabian"] = function(self, data)
	local damage = data:toDamage()
	return self:isEnemy(damage.to)
end

-- yuanlv
sgs.ai_skill_invoke["yuanlv"] = true
local yuanlv_skill = {}
yuanlv_skill.name = "yuanlv"
table.insert(sgs.ai_skills, yuanlv_skill)
yuanlv_skill.getTurnUseCard = function(self)
	if self.player:hasUsed("YuanlvCard") or self.player:isWounded() then return end
	local yuanlv_card = sgs.Card_Parse("@YuanlvCard=.")
	assert(yuanlv_card)
	return yuanlv_card
end
sgs.ai_skill_use_func["YuanlvCard"] = function(card, use, self)
	if use.to then use.to:append(self.enemies[1]) end
	use.card = card
end

-- zhongjian
sgs.ai_skill_invoke["zhongjian"] = function(self, data)
	return #self.friends_noself ~= 0
end
sgs.ai_skill_playerchosen["zhongjian"] = function(self, targets)
	return self.friends_noself[1]
end

