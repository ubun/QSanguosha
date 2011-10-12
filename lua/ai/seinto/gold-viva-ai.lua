-- xiufu
sgs.ai_skill_invoke["xiufu"] = function(self, data)
	local dying = data:toDying()
	return self:isFriend(dying.who)
end

-- jingqiang
sgs.ai_skill_invoke["jingqiang"] = function(self, data)
	return true
end
sgs.ai_skill_playerchosen["jingqiang"] = function(self, targets)
	for _, player in sgs.qlist(targets) do
		if self:isFriend(player) then
			return player
		end
	end
end

-- haojiao
sgs.ai_skill_use["@@haojiao"] = function(self, prompt)
	local first_index, second_index
	for i=1, #self.enemies-1 do																			
		if (self.enemies[i]:hasSkill("kongcheng") and self.enemies[i]:getHandcardNum() == 0)  then 
				local bullshit

		elseif not first_index then 
				first_index = i 
			else 
				second_index = i 
		end
		if second_index then break end
	end

	if first_index and not second_index then
		local others = self.room:getOtherPlayers(self.player)
		for _, other in sgs.qlist(others) do
			if (not self:isFriend(other) or (self:hasSkills(sgs.need_kongcheng, other) and other:getHandcardNum() == 0)) and 
				self.enemies[first_index]:objectName() ~= other:objectName() then 
				return ("@HaojiaoCard=.->%s+%s"):format(self.enemies[first_index]:objectName(), other:objectName())
			end
		end
	end

	if not second_index then return "." end

	self:log(self.enemies[first_index]:getGeneralName() .. "+" .. self.enemies[second_index]:getGeneralName())
	local first = self.enemies[first_index]:objectName()
	local second = self.enemies[second_index]:objectName()
	return ("@HaojiaoCard=.->%s+%s"):format(first, second)
end

-- longba
sgs.ai_skill_invoke["longba"] = function(self, data)
	return true
end

-- hongzhen
sgs.ai_skill_invoke["hongzhen"] = function(self, data)
	return true
end
sgs.ai_skill_playerchosen["hongzhen"] = function(self, targets)
	for _, player in sgs.qlist(targets) do
		if self:isEnemy(player) and player:getHp() <= 3 then
			return player
		end
	end
end

-- binghuan
sgs.ai_skill_invoke["binghuan"] = function(self, data)
	return true
end

-- meigui
sgs.ai_skill_invoke["meigui"] = function(self, data)
	local effect = data:toSlashEffect()
	if effect.slash:isRed() then
		return true
	elseif effect.slash:isBlack() then
		return true
	end
end
