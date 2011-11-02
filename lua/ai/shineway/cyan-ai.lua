-- CyanPackage's AI by Ubun.

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
