-- card's AI is here.

function SmartAI:useCardSlash(card, use)
	if self:slashIsAvailable() then
	--	local target_count = 0
		self:sort(self.enemies, "hp")
		for _, enemy in ipairs(self.enemies) do
			local slash_prohibit = false
			slash_prohibit = self:slashProhibit(card,enemy)
			if not slash_prohibit then
				if self.player:hasFlag("mp3") and enemy:hasFlag("mp3") then
					use.card = card
					if use.to then use.to:append(enemy) end
				elseif self.player:canSlash(enemy) and self:objectiveLevel(enemy) > 3 then
					use.card = card
					if use.to then use.to:append(enemy) end
				--	target_count = target_count + 1
				--	if self.slash_targets <= target_count then 
					return
				--	end
				end
			end
		end
	end
end

function SmartAI:useCardPeach(card, use)
	self:sort(self.friends, "hp")
	for _, friend in ipairs(self.friends) do
		if friend:isWounded() then
			use.card = card
			if use.to then use.to:append(friend) end
			return
		end
	end
	return
end

function SmartAI:useCardConcludence(card, use)
	self:sort(self.enemies, "hp")
	use.card = card
	if use.to then use.to:append(self.enemies[1]) end
end

function SmartAI:useCardExNihilo(card, use)
	use.card = card
	if not use.isDummy then
		self:speak("lucky")
	end
end

function SmartAI:useCardIgnore(card, use)
	if self.player:getPile("ignore"):isEmpty() then
		use.card = card
	end
end
sgs.ai_skill_invoke["ignore"] = function(self, data)
	local effect = data:toCardEffect()
	if effect.card:inherits("Duel") and self:getCardsNum("Nullification") == 0 then
		return true
	elseif effect.card:inherits("Slash") then
		local jink = self:getCardsNum("Jink") + self:getCardsNum("Ingenarg") + self:getCardsNum("Nullification")
		return jink == 0
	else
		return true
	end
end

function SmartAI:useCardAmazingGrace(card, use)
	if #self.friends >= #self.enemies then
		use.card = card
	end
end

local function factorial(n)
	if n <= 0.1 then return 1 end
	return n*factorial(n-1)
end

function SmartAI:useCardDuel(duel, use)
	self:sort(self.enemies,"handcard")
	local enemies = self:exclude(self.enemies, duel)
	for _, enemy in ipairs(enemies) do
		if self:objectiveLevel(enemy) > 3 then
			local n1 = self:getCardsNum("Slash")
			local n2 = enemy:getHandcardNum()
			if enemy:hasSkill("zhizhi") then n2 = n2*2 end
			local useduel
			if self:hasTrickEffective(duel, enemy) then
				if n1 >= n2 then
					useduel = true
				elseif n1 > 0 then
					local percard = 0.35
					if enemy:hasSkill("zhuisuo") or enemy:hasFlag("mp3") then percard = 0.2 end
					local poss = percard ^ n1 * (factorial(n1)/factorial(n2)/factorial(n1-n2))
					if math.random() > poss then useduel = true end
				end
				if useduel then
					use.card = duel
					if use.to then
						use.to:append(enemy)
						self:speak("duel", self.player:getGeneral():isFemale())
					end
					return
				end
			end
		end
	end
end

function SmartAI:useCardDismantlement(dismantlement, use)
	self:sort(self.enemies,"defense")
	for _, enemy in ipairs(self.enemies) do
		if getDefense(enemy) < 8 then break
		else self:sort(self.enemies,"threat")
		break
		end
	end
	local enemies = self:exclude(self.enemies, dismantlement)
	for _, enemy in ipairs(enemies) do
		local equips = enemy:getEquips()
		    if not enemy:isKongcheng() and self:hasTrickEffective(dismantlement, enemy) then
				if enemy:getHandcardNum() == 1 then
					if enemy:hasSkill("kongcheng") or enemy:hasSkill("lingxi") then return end
				end
				use.card = dismantlement
                if use.to then
                    use.to:append(enemy)
                    self:speak("hostile", self.player:getGeneral():isFemale())
                end
				return
		    end
	end
end

function SmartAI:useCardSpeak(card, use)
	if not self.player:containsTrick("microphone") then
		use.card = card
	end
end
