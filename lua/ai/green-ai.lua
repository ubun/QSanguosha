-- GreenPackage's AI by Ubun.

-- yabian
sgs.ai_skill_invoke["yabian"] = function(self, data)
	local damage = data:toDamage()
	return self:isEnemy(damage.to)
end
