-- translation for BronzeSeintoPackage

local t = {
	["bronzeseinto"] = "黄金圣斗士", 
	["ao"] = "青", 
	["designer:bronzeseinto"] = "StarmK23",

--seiya星矢  天马星座   青   4
	["seiya"] = "星矢",
	["liuxing"] = "流星",
	[":liuxing"] = "你使用【杀】对装备区有牌的角色造成伤害时，可以+1",
	["huixuan"] = "回旋",
	[":huixuan"] = "觉醒技，回合开始阶段，若你的手牌数大于体力上限，你须摸2张牌或回复1点体力，然后减去1点体力上限，永久获得“人马”和“烈弓”",
	["huixing"] = "彗星",
	[":huixing"] = "限定技，出牌阶段，你可以弃掉一张【杀】，让一名角色进行X次【杀】的结算（X为你损失的体力）",
	["@hx"] = "彗",
	["shenzui"] = "神罪",
	[":shenzui"] = "主公技，觉醒技，回合开始阶段，当你只有1点体力，你须减1点体力上限并永久获得技能“不屈”和“克己”",

--shiryu紫龙    天龙星座    青    4
	["shiryu"] = "紫龙",
	["shenglong"] = "升龙",
	[":shenglong"] = "你的回合内可找一名角色拼点，若你嬴则获得拼点牌和“铁骑”直到回合结束",
	["kanglong"] = "亢龙",
	[":kanglong"] = "限定技，当你觉醒后，可在出牌阶段弃2张不同颜色的牌和一名角色一起进入频死状态。 【若救回紫龙X/2的血（向下去整），目标回复至X，X为亢龙前血量】",
	["showhu"] = "守护",
	[":showhu"] = "主公技，回合开始阶段当你没有手牌你永久获得“苦肉”，“崩坏”和“毅重”技能",

--hyoga冰河      白鸟星座     青      4
	["hyoga"] = "冰河",
	["zuanxing"] = "钻星", 
	[":zuanxing"] = "你的杀造成的伤害你可选择防止此伤害，改为弃置该目标角色的3张牌",
	["jinguang"] = "金光",
	[":jinguang"] = "觉醒技，回合开始阶段，若你的体力为1，你须减1点体力上限，并永久获得技能“曙恕”和“刚烈”",
	["bingqiang"] = "冰墙",
	[":bingqiang"] = "回合开始阶段进行判定，若为黑桃则获得“断粮”技能直到下回合开始，不为黑桃则获得“冰柜”技能直到下回合开始",

--shum瞬     仙女星座    青       3
	["shum"] = "瞬", 
	["suolian"] = "锁链",
	[":suolian"] = "你的杀、闪、铁锁连环可互相转换",
	["xingyun"] = "星云",
	[":xingyun"] = "你可以从牌堆展示5张牌选择X张交给1名角色然后使其武将牌翻面，每回合限1次（X为5-你损失的体力，可以是自己）", 
	["qiliu"] = "气流",
	[":qiliu"] = "锁定技，你的杀对翻面角色的伤害+1，你翻面受到伤害则可将武将牌重置",
	["fengbao"] = "风暴",
	[":fengbao"] = "限定技，出牌阶段弃3种不同花色手牌，然后自减1点体力上限，指定1名角色受到1点火属性伤害1点体力流失",

--ikki一辉      凤凰星座       青        4
	["ikki"] = "一辉", 
	["xuechi"] = "幻魔",
	[":xuechi"] = "你出牌阶段可弃一张红牌指定一名角色对其攻击范围的一名角色出杀，若不此作视为自己对自己造成1点伤害",
	["sixd"] = "凤翼",
	["#SixdBuff"] = "锁定技，你的杀都带有火属性，你的火杀牌造成的伤害+1",
	["jiangmo"] = "重生",
	[":jiangmo"] = "受到1点火属性伤害回复2点体力，当你处于濒死状态时，你可以丢弃你所有的牌和你判定区里的牌，并重置你的武将牌，然后摸三张牌且体力回复至体力上限，然后失去 1点体力上限（体力上限为1不能发动）,

}

local bronze = {"seiya", "shiryu", "ikki", "shum", "hyoga"}

for _, seinto in ipairs(bronze) do
	t["designer:" .. seinto] = t["designer:bronzeseinto"]
end

return t