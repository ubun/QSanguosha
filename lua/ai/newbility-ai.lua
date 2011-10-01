-- diezhi
local kanze_ai = SmartAI:newSubclass "kanze"

function kanze_ai:activate_dummy(use)
	super.activate(self, use)
	use.card = sgs.Card_Parse("@DiezhiCard=.")
	return
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