-- kaituo
local kaituo_skill = {}
kaituo_skill.name = "kaituo"
table.insert(sgs.ai_skills, kaituo_skill)
kaituo_skill.getTurnUseCard = function(self)
	if self.player:getHandcardNum() < 2 or not self.player:isWounded() then return nil end
	local cards = self.player:getHandcards()
	cards=sgs.QList2Table(cards)
	local discard = {}
	self:sortByUseValue(cards, true)
	for _, card in ipairs(cards) do
		table.insert(discard, card:getEffectiveId())
		if #discard == 2 then break end
	end
	if #discard ~= 2 then return end
	return sgs.Card_Parse("@KaituoCard=" .. table.concat(discard, "+"))
end
sgs.ai_skill_use_func["KaituoCard"]=function(card,use,self)
	use.card = card
end

-- gouxian
local gouxian_skill = {}
gouxian_skill.name = "gouxian"
table.insert(sgs.ai_skills, gouxian_skill)
gouxian_skill.getTurnUseCard = function(self, inclusive)
    local cards = self.player:getHandcards()
    cards=sgs.QList2Table(cards)
	self:sortByUseValue(cards,true)
	local gx
	for _,card in ipairs(cards)  do
		if not card:inherits("Slash") and not card:inherits("Peach")
			and ((self:getUseValue(card)<sgs.ai_use_value["Slash"]) or inclusive) then
			gx = card
			break
		end
	end
	if gx then		
		local suit = gx:getSuitString()
    	local number = gx:getNumberString()
		local card_id = gx:getEffectiveId()
		local card_str = ("slash:gouxian[%s:%s]=%d"):format(suit, number, card_id)
		local slash = sgs.Card_Parse(card_str)
		assert(slash)
        return slash
	end
end

-- shexian
local shexian_skill = {}
shexian_skill.name = "shexian"
table.insert(sgs.ai_skills, shexian_skill)
shexian_skill.getTurnUseCard = function(self)
	if (self.player:getHp() > 3 and self.player:getHandcardNum() > self.player:getHp()) or		
		(self.player:getHp() - self.player:getHandcardNum() >= 2) then
		return sgs.Card_Parse("@ShexianCard=.")
	end
	if self.player:hasFlag("mp3") then
		for _, enemy in ipairs(self.enemies) do
			if self.player:canSlash(enemy, true) and self.player:getHp() > 1 then
				return sgs.Card_Parse("@ShexianCard=.")
			end
		end
	end
end
sgs.ai_skill_use_func["ShexianCard"]=function(card,use,self)
	use.card = card
end

-- mp3
local mp3_skill = {}
mp3_skill.name = "mp3"
table.insert(sgs.ai_skills, mp3_skill)
mp3_skill.getTurnUseCard = function(self)
	if not self.player:containsTrick("microphone") then return end
	if self.player:usedTimes("Mp3Card") > 0 then return nil end
	local card_str = "@Mp3Card=."
	local slash = sgs.Card_Parse(card_str)
	assert(slash)
	return slash
end
sgs.ai_skill_use_func["Mp3Card"] = function(card,use,self)
	self:sort(self.enemies, "handcard")
	if use.to then use.to:append(self.enemies[1]) end
	use.card = card
end

-- mp4
local mp4_skill = {}
mp4_skill.name = "mp4"
table.insert(sgs.ai_skills, mp4_skill)
mp4_skill.getTurnUseCard = function(self)
	if not self.player:containsTrick("microphone") or not self.player:isWounded() then return end
	if self.player:usedTimes("Mp4Card") > 0 then return nil end
	local cards = self.player:getHandcards()
	cards = sgs.QList2Table(cards)
	self:sortByKeepValue(cards)
	local card_str = ("@Mp4Card=%d"):format(cards[1]:getId())
	return sgs.Card_Parse(card_str)
end
sgs.ai_skill_use_func["Mp3Card"] = function(card,use,self)
	use.card = card
end

-- heiyi
local heiyi_skill = {}
heiyi_skill.name = "heiyi"
table.insert(sgs.ai_skills, heiyi_skill)
heiyi_skill.getTurnUseCard = function(self)
	return sgs.Card_Parse("@HeiyiCard=.")
end
sgs.ai_skill_use_func["HeiyiCard"] = function(card,use,self)
	use.card = card
end

-- ganglie
sgs.ai_skill_invoke.ganglie = function(self, data)
	return not self:isFriend(data:toPlayer())
end

-- fankui
sgs.ai_skill_invoke.fankui = function(self, data)
	local target = data:toPlayer()
	if self:isFriend(target) then
		return (target:hasSkill("xiaoji") and not target:getEquips():isEmpty()) or (self:isEquip("SilverLion",target) and target:isWounded())
	end
	if self:isEnemy(target) then				---fankui without zhugeliang and luxun
			if (target:hasSkill("kongcheng") or target:hasSkill("lianying")) and target:getHandcardNum() == 1 then
				if not target:getEquips():isEmpty() then return true
				else return false
				end
			end
	end
				--self:updateRoyalty(-0.8*sgs.ai_royalty[target:objectName()],self.player:objectName())
	return true
end

-- tieji
sgs.ai_skill_invoke.tieji = function(self, data)
	local effect = data:toSlashEffect()
	return not self:isFriend(effect.to) and (not effect.to:isKongcheng() or effect.to:getArmor())
end

sgs.ai_skill_use["@@liuli"] = function(self, prompt)

	local others=self.room:getOtherPlayers(self.player)
	others=sgs.QList2Table(others)
	local source
	for _, player in ipairs(others) do
		if player:hasFlag("slash_source") then
			source = player
			 break
		end
	end
	for _, enemy in ipairs(self.enemies) do
		if self.player:canSlash(enemy,true) and not (source:objectName() == enemy:objectName()) then
			local cards = self.player:getCards("he")
			cards=sgs.QList2Table(cards)
			for _,card in ipairs(cards) do
				if (self.player:getWeapon() and card:getId() == self.player:getWeapon():getId()) and self.player:distanceTo(enemy)>1 then local bullshit
				elseif card:inherits("OffensiveHorse") and self.player:getAttackRange()==self.player:distanceTo(enemy)
					and self.player:distanceTo(enemy)>1 then
					local bullshit
				else
					return "@LiuliCard="..card:getEffectiveId().."->"..enemy:objectName()
				end
			end
		end
	end
	return "."
end

sgs.ai_skill_invoke["@guicai"]=function(self,prompt,judge)
	judge = judge or self.player:getTag("Judge"):toJudge()

	if self:needRetrial(judge) then
		local cards = sgs.QList2Table(self.player:getHandcards())
		local card_id = self:getRetrialCardId(cards, judge)
		if card_id ~= -1 then
			self.room:writeToConsole(card_id)
			return "@GuicaiCard=" .. card_id
		end
	end

	return "."
end
