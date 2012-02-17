-- translation for PurplePackage

return {
	["purple"] = "★玉女★",

--WEI08 纤纤素手 3
	["purpleyelai"] = "夜来",
	["designer:purpleyelai"] = "安歧大小姐",
	["zhuosu"] = "擢素",
	[":zhuosu"] = "你每受到一次伤害，可依以下效果执行：\
  1.若你的武将牌正面向上，指定一角色获得一个额外的回合；\
  2.若你的武将牌背面向上，指定一角色武将牌翻面。",
	["difu"] = "帝服",
	[":difu"] = "当你濒死时，可指定一名角色摸X张牌，X为你所拥有的牌的数目，之后你立即死亡。",
	["bui"] = "不衣",
	["bui"] = "锁定技，杀死你的角色失去装备区直到游戏结束。",

--SHU#08 4
	["purpleguanfeng"] = "关凤",
	["designer:purpleguanfeng"] = "东风",
	["fengnv"] = "凤女",
	[":fengnv"] = "出牌阶段，你可以指定一名角色，若该角色装备区不为空，则目标角色需选择弃置X张牌，X为其装备的数量。其每少弃置1张牌则失去一点体力。",

--SHU#09 将门皇后 4
	["purplexingcai"] = "星彩",
	["designer:purplexingcai"] = "sailiss",
	["xiannei"] = "贤内",
	[":xiannei"] = "当一名手牌数小于手牌上限的角色失去牌后，你可以弃置一张牌，令其回复1点体力或摸两张牌。任意角色回合内限一次。",

--SHU#10 皇思夫人 4 暂定
	["purpleganfuren"] = "甘夫人",
	["designer:purpleganfuren"] = "sys",
	["neizhu"] = "内助",
	[":neizhu"] = "其他角色在其回合内除使用或打出手牌而失去手牌时，你可以立即将其手牌数补充到体力上限，每回合限用一次",
	["zijin"] = "自尽",
	[":zijin"] = "觉醒技，回合开始阶段，若你的体力值为一，你可以令场上存活的一名角色立即摸三张牌，获得技能“毅重”“咆哮”。然后你立即死亡",

--QUN#06 4
	["purplezoushi"] = "邹氏",
	["designer:purplezoushi"] = "镝木琉星",
	["shangjue"] = "伤绝",
	[":shangjue"] = "当你在回合外失去最后一张手牌时，你可以立即摸两张牌并对任意一名角色造成1点伤害。",
	["quling"] = "屈凌",
	[":quling"] = "觉醒技，回合开始阶段，若除你以外没有群雄武将在场，你须减1点体力上限，势力变更为魏，并永久获得技能“祸水”（当你进入濒死状态时，你可以选择立即死亡。若如此做，所有男性角色失去一点体力。）",

--WU#06 
	["greenchenwu"] = "陈武",
	["designer:greenchenwu"] = "曉ャ絕對",
	["qilin"] = "麒麟",
	[":qilin"] = "回合开始阶段，你可令一名其他角色将手牌弃至或补至等同于你当前拥有的手牌数，若补或弃的牌不少于三张，将你的武将牌翻面",

--WU#07 
	["greenmayunlu"] = "马云禄",
	["designer:greenmayunlu"] = "安歧大小姐",
	["jinguo"] = "巾帼",
	[":jinguo"] = "回合开始阶段，你可以弃一张牌并指定一名角色，直到你的下回合开始，该角色成为【杀】的目标时，须交给你一张【杀】才能抵消",
	["@jinguo"] = "你可以弃一张牌并指定一名角色，该角色获得一个巾帼标记",
	["@jin"] = "巾帼",
	["#JinguoEffect"] = "%to 受到【%arg】的影响，必须交出一张【杀】来响应 %from 的【杀】",
	["jinguo-jink"] = "%src 使用了【杀】，但你受到【巾帼】影响，必须交出一张【杀】视作【闪】",
	["wuqi"] = "武骑",
	[":wuqi"] = "回合外，任一角色的判定结果为红色时，你可以摸一张牌并对攻击范围内的一名角色使用一张【杀】",

--QUN#07 治世之才
	["greenchenqun"] = "陈群",
	["designer:greenchenqun"] = "司马子上",
	["quanyi"] = "劝议",
	[":quanyi"] = "出牌阶段，你可以弃一张牌并指定两名角色，前者摸取与你的体力值相等的牌数(不能超过3张)，直到你下个回合开始阶段对后者使用的【杀】和非延时锦囊(唯一目标)无效，每阶段限一次",
	["zhunsuan"] = "准算",
	[":zhunsuan"] = "回合结束阶段，你可以声明一个数字，直到你下个回合开始，其他角色使用或打出的基本牌进入弃牌堆时，若其点数和你声明的数字之差小于2，你获得这张牌",
	["#Zhunsuan"] = "%from 声明的数字是 %arg",
	["zhunsuan:back"] = "大了",
	["zhunsuan:next"] = "小了",

	["cv:greenyanpeng"] = "",
	["cv:greencaozhang"] = "",
	["cv:greenjushou"] = "",
	["cv:greenkanze"] = "",
	["cv:greenwanglang"] = "",
	["cv:greenchenwu"] = "",
	["cv:greenmayunlu"] = "",
	["cv:greenchenqun"] = "",
}
