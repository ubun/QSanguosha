-- PurplePackage's AI by Ubun.

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
