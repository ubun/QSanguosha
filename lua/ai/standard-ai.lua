-- jianxiong
sgs.ai_skill_invoke.jianxiong = function(self, data)
		return not sgs.Shit_HasShit(data:toCard())
end --奸雄：非屎必然发动

sgs.ai_skill_invoke.jijiang = function(self, data)
	if self:getCardsNum("Slash")<=0 then
		return true
	end
	return false
end --激将：无杀发动

sgs.ai_skill_choice.jijiang = function(self , choices)
	if not self.player:hasLordSkill("jijiang") then
		if self:getCardsNum("Slash") <= 0 then return "ignore" end
	end

	if self.player:isLord() then
		local target
		for _, player in sgs.qlist(self.room:getOtherPlayers(self.player)) do
			if player:hasSkill("weidi") then
				target = player
				break
			end
		end
		if target and self:isEnemy(target) then return "ignore" end
	elseif self:isFriend(self.room:getLord()) then return "accept" end
	return "ignore"
end --响应激将：无杀则无视，自己为主公，伪帝激将敌人无视，不为主公则友方接受敌方无视

sgs.ai_skill_choice.hujia = function(self , choices)
	if not self.player:hasLordSkill("hujia") then
		if self:getCardsNum("Jink") <= 0 then return "ignore" end
	end
	if self.player:isLord() then
		local target
		for _, player in sgs.qlist(self.room:getOtherPlayers(self.player)) do
			if player:hasSkill("weidi") then
				target = player
				break
			end
		end
		if target and self:isEnemy(target) then return "ignore" end
	elseif self:isFriend(self.room:getLord()) then return "accept" end
	return "ignore"
end --响应护驾：同理

-- hujia
sgs.ai_skill_invoke.hujia = function(self, data)
	local cards = self.player:getHandcards()
	for _, friend in ipairs(self.friends_noself) do
		if friend:getKingdom() == "wei" and self:isEquip("EightDiagram", friend) then return true end
	end
	for _, card in sgs.qlist(cards) do
		if card:inherits("Jink") then
			return false
		end
	end
	return true
end --护驾：友方魏将有八卦发动，自己有闪不发动，除此之外发动

-- tuxi
sgs.ai_skill_use["@@tuxi"] = function(self, prompt)
	self:sort(self.enemies, "handcard")

	local first_index, second_index
	for i=1, #self.enemies-1 do
		if self:hasSkills(sgs.need_kongcheng, self.enemies[i]) and self.enemies[i]:getHandcardNum() == 1 then
		elseif not self.enemies[i]:isKongcheng() then
			if not first_index then
				first_index = i
			else
				second_index = i
			end
		end
		if second_index then break end
	end

	if first_index and not second_index then
		local others = self.room:getOtherPlayers(self.player)
		for _, other in sgs.qlist(others) do
			if (not self:isFriend(other) or (self:hasSkills(sgs.need_kongcheng, other) and other:getHandcardNum() == 1)) and
				self.enemies[first_index]:objectName() ~= other:objectName() and not other:isKongcheng() then
				return ("@TuxiCard=.->%s+%s"):format(self.enemies[first_index]:objectName(), other:objectName())
			end
		end
	end

	if not second_index then return "." end

	self:log(self.enemies[first_index]:getGeneralName() .. "+" .. self.enemies[second_index]:getGeneralName())
	local first = self.enemies[first_index]:objectName()
	local second = self.enemies[second_index]:objectName()
	return ("@TuxiCard=.->%s+%s"):format(first, second)
end --突袭卡：敌方1牌有空城技不突，否则加入突袭对象，若满则返回，否则检查友方有无空城技1牌者，若满则突袭，不满则不突袭

-- yiji (frequent)
sgs.ai_skill_invoke.tiandu = sgs.ai_skill_invoke.jianxiong

-- 遗计常用技，天妒：同奸雄

-- ganglie
sgs.ai_skill_invoke.ganglie = function(self, data)
	return not self:isFriend(data:toPlayer())
end -- 刚烈：友方不刚烈

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
end -- 反馈：反馈友方的孙尚香装备/白银狮子（受伤时）（建议血少于等于一半再拆），敌方空城/连营且1牌只反馈装备

-- tieji
sgs.ai_skill_invoke.tieji = function(self, data)
	local effect = data:toSlashEffect()
	return not self:isFriend(effect.to) and (not effect.to:isKongcheng() or effect.to:getArmor())
end -- 铁骑：友方不铁骑，空城不铁骑，有防具铁骑（not是怎么回事我没看懂）

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
end -- 流离卡：local bullshit是啥？

sgs.ai_skill_invoke["@guicai"]=function(self,prompt,judge)
	judge = judge or self.player:getTag("Judge"):toJudge()

	if self:needRetrial(judge) then
		local cards = sgs.QList2Table(self.player:getHandcards())
		local card_id = self:getRetrialCardId(cards, judge)
		if card_id ~= -1 then
			return "@GuicaiCard=" .. card_id
		end
	end

	return "."
end
