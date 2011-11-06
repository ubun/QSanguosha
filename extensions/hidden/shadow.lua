module("extensions.shadow", package.seeall)

extension = sgs.Package("shadow")

huzhu_main=sgs.CreateTriggerSkill{

	name="huzhu_main",

	events={sgs.PhaseChange,sgs.Damaged},

	on_trigger=function(self,event,player,data)
		local room=player:getRoom()
		local players=room:getAlivePlayers()

		if event==sgs.PhaseChange then
			if player:hasSkill("huzhu_main") and player:getPhase()==sgs.Player_Finish then

				if not room:askForSkillInvoke(player,"huzhu_main") then return end
				 local target=room:askForPlayerChosen(player, players, "huzhu_main");
				local log=sgs.LogMessage()
				log.type = "#huzhu_selected"
				log.arg = target:getGeneralName()
				log.arg2 = player:getGeneralName()
				room:sendLog(log);
				room:playSkillEffect("huzhu_main")

				 local data=sgs.QVariant(0)
				 data:setValue(player)

				 target:setTag("huzhu_source",data)
			elseif player:hasSkill("huzhu_main") and player:getPhase()==sgs.Player_Start then
				for _,aplayer in sgs.qlist(players) do
					if aplayer:getTag("huzhu_source"):toPlayer() then
						aplayer:setTag("huzhu_source",sgs.QVariant(false))
					end
				end
			end
		elseif event==sgs.Damaged and player:getTag("huzhu_source"):toPlayer() then
			room:output("aaaa")
			if player:isDead() then return false end
			local caohong=player:getTag("huzhu_source"):toPlayer()
			if caohong:isDead() then return false end

			local log=sgs.LogMessage()
			log.type = "#huzhu_effected"
			log.arg = caohong:getGeneralName()
			room:sendLog(log);
			room:playSkillEffect("huzhu_main")



			room:loseHp(caohong,1)

			local recover=sgs.RecoverStruct()
			recover.who=player
			recover.recover=1
			room:recover(player,recover)
		end
	end,

	can_trigger=function(self,target)
		return true
	end,
}

huitian=sgs.CreateTriggerSkill{

	name="huitian",

	events={sgs.HpLost},

	on_trigger=function(self,event,player,data)
		local log=sgs.LogMessage()
		log.type = "#huitian"
		log.arg = player:getGeneralName()
		player:getRoom():sendLog(log);

		player:drawCards(player:getLostHp())
	end,
}

yinlang_card=sgs.CreateSkillCard{

	name="yinlang_card",

	target_fixed=false,

	will_throw=false,

	filter=function(self,targets,to_select)
		return not (#targets>0
					or to_select:objectName()==sgs.Self:objectName()
					or to_select:getHandcardNum()<sgs.Self:getHandcardNum()
					)
	end,

	on_effect=function(self,effect)
		local room=effect.to:getRoom()

		room:playSkillEffect("yinlang")

		local card_id=room:askForCardChosen(effect.from,effect.to,"hej",self:objectName())


		if room:getCardPlace(card_id)==sgs.Player_Hand then
			room:moveCardTo(sgs.Sanguosha:getCard(card_id),effect.from,sgs.Player_Hand,false)
		else
			room:obtainCard(effect.from,card_id)
		end


		room:setPlayerFlag(effect.from,"yinlang_used")
	end
}

yinlang_main=sgs.CreateViewAsSkill{

	name="yinlang",

	n=0,

	view_filter=function()
		return false
	end,

	view_as=function()
		return yinlang_card:clone()
	end,

	enabled_at_play=function()
		return not sgs.Self:hasFlag("yinlang_used")
	end
}

renluan=sgs.CreateTriggerSkill{

	name="renluan",

	events=sgs.Damaged,

	on_trigger = function(self,event,player,data)
		local room=player:getRoom()

		if not room:askForSkillInvoke(player,self:objectName()) then return end

		local judge=sgs.JudgeStruct()
		judge.pattern=sgs.QRegExp("(.*)")
		judge.good=true
		judge.reason=self:objectName()
		judge.who=player

		room:judge(judge)
		local suit=judge.card:getSuitString()
		suit="."..suit:sub(1,1):upper()

		local card=room:askForCard(player,suit,"@renluan:"..suit)



		if not card then room:playSkillEffect("renluan",2) return end

		local recover=sgs.RecoverStruct()
		recover.who=player
		recover.recover=1
		room:recover(player,recover)

		room:playSkillEffect("renluan",1)

	end
}

guaduan=sgs.CreateTriggerSkill{

	name="guaduan",

	events=sgs.CardLost,

	on_trigger = function(self,event,player,data)
		local room=player:getRoom()
		local move=data:toCardMove()

		if not move.to then return end
		if move.from:objectName()==move.to:objectName() then return end
		if move.to_place==sgs.Player_Judging then return end

		if move.to:hasSkill(self:objectName()) and
		(move.from_place==sgs.Player_Hand or
		 move.from_place==sgs.Player_Judging or
		 move.from_place==sgs.Player_Equip) then

			if move.to:getHp()>=move.from:getHp() then
				room:playSkillEffect("guaduan")

				local damage=sgs.DamageStruct()
				damage.damage=1
				damage.from=move.from
				damage.to=move.to
				damage.nature=sgs.DamageStruct_Normal
				damage.chain=false
				room:damage(damage)

				local log=sgs.LogMessage()
				log.type = "#guaduan"
				log.arg = player:getGeneralName()
				room:sendLog(log);
			end

		 end

	end,

	can_trigger=function()
		return true
	end
}

YJshangshi=sgs.CreateTriggerSkill{

	name="YJshangshi",

	events={sgs.Damaged,sgs.CardLost,sgs.HpLost,sgs.PhaseChange},

	priority=-1,

	frequency=sgs.Skill_Frequent,

	on_trigger=function(self,event,player,data)
		local room=player:getRoom()
		if player:getLostHp()<=player:getHandcardNum() then return end
		if player:getPhase()==sgs.Player_Discard then return end
		if (event==sgs.PhaseChange) and (player:getPhase()==sgs.Player_Finish) then
			if not room:askForSkillInvoke(player,self:objectName()) then return end
			player:drawCards(player:getLostHp()-player:getHandcardNum())
		end

		local move=data:toCardMove()
		if move and not move.from_place==sgs.Player_Hand then return end


		if not room:askForSkillInvoke(player,self:objectName()) then return end

		local log=sgs.LogMessage()
		log.type ="#YJshangshi"
		log.arg  =player:getGeneralName()
		room:sendLog(log)

		player:drawCards(player:getLostHp()-player:getHandcardNum())
	end,
}

YJjueqing=sgs.CreateTriggerSkill{

	name="YJjueqing",
	events={sgs.Predamage},
	frequency=sgs.Skill_Compulsory,
	on_trigger=function(self,event,player,data)
		local room=player:getRoom()
		local damage=data:toDamage()

		local log=sgs.LogMessage()
		log.type ="#YJjueqing"
		log.arg  =player:getGeneralName()
		room:sendLog(log)

		room:loseHp(damage.to,damage.damage)
		return true
	end,
}

qingman=sgs.CreateTriggerSkill{
	name="qingman",
	events={sgs.CardEffected},
	frequency=sgs.Skill_Frequent,
	on_trigger=function(self,event,player,data)
		local effect=data:toCardEffect()
		local room=player:getRoom()

		if not effect.card:isNDTrick() then return end
		if not effect.card:isBlack() then return end
		if not room:askForSkillInvoke(player,self:objectName()) then return end

		local log=sgs.LogMessage()
		log.type ="#qingman"
		log.arg  =player:getGeneralName()
		room:sendLog(log)

		player:drawCards(1)
	end
}

fange_card=sgs.CreateSkillCard{

	name="fange_card",

	target_fixed=false,

	will_throw=false,

	filter=function(self,targets,to_select)
		return not (#targets>0
					or to_select:objectName()==sgs.Self:objectName()
					or to_select:isKongcheng()
					)
	end,

	on_effect=function(self,effect)
		local room=effect.to:getRoom()

		opponent=function(player)
			if player:objectName()==effect.to:objectName() then
				return effect.from
			else
				return effect.to
			end
		end

		room:playSkillEffect("fange")

		local suit="."

		local target=effect.from
		local card

		repeat
			target= opponent(target)
			card=room:askForCard(target,suit,"@fange:"..suit)
			if card then
				suit=card:getSuitString()
				suit="."..suit:sub(1,1):upper()
			end
		until not card

		local damage=sgs.DamageStruct()
				damage.damage=1
				damage.from=opponent(target)
				damage.to=target
				damage.nature=sgs.DamageStruct_Normal
				damage.chain=false
				room:damage(damage)
		room:setPlayerFlag(effect.from,"fange_used")
	end
}

fange=sgs.CreateViewAsSkill{
	name="fange",

	n=0,

	view_filter=function()
		return false
	end,

	view_as=function()
		return fange_card:clone()
	end,

	enabled_at_play=function()
		return not sgs.Self:hasFlag("fange_used")
	end
}

xiaoyong=sgs.CreateTriggerSkill{

	name="xiaoyong",
	events={sgs.Damage},
	on_trigger=function(self,event,player,data)
		local damage=data:toDamage()
		local room=player:getRoom()

		if not damage.card then return end
		if damage.to:isDead() then return end
		if damage.to:getHandcardNum()<=damage.to:getHp() then return end
		if not room:askForSkillInvoke(player,self:objectName()) then return end

		room:askForDiscard(damage.to,self:objectName(),damage.to:getHandcardNum()-damage.to:getHp(),false,false)
	end
}

tunwu_slash=sgs.CreateSkillCard{
	name="tunwu_slash",

	target_fixed=false,

	will_throw=true,

	filter=function(self,targets,to_select)
		return sgs.Self:canSlash(to_select,false) and #targets<1
	end,

	on_effect=function(self,effect)
		local slash = sgs.Sanguosha:cloneCard("slash",sgs.Card_NoSuit, 0)
		slash:setSkillName(self:objectName())
		local use=sgs.CardUseStruct()
		use.card = slash
		use.from = effect.from
		local sp=sgs.SPlayerList()
		sp:append(effect.to)
		use.to = sp

		effect.to:getRoom():useCard(use,false)
	end
}

tunwu_viewas=sgs.CreateViewAsSkill{
	name = "tunwu_viewas",
	n = 2,
	view_filter = function(self, selected, to_select)
		return to_select:inherits("Analeptic")
	end,

	view_as = function(self, cards)
		if #cards == 2 then
			local new_card =tunwu_slash:clone()
			new_card:addSubcard(cards[1]:getEffectiveId())
			new_card:addSubcard(cards[2]:getEffectiveId())
			return new_card
		end
	end
}

tunwu=sgs.CreateFilterSkill{
	name="tunwu",
	view_filter=function(self,to_select)
		return to_select:getSuit()==sgs.Card_Diamond
	end,
	view_as=function(self,card)
		local filtered=sgs.Sanguosha:cloneCard("analeptic", card:getSuit(), card:getNumber())
		filtered:addSubcard(card)
		filtered:setSkillName(self:objectName())
		return filtered
	end
}

siying=sgs.CreateTriggerSkill{
	name="siying",
	events={sgs.Predamaged},
	frequency=sgs.Skill_Compulsory,
	on_trigger=function(self,event,player,data)
		local damage=data:toDamage()
		if damage.nature==sgs.DamageStruct_Fire then
			if player:getHandcardNum()>1 then
				if player:getRoom():askForDiscard(player,self:objectName(),2,true,false) then return end
				damage.damage=damage.damage+1
				data:setValue(damage)
			end
		end
	end
}

Ybawang_card=sgs.CreateSkillCard{

name="Ybawang_card",

target_fixed=false,

will_throw=false,

filter=function(self,targets,to_select)

	if #targets>=sgs.Self:getLostHp()-1 then return false end

	if to_select:hasFlag("Ybawang_target") then return false end

	return sgs.Self:canSlash(to_select,true)
end,

on_effect=function(self,effect)
	effect.to:getRoom():setPlayerFlag(effect.to,"Ybawang_target")
end

}

Ybawang_viewas=sgs.CreateViewAsSkill{
	name = "Ybawang_viewas",
	n = 0,
	view_filter = function()
		return false
	end,

	view_as = function()
		return Ybawang_card:clone()
	end,

	enabled_at_play=function()
		return false
	end,

	enabled_at_response=function()
		return sgs.Self:getPattern()=="#Ybawang_card"
	end
}

Ybawang=sgs.CreateTriggerSkill{
	name="Ybawang",
	view_as_skill=Ybawang_viewas,
	events={sgs.SlashMissed},
	priority=2,
	on_trigger=function(self,event,player,data)
		local room=player:getRoom()
		local effect=data:toSlashEffect()
		if effect.to:isKongcheng() or player:isKongcheng() then return end
		if not room:askForSkillInvoke(player,self:objectName()) then return end

		local isGood=player:pindian(effect.to,self:objectName(),nil)

		if isGood then
			if player:getLostHp()<1 then return end
			room:setPlayerFlag(effect.to,"Ybawang_target")
			room:askForUseCard(player,"#Ybawang_card","#Ybawang_card:"..effect.to:objectName())

			local players=room:getOtherPlayers(player)

			local targets=sgs.SPlayerList()

			for _,aplayer in sgs.qlist(players) do
				if aplayer:hasFlag("Ybawang_target") then
					room:setPlayerFlag(aplayer,"-Ybawang_target")
					targets:append(aplayer)
				end
			end

			local slash = sgs.Sanguosha:cloneCard("slash",sgs.Card_NoSuit, 0)
			slash:setSkillName(self:objectName())
			local use=sgs.CardUseStruct()
			use.card = slash
			use.from = player
			use.to=targets

			room:useCard(use,false)
		end
	end
}

zeifen = sgs.CreateTriggerSkill{

frequency = sgs.Skill_Frequent,
name = "zeifen",
events={sgs.DrawNCards},
on_trigger=function(self,event,player,data)
local room=player:getRoom()
if (event==sgs.DrawNCards) then
     if player:getLostHp()>0 and player:getLostHp()<4 then
     room:playSkillEffect("zeifen")
     data:setValue(data:toInt()+player:getLostHp())
     elseif player:getLostHp()>3 then
     room:playSkillEffect("zeifen")
     data:setValue(data:toInt()+3)
end
end
end}

jietuo = sgs.CreateTriggerSkill{
        frequency = sgs.Skill_Frequent,
        name      = "jietuo",
        events={sgs.Predamage},
        on_trigger = function(self,event,player,data)
                        local room = player:getRoom()

                        local damage = data:toDamage()
                        if (damage.card:inherits("Slash")~=true) then
                                return
                        end

                        if (room:askForSkillInvoke(player,"jietuo")~=true) then
                                return
                        end

                        local judge=sgs.JudgeStruct()
                        judge.pattern=sgs.QRegExp("(.*):(heart|diamond):(.*)")
                        judge.good=true
                        judge.reason="jietuo"
                        judge.who=player
                        room:judge(judge)

                        if judge:isGood() then
                                 damage.damage = damage.damage + 1
                                 data:setValue(damage)
                        end

        end
}

--详细文章参考：http://qsanguosha.com/forum.php?mod=viewthread&tid=2045&highlight=%B4%FA%C2%EB

liuzhang=sgs.General(extension, "liuzhang", "qun", 3)
liuzhang:addSkill(yinlang_main)
liuzhang:addSkill(renluan)
liuzhang:addSkill(guaduan)

caohong=sgs.General(extension, "caohong", "wei")
caohong:addSkill(huzhu_main)
caohong:addSkill(huitian)

xuyou=sgs.General(extension,"xuyou","qun",3)
xuyou:addSkill(fange)
xuyou:addSkill(qingman)

yuejin=sgs.General(extension,"yuejin","wei")
yuejin:addSkill(xiaoyong)

YJzhangchunhua=sgs.General(extension,"YJzhangchunhua","wei",3,false)
YJzhangchunhua:addSkill(YJjueqing)
YJzhangchunhua:addSkill(YJshangshi)

shenliubei=sgs.General(extension,"shenliubei","shu",6)
shenliubei:addSkill(tunwu)
shenliubei:addSkill(siying)
shenliubei:addSkill(tunwu_viewas)

Ysunce=sgs.General(extension,"Ysunce","wu")
Ysunce:addSkill(Ybawang)

sgs.LoadTranslationTable{
	["shadow"] = "阴包",
	
	["!Ysunce"] = "3401",
	["!xuyou"] = "3402",
	["!yuejin"] = "3403",
	["!shenliubei"] = "3491",
	["!YJzhangchunhua"] = "2211",
	["!caohong"] = "3404",
	["!liuzhang"] = "3405",

	--孙策
	["Ysunce"]="孙策",
	["Ybawang"]="霸王",
	[":Ybawang"]="你的杀被闪避时，你可以与目标拼点；若你赢，则视为你对包含目标在内的最多X名角色使用了杀。x为你已损失的体力值。",
	["#Ybawang_card"]="你可以另外指定x-1名角色，x为你已损失的体力值",

	--许攸
	["xuyou"]="许攸",
	["fange"]="反戈",
	[":fange"]="出牌阶段，你可指定一名玩家打出一张手牌；然后从你开始，你和他依次打出1张与该牌同花色的手牌。首先不如此做者受到对方的一点伤害。",
	["qingman"]="轻慢",
	[":qingman"]="每当你成为黑色非延时锦囊牌的目标，你摸1张牌",
	["designer:xuyou"] = "hypercross",

	--乐进
	["yuejin"]="乐进",
	["xiaoyong"]="骁勇",
	[":xiaoyong"]="你的杀命中时，若目标的手牌数多于生命值，你可令其将手牌弃至生命值的张数。",
	["designer:yuejin"] = "hypercross",

	--神刘备
	["shenliubei"]="神刘备",
	["tunwu"]="吞吴",
	["tunwu_viewas"]="吞吴杀",
	[":tunwu_viewas"]="你可将两张酒当做杀打出。",
	[":tunwu"]="锁定技，你的方片牌都视为酒。",
	["siying"]="死营",
	[":siying"]="锁定技，每当你受到火焰伤害，你需弃两张手牌，否则该伤害+1",
	["designer:shenliubei"] = "jjmyc1|hypercross",

	--张春华
	["YJzhangchunhua"]="张春华",
	["YJshangshi"]="伤逝",
	[":YJshangshi"]="锁定技，若你已受伤,当你的手牌数小于你损失的体力值时，立即将手牌补至你损失的体力值张。",
	["YJjueqing"]="绝情",
	[":YJjueqing"]="锁定技，你将要造成的所有伤害均改为流失体力。",

	--曹洪
	["$huzhu_main1"] = "让领导先走",
	["$huzhu_main2"] = "不能打架，不能打架",
	["caohong"] = "曹洪",
	["~caohong"] = "气死偶咧！！！",
	["huzhu_main"] = "护主",
	[":huzhu_main"] = "你的回合结束阶段，你指定一名玩家。直到你的下回合开始，该玩家受到伤害时你需流失1点体力并令该玩家回复一点体力。",
	["#huzhu_selected"] = "%arg2指定了%arg为【护主】的对象",
	["#huzhu_effected"]= "%arg的技能【护主】被触发",
	["huitian"] = "回天",
	[":huitian"]= "每当你流失体力，你可在体力流失前摸x张牌，x为你损失的体力值。",
	["#huitian"]= "%arg的技能【回天】被触发",
	["cv:caohong"] = "油库里",
	["designer:caohong"] = "qwtyd|hypercross",

	--刘璋
	["liuzhang"] = "刘璋",
	["~liuzhang"] = "宅死啦",
	["renluan"] = "仁乱",
	[":renluan"] = "每当你受到伤害，你可以进行一次判定；如果你能弃掉一张与判定牌同花色的手牌，则你回复1点体力。",
	["$renluan1"]="爷练过哦",
	["$renluan2"]="木有啊有木有",
	["@renluan"]="请弃掉一张与判定牌同花色的手牌",
	["yinlang"] = "引狼",
	[":yinlang"] = "出牌阶段，你可以从任意手牌数不少于你的玩家处获得一张牌。",
	["$yinlang"]="（四川话）再来一张",
	["guaduan"] = "寡断",
	[":guaduan"] = "锁定技，每当你从其他角色处获得牌，若该角色的生命值不多于你，该角色对你造成1点伤害。",
	["#guaduan"] = "%arg 的技能【寡断】被触发",
	["$guaduan"]="你妹啊",
	["cv:liuzhang"] = "油库里",
	["designer:liuzhang"] = "长孙玄武|hypercross",
}
