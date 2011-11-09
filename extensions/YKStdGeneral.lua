module("extensions.YKStdGeneral", package.seeall)

extension = sgs.Package("YKStdGeneral")

--0101 liubei unf
Rende_Card = sgs.CreateSkillCard{
	name = "Rende_card",
	will_throw = false,
	
	on_use = function(self, room, source, targets)
		local target = nil
		if(targets:isEmpty()) then--[[
			table.foreach(room:getAlivePlayers(),
				function(player, v)
					if(player ~= source) then
						target = player
					break
					end
				end
			)]]
		else
			target = targets:first()

			room:moveCardTo(this, target, sgs.Player_Hand, false)

			local old_value = source:getMark("rende");
			local new_value = old_value + subcards:length();
			room:setPlayerMark(source, "rende", new_value);

			if(old_value < 2 and new_value >= 2) then
				local recover = sgs.RecoverStruct()
				recover.card = this;
				recover.who = source;
				room:recover(source, recover);
			end
		end
	end,
}

--0102 Guanyu unf
Wusheng = sgs.CreateViewAsSkill{
	name = "wusheng",
	n = 0,
	
	enabled_at_play=function()
		return sgs.Self:canSlashWithoutCrossbow()
	end,
	
	enabled_at_response=function()
		return sgs.Self:getPattern() == "slash"
	end,
	
	view_filter=function(self, selected, to_select)
		local card = to_select:getFilteredCard()
		if(not card:isRed()) then return false end
		
		if(card == Self:getWeapon() and card:objectName() == "crossbow") then
			return Self:canSlashWithoutCrossbow()
		else
			return true
		end
	end,
	
	view_as=function(self, card)
		local filtered=sgs.Sanguosha:cloneCard("slash", card:getSuit(), card:getNumber())
		filtered:addSubcard(card)
		filtered:setSkillName(self:objectName())
		return filtered
	end
}

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
		
	filter = function(self,targets,to_select)
		
		if (not to_select:getGeneral():isMale()) or #targets>1 then return false 
		elseif to_select:hasSkill("kongcheng") and to_select:isKongcheng() and #targets==0 then return false
		else return true end--[[
		if(#targets > 1) then
			return false
		end
		
		if(to_select == self) then return false end
		
		return not to_select:isKongcheng()]]
	end,
		
	on_effect=function(self,effect)
		local from=effect.from
		local to  =effect.to
		local room=to:getRoom()
		local card_id= room:askForCardChosen(from, to, "h", "tuxi")
		local card   = sgs.Sanguosha:getCard(card_id)
		room:moveCardTo(card, from, sgs.Player_Hand, false)
		
		room:setEmotion(to, "bad")
		room:setEmotion(from, "good")
	end,
}

luatuxi_viewas = sgs.CreateViewAsSkill{
	n = 0,
	
	name="luatuxi",
	
	view_as=function(self, card)
		local view_as_card = luatuxi_card:clone()
		
		view_as_card:setSkillName(self:objectName())
		
		return view_as_card
	end,
	
	enabled_at_play=function()
		return false
	end,
	
	enabled_at_response=function()
		return sgs.Self:getPattern()=="@@luatuxi"
	end
}

luatuxi = sgs.CreateTriggerSkill{

	name = "luatuxi",
	
	view_as = luatuxi_viewas,
	
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
			room:askForSkillInvoke(player, "luatuxi")
			if(can_invoke and room:askForUseCard(player, "@@luatuxi", "@luatuxi_card")) then return true end
		return false
		end
		
	end
}
--Guanyu = sgs.General(extension, "guanyu", "shu", 4)
--Guanyu:addSkill(Wusheng)

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
luazhangliao:addSkill(luatuxi)


sgs.LoadTranslationTable{
	["YKStdGeneral"] = "YOKA标准武将",
}
