-- translation for PurplePackage

return {
	["purple"] = "★玉女★",

--WEI08 纤纤素手 3
	["purpleyelai"] = "夜来",
	["designer:purpleyelai"] = "安歧大小姐",
	["zhuosu"] = "擢素",
	[":zhuosu"] = "你每受到一次伤害，可依以下效果执行：\
  1.若你的武将牌正面向上，指定一名其他角色在本回合结束时获得一个额外的回合；\
  2.若你的武将牌背面向上，指定一名其他角色武将牌翻面。\
  之后，你将自己的武将牌翻面。",
	["difu"] = "帝服",
	[":difu"] = "当你濒死时，可指定一名角色摸X张牌，X为你所拥有的牌的数目，之后你立即死亡。",
	["bui"] = "不衣",
	[":bui"] = "锁定技，杀死你的角色失去装备区直到游戏结束。",
	["@noequip"] = "不衣",

--WU#06 美姿容颜 3
	["purplesunnichang"] = "孙霓裳",
	["designer:purplesunnichang"] = "Dean",
	["wuxian"] = "五弦",
	[":wuxian"] = "其他角色在其回合外获得手牌时，你可以摸一张牌；当你的手牌数大于5张时，你须弃置部分手牌以保证手牌数与体力上限相等。",
	["xianming"] = "弦鸣",
	[":xianming"] = "每当你使用【过河拆桥】或【顺手牵羊】或成为该锦囊目标时，（在结算前），你可以将两种效果互换。\
★即过拆变顺牵，顺牵变过拆。因为目标已指定，所以可破谦逊。",

--SHU#09 将门皇后 4
	["purplexingcai"] = "星彩",
	["designer:purplexingcai"] = "sailiss",
	["xiannei"] = "贤内",
	[":xiannei"] = "当一名手牌数小于手牌上限的角色失去牌后，你可以弃置一张牌，令其回复1点体力或摸两张牌。任意角色回合内限一次。",
	["xiannei:draw"] = "摸两张牌",
	["xiannei:hp"] = "回复1点体力",

--SHU#10 墨衣魅影 3
	["purpleduanmeng"] = "端蒙",
	["designer:purpleduanmeng"] = "安岐大小姐",
	["poxie"] = "破邪",
	[":poxie"] = "摸牌阶段，你可以少摸一张牌并指定一名角色，该回合的出牌阶段你使用的第一张基本牌或锦囊牌在结算前，指定角色须弃掉一张与你使用的牌同花色的手牌，否则你对其造成一点伤害。",
	["@poxie"] = "%src 要你弃掉一张花色为 %arg 的手牌，否则会受到伤害",
	["xiangfeng"] = "翔凤",
	[":xiangfeng"] = "当你使用【闪】抵消【杀】时，你可以立刻获得那张【杀】。",
	["fuchou"] = "父仇",
	[":fuchou"] = "关联技，锁定技，你立即获得任意一名角色因【挥泪】技能弃置的所有牌。",

--QUN#06 无称号 4
	["purplezoushi"] = "邹氏",
	["designer:purplezoushi"] = "镝木琉星",
	["shangjue"] = "伤绝",
	[":shangjue"] = "当你在回合外失去最后一张手牌时，你可以立即摸两张牌或对任意一名角色造成1点伤害。",
	["shangjue:draw"] = "摸两张牌",
	["shangjue:damage"] = "对任意一名角色造成1点伤害",
	["quling"] = "屈凌",
	[":quling"] = "觉醒技，回合开始阶段，若除你以外没有群雄武将在场，你须减1点体力上限，势力变更为魏，并永久获得技能“祸水”（当你进入濒死状态时，你可以令所有男性角色失去一点体力，然后立即死亡。）",
	["huoshui"] = "祸水",
	[":huoshui"] = "当你进入濒死状态时，你可以令所有男性角色失去一点体力，然后立即死亡。",

--WU#07 大懿皇后 3
	["purplewangmengru"] = "王贵人",
	["designer:purplewangmengru"] = "嘻哈绿豆沙",
	["chongdu"] = "宠妒",
	[":chongdu"] = "锁定技，当你受到一次伤害，结算后从场上每名男性角色处获得一张牌，然后每名其他女性视为对你使用一张杀。",
	["chansha"] = "谗杀",
	[":chansha"] = "锁定技，女性角色对你造成的伤害均视为体力流失。",

--SHU#08 文武双精 4
	["purplebao3niang"] = "鲍三娘",
	["designer:purplebao3niang"] = "Dave杰",
	["xiayi"] = "侠义",
	[":xiayi"] = "每当一名男性角色受到其他角色造成的伤害时，你可以弃置一张【杀】，视为该角色对伤害来源使用了一张【杀】。",
	["@xiayi"] = "你可以弃置一张【杀】，视为伤害目标对伤害来源使用了一张【杀】。",

--QUN#07 东瀛夷主 3
	["purplebeimihu"] = "卑弥呼",
	["designer:purplebeimihu"] = "幻象迷宫",
	["nvwang"] = "女王",
	[":nvwang"] = "锁定技，你的手牌上限永远等于场上女性角色数量+1；你的摸牌阶段摸牌数永远等于场上男性角色数量+1（最多4张）。",
	["shouguo"] = "授国",
	[":shouguo"] = "出牌阶段，可随机获得一名已受伤男性角色的两张手牌，若如此做，该角色回复一点体力。",
	["yaofa"] = "妖法",
	[":yaofa"] = "限定技，弃牌阶段，你可以弃置你所有的牌，并指定场上一名角色弃置相同数量的牌。",

	["cv:purpleyelai"] = "",
	["cv:purplesunnichang"] = "",
	["cv:purplexingcai"] = "",
	["cv:purpleduanmeng"] = "",
	["cv:purplezoushi"] = "",
	["cv:purplewangmengru"] = "",
	["cv:purplebao3niang"] = "",
	["cv:purplebeimihu"] = "",
}
