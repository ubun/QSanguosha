-- diezhi
diezhi_skill={}
diezhi_skill.name = "diezhi"
table.insert(sgs.ai_skills, diezhi_skill)
diezhi_skill.getTurnUseCard = function(self)
	if self.player:isLord() or self.player:getMark("@drig") < 1 then return end
	if self.player:getHandcardNum() > self.player:getHp() then
		return sgs.Card_Parse("@DiezhiCard=.")
	end
end
sgs.ai_skill_use_func["DiezhiCard"]=function(card,use,self)
	use.card = card
end

sgs.ai_skill_choice["drig-guess"] = function(self, choices)
	local players = self.room:getOtherPlayers(self.player)
	players = sgs.QList2Table(players)
	local kanze
	for _, other in ipairs(players) do
		if other:hasSkill("diezhi") then kanze = other break end
	end
	if kanze:isLord() then return "lord" 
	else
		local r = math.random(0, 1)
		if r == 0 then
			return "loyalist"
		else
			return "rebel"
		end
	end
end