module("extensions.YKStdGeneral", package.seeall)

extension = sgs.Package("YKStdGeneral")

--0201 Caocao 
--jianxiong by hypercross
luajianxiong=sgs.CreateTriggerSkill{
	
	frequency = sgs.Skill_NotFrequent,
	
	name      = "luajianxiong",
	
	events={sgs.Damaged}, --或者events=sgs.Damaged
	
	on_trigger=function(self,event,player,data)
		local room = player:getRoom()
		local card = data:toDamage().card
		if not room:obtainable(card,player) then return end
		if room:askForSkillInvoke(player,"luajianxiong") then
			room:playSkillEffect("luajianxiong")
			player:obtainCard(card)
		end
	end
}

--hujia by ibicdlcod
luahujia=sgs.CreateTriggerSkill{

	name = "luahujia$",
	
	default_choice = "ignore",
	
	events={sgs.CardAsked,sgs.Damaged},
	
	on_trigger=function(self,event,player,data)
		local room=player:getRoom()
		if(not player:hasLordSkill("luahujia")) then return false end
		
		if(data:toString() ~= "jink") then return false end
		
		if(not room:askForSkillInvoke(player, "luahujia")) then return false end
		room:playSkillEffect("luahujia")
		for _,liege in sgs.qlist(room:getOtherPlayers(player)) do
			local data=sgs.QVariant(0)
			local jink=0
			if(liege:getKingdom()~="wei") then return false end
			data:setValue(player)
			jink = room:askForCard(liege, "jink", "@hujia-jink", data--[["@hujia-jink:"+player:getGeneralName(), player]])
			if(jink) then
				room:provide(jink)
				return true
			end
		end
		return false
	end
}

--0202 Simayi
--fankui by ibicdlcod
luafankui = sgs.CreateTriggerSkill{
	
	frequency = sgs.Skill_NotFrequent,
	
	name      = "luafankui",
	
	events={sgs.Damaged}, --或者events=sgs.Damaged
	
	on_trigger=function(self,event,player,data)
		local room = player:getRoom()
		local from = data:toDamage().from
		local data = sgs.QVariant(0)
		data:setValue(from)
		if(from and (not from:isNude()) and room:askForSkillInvoke(player, "luafankui", data)) then
			local card_id = room:askForCardChosen(player, from, "he", "luafankui")
			if(room:getCardPlace(card_id) == sgs.Player_Hand) then
				room:moveCardTo(sgs.Sanguosha:getCard(card_id), player, sgs.Player_Hand, false)
			else
				room:obtainCard(player, card_id)
			end
			room:playSkillEffect("luafankui")
		end
	end

}

--guicai unf
--[[
luaguicai_card = sgs.CreateSkillCard{
	target_fixed = true,
	
	will_throw = false,
	
	on_use = function(self,room,source,targets)
	end
}

luaguicai_viewas = sgs.CreateViewAsSkill{
	
	name = "",
	
	n = 1,
	
	enabled_at_response=function()
		return sgs.Self:getPattern() == "@luaguicai"
	end,
	
	view_filter=function(self, selected, to_select)
		return not to_select:isEquipped()
	end,
	
	view_as=function(self, cards)
		local view_as_card = luaguicai_card:clone()
		for _,card in ipairs(cards) do
			view_as_card:addSubcard(card:getId())
		end
		
		view_as_card:setSkillName(self:objectName())
		
		return view_as_card
	end,
}

luaguicai = sgs.CreateTriggerSkill{
	view_as_skill = luaguicai_viewas,
	
	events = {sgs.AskForRetrial},
	
	on_trigger=function(self, event, player, data)
		local room = player:getRoom()
		local judge = data:toJudge()
		prompt = "@luaguicai-card:"+judge:who_get:objectName()+"::"+judge:reason_get+":"+judge:card_get:getEffectIdString()
		if(player:isKongcheng()) then return false end
		player:setTag("judge",data)
		local card = room:askForCard(player, "@luaguicai", prompt, data)
		if(card) then
			room:throwCard(judge:card_get)
			judge:card_set = sgs.Sanguosha:getCard(card:getEffectiveId())
			room:moveCardTo(judge:card_get, NULL, sgs.Player_Special)
			
            LogMessage log;
            log.type = "$ChangedJudge";
            log.from = player;
            log.to << judge->who;
            log.card_str = card->getEffectIdString();
            room->sendLog(log);

			room:sendJudgeResult(judge)
		end
	end,
}
]]

--0203 Xiahoudun
--ganglie by ibicdlcod
luaganglie = sgs.CreateTriggerSkill{
	
	name = "luaganglie",
	
	events = {sgs.Damaged},
	
	on_trigger=function(self,event,player,data)
		local room = player:getRoom()
		local from = data:toDamage().from
		source = sgs.QVariant(0)
		source:setValue(from)
		
		if(from and from:isAlive() and room:askForSkillInvoke(player, "luaganglie", source)) then
			room:playSkillEffect("luaganglie")
			
			local judge=sgs.JudgeStruct()
			judge.pattern=sgs.QRegExp("(.*):(heart):(.*)")
			judge.good=false
			judge.reason=self:objectName()
			judge.who=player

			room:judge(judge)
			if(judge:isGood()) then
				if(not room:askForDiscard(from, "luaganglie", 2, true)) then
					local damage = sgs.DamageStruct()
					damage.from = player
					damage.to = from
					
					room:damage(damage)
				end
				room:setEmotion(player, "good")
			else
				room:setEmotion(player, "bad")
			end
		end
	end
}

--0204 Zhangliao
--tuxi by ibicdlcod
luatuxi_card = sgs.CreateSkillCard{

	name = "luatuxi_card",
	
	target_fixed=false,
	
	will_throw=false,

		
	filter = function(self,targets,to_select)
		
		if(#targets > 1) then return false end
		
		if(to_select == self) then return false end
		
		return not to_select:isKongcheng()
	end,
		
	on_effect=function(self,effect)
		local from=effect.from
		local to  =effect.to
		local room=to:getRoom()
		local card_id= room:askForCardChosen(from, to, "h", "luatuxi_main")
		local card   = sgs.Sanguosha:getCard(card_id)
		room:moveCardTo(card, from, sgs.Player_Hand, false)
		
		room:setEmotion(to, "bad")
		room:setEmotion(from, "good")
	end,
}

luatuxi_viewas = sgs.CreateViewAsSkill{

	name="luatuxi_viewas",
	
	n = 0,

	view_as=function()
		return luatuxi_card:clone()		
	end,
	
	enabled_at_play=function()
		return false
	end,
	
	enabled_at_response=function(player,pattern)
		return pattern=="@@luatuxi_main"
	end
}

luatuxi_main = sgs.CreateTriggerSkill{

	name = "luatuxi_main",
	
	view_as_skill = luatuxi_viewas,
	
	events = {sgs.PhaseChange},
	
	on_trigger=function(self,event,player,data)
		if(player:getPhase() == sgs.Player_Draw) then
			local room=player:getRoom()
			local can_invoke=false
			local other=room:getOtherPlayers(player)
			for _,aplayer in sgs.qlist(other) do
				if(not aplayer:isKongcheng()) then
					can_invoke=true
					break
				end
			end
			if(not room:askForSkillInvoke(player, "luatuxi_main")) then return false end
			if(can_invoke and room:askForUseCard(player, "@@luatuxi_main", "@luatuxi_card")) then return true end
		return false
		end
	end
}

--0205 Xuchu
--luoyi by ibicdlcod
lualuoyi_buff = sgs.CreateTriggerSkill{
	
	name="#lualuoyi",
	
	events={sgs.Predamage},
	
	on_trigger=function(self,event,player,data)
		if(player:hasFlag("lualuoyi") and player:isAlive()) then
			local damage = data:toDamage()
			local room = player:getRoom()
			local reason = damage.card
			if(not reason) then return false end
			if(reason:inherits("Slash") or reason:inherits("Duel")) then
				--[[
				local log=sgs.LogMessage()
				log.type = "#luaLuoyiBuff"
				log.from = player
				log.to = damage.to
				log.arg = damage.damage
				log.arg2 = damage.damage+1
				room:sendLog(log)				
				
				room:askForSkillInvoke(player,"#lualuoyi")]]
				
				damage.damage = damage.damage+1
				data:setValue(damage)
				return false
			end
		else return false
		end
	end
}

lualuoyi = sgs.CreateTriggerSkill{
	name = "lualuoyi",
	
	events={sgs.DrawNCards},
	
	on_trigger=function(self,event,player,data)
		local room = player:getRoom()
		local x = data:toInt()
		if(room:askForSkillInvoke(player, "lualuoyi")) then
			room:playSkillEffect("lualuoyi")
			
			player:setFlags("lualuoyi")
			data:setValue(x-1)
		end
	end
}

--0206 Guojia
--tiandu by ibicdlcod

luatiandu = sgs.CreateTriggerSkill{
	name = "luatiandu",
	
	frequency = sgs.Skill_Frequent,
	
	events = {sgs.FinishJudge},
	
	on_trigger=function(self,event,player,data)
		local room = player:getRoom()
		local judge = data:toJudge()
		local card = judge.card
		data_card = sgs.QVariant(0)
		data_card:setValue(card)
		if(player:askForSkillInvoke("luatiandu", data_card)) then
			player:obtainCard(judge.card)
			room:playSkillEffect("luatiandu")
			return true
		end
		return false
	end
}

--yiji by ibicdlcod(unf)
luayiji = sgs.CreateTriggerSkill{
	name = "luayiji",
	
	frequency = sgs.Skill_Frequent,
	
	events = {sgs.Damaged},
	
	on_trigger=function(self,event,player,data)
		local room = player:getRoom()
		local damage = data:toDamage()
		local x = damage.damage
		local i = 0
		if(not room:askForSkillInvoke(player, "luayiji")) then return false end
		room:playSkillEffect("luayiji")
		repeat
			player:drawCards(2)--[[
			local good = room:askForPlayerChosen(player, room:getOtherPlayers(player), "luayiji")
			if(good) then
				local cards = sgs.qlist(player:handCards())
				good:obtainCard(cards.last())
			end]]
		until i == x - 1
	end
}

--0207 Zhenji
--qingguo by ibicdlcod(unf)
luaqingguo = sgs.CreateViewAsSkill{
	name = "luaqingguo",
	
	n = 1,
	
	view_filter = function(self, selected, to_select)
		return to_select:isBlack() and not to_select:isEquipped()
	end,
	
	view_as = function(self, cards)
		if #cards == 1 then
			local card = cards[1]
			local new_card =sgs.Sanguosha:cloneCard("jink", card:getSuit(), card:getNumber())
			new_card:addSubcard(card:getId())
			new_card:setSkillName(self:objectName())
			return new_card
		end
	end,
	
	enabled_at_play = function()
		return false
	end,
	
	enabled_at_response = function(player, pattern)
		return true --pattern == "jink"
	end
}

--luoshen by ibicdlcod
lualuoshen = sgs.CreateTriggerSkill{
	name = "lualuoshen",
	
	frequency = sgs.Skill_Frequent,
	
	events = {sgs.PhaseChange,sgs.FinishJudge},
	
	on_trigger=function(self,event,player,data)
		if(event == sgs.PhaseChange and player:getPhase() == sgs.Player_Start) then
			local room = player:getRoom()
			while(player:askForSkillInvoke("lualuoshen")) do
				room:playSkillEffect("lualuoshen")
				
				    local judge=sgs.JudgeStruct()
                    judge.pattern=sgs.QRegExp("(.*):(spade|club):(.*)")
                    judge.good=true
                    judge.reason="lualuoshen"
                    judge.who=player
                    room:judge(judge)
				if(judge:isBad()) then
					break
				end
			end
		else if(event == sgs.FinishJudge) then
			judge = data:toJudge()
			if(judge.reason == "lualuoshen") then
				if(judge.card:isBlack()) then
					player:obtainCard(judge.card)
					return true
				end
			end
		end
		end
	end
}

--0201
luacaocao = sgs.General(extension, "luacaocao$", "wei", 4)
luacaocao:addSkill(luajianxiong)
luacaocao:addSkill(luahujia)

--0202
luasimayi = sgs.General(extension, "luasimayi", "wei", 3)
luasimayi:addSkill(luafankui)
--luasimayi:addSkill(luaguicai)

--0203
luaxiahoudun = sgs.General(extension, "luaxiahoudun", "wei", 4)
luaxiahoudun:addSkill(luaganglie)

--0204
luazhangliao = sgs.General(extension, "luazhangliao", "wei", 4)
luazhangliao:addSkill(luatuxi_main)

--0205
luaxuchu = sgs.General(extension, "luaxuchu", "wei", 4)
luaxuchu:addSkill(lualuoyi_buff)
luaxuchu:addSkill(lualuoyi)

--0206
luaguojia = sgs.General(extension, "luaguojia", "wei", 3)
luaguojia:addSkill(luatiandu)
luaguojia:addSkill(luayiji)

--0207
luazhenji = sgs.General(extension, "luazhenji", "wei", 3, false)
luazhenji:addSkill(luaqingguo)
luazhenji:addSkill(lualuoshen)

sgs.LoadTranslationTable{
	["YKStdGeneral"] = "YOKA标准武将",
}
