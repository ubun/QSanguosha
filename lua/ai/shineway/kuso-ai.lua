-- KusoPackage's AI by Ubun.

-- huaxu
sgs.ai_skill_invoke["huaxu"] = function(self, data)
	local damage = data:toDamage()
	return self:isEnemy(damage.to)
end
