--大家好我是hypercross。
--从这个文件开始讲解DIY接口的用法。

--首先，这个文件说明DIY需要的文件及其结构。

--DIY是以module的形式存在的。每个Module即是一个UTF8格式的Lua文件（建议用notepad++编辑），包含如下格式的代码：

module("extensions.omegatan", package.seeall)  -- 进入module。这里moligaloo这个词必须和文件名相同。

extension = sgs.Package("omegatan")            -- 创建扩展包对象。变量名必须为extension。参数名为扩展包的objectName，也是通常会使用的扩展包标识

luaqianshui = sgs.CreateTriggerSkill
{ --创建技能，技能种类为ViewAsSkill。 
	frequency = sgs.Skill_Compulsory,
	name = "luaqianshui",
	n = 1,
	events={sgs.CardUsed,sgs.CardEffected},
	
	on_trigger=function(self,event,player,data)
		local room = player:getRoom()
		local effect = data:toCardEffect()
		if(effect.card:inherits("Duel") or (effect.card:inherits("Slash") and effect.card:isRed())) then
			if(effect.to:hasMark("shangfu")) then return false
			else return true-- if(effectto_hasSkill(_objectName())) then
			end
			end-- end
		return false
	end
}--关于技能的说明将是几乎所有其他帮助文件的重点。此处省略。

--[[niubi = sgs.CreateTriggerSkill
{
	name = "niubi",
	events={sgs.PhaseChange},
		
	on_trigger=function(self,event,player,data)
		local room=player:getRoom()
		local players=room:getAlivePlayers()
		
		if(player:getPhase() == sgs.Player_Play) then
			if(room:askForSkillInvoke(player, "niubi")) then
				room:askForDiscard(player, "niubi", (room:getAlivePlayers().length()+1)/2, false, true)
				player:gainMark("niubi");
			end
			return false;
		else if(player:getPhase() == sgs.Player_Finish) then
			if(player:getMark("niubi")) then
				while(room:askForSkillInvoke(player, "niubi") == true) do
                    room:swapSeat(room:askForPlayerChosen(player, room:getOtherPlayers(luaubun), "niubi"),room:askForPlayerChosen(player, room:getOtherPlayers(luaubun), "niubi"))
                    player:loseAllMarks("niubi");
				end
			end
		end
		end
	end,				
}]]

luazhongpao = sgs.CreateTriggerSkill
{
	frequency = sgs.Skill_Compulsory,
	name = "luazhongpao",
	events={sgs.Predamage},
	
	on_trigger=function(self,event,player,data)
		local room = player:getRoom()
		local damage = data:toDamage()
		local reason = damage.card
		if not reason then return end
		if (reason:inherits("Slash")==true) then
			--[[local log=sgs.LogMessage()
				log.type = "#luazhongpao"
				log.from = player:getGeneralName()
				log.to = damage.to:getGeneralName()
				log.arg = damage.damage
				log.arg2 = damage.damage + 1]]
				damage.damage = damage.damage + 1;
				--room:sendLog(log);
				data:setValue(damage);
		end
	end
}

luaguisu = sgs.CreateTriggerSkill
{
	frequency = sgs.Skill_Compulsory,
	name = "luaguisu",
	events = {sgs.CardEffect},
	
	on_trigger=function(self,event,player,data)
		local room = player:getRoom()
		local effect = data:toCardEffect()
		
		if(effect.card:inherits("Slash")) then
			
		end
	end
}
luaibicdlcod = sgs.General(extension, "luaibicdlcod", "god", 4, true) -- 创建武将对象。关于武将属性的详细说明见reference文档。
luaibicdlcod:addSkill(luaqianshui) --因上浮无法实现，MOD不予考虑(既然现在有mark，可以直接写成锁定技啊。。。DT，以后再说)

--luaubun = sgs.General(extension, "luaubun", "god", 4, true)
--luaubun:addSkill(niubi)  --顶座无法实现

luatenkei = sgs.General(extension, "luatenkei", "god", 3, false)
luatenkei:addSkill(luazhongpao)

sgs.LoadTranslationTable{
	["omegatan"] = "坛",
	["luaibicdlcod"] = "圆月",
	["designer:luaibicdlcod"] = "luaibicdlcod",
	["cv:luaibicdlcod"] = "",
	["!luaibicdlcod"] = "3112",
	["luaqianshui"] = "潜水",
	[":luaqianshui"] = "红色的【杀】和【决斗】对你无效",
	
	["luatenkei"] = "天启",
	["designer:luatenkei"] = "luaibicdlcod",
	["cv:luatenkei"] = "",
	["!luatenkei"] = "3136",
}--此段为翻译，将技能名称与描述中文化

 --赋予武将技能。

--你可以将本文件保存至extension目录下的moligaloo.lua并启动游戏。此时扩展包即已经被添加至游戏。

--为了完善DIY扩展包，需要将音频、图片以及翻译代码放到指定目录。这一点将在其他文档中说明。