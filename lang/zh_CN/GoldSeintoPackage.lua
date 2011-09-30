-- translation for GoldSeintoPackage

local t = {
	["goldseinto"] = "黄金圣斗士", 
	["st"] = "圣", 
	["designer:goldseinto"] = "StarmK32",

--mu穆—3血—圣——白羊星座
	["mu"] = "穆",
	["xiufu"] = "修复",
	[":xiufu"] = "出牌阶段，你可以弃一张红桃牌为自己回复一点体力，每阶段限一次；当有角色濒死求桃时，你可以弃掉一张红桃手牌，解除其濒死状态并将其体力设为1",
	["@xiufu"] = "你可以弃掉一张红桃牌进行修复",
	["#Xiufu"] = "%from 发动了技能【%arg】，将 %to 的体力设为了 <b>1</b>",
	["xingmie"] = "星灭",
	[":xingmie"] = "限定技，当你造成伤害后可以弃1张黑桃手牌让不在你攻击范围的所有角色受到1点同属性伤害",
	["@xm"] = "灭",
	["$xingmie"] = "--星灭--",
	["xingmie-ask"] = "你可以发动【星灭】：弃1张黑桃手牌让不在你攻击范围的所有角色受到1点同属性伤害（限定技）",

--aldebaran阿鲁迪巴—4血——圣—金牛星座
	["aldebaran"] = "阿鲁迪巴",
	["hao2jiao"] = "号角",
	[":hao2jiao"] = "当你使用【杀】指定一名角色为目标后，可以进行判定，若结果为红色，此【杀】不可被闪避，且当结果为方块时，伤害+1，为红桃时，你获得判定牌",

--saga撒加——4血——圣——双子星座
	["saga"] = "撒加",
	["huanlong"] = "幻胧", 
	[":huanlong"] = "出牌阶段，你可以与一名角色拼点，若你赢，目标角色对其攻击范围内，你指定的另一名角色（可以是被幻胧的角色本身）造成1点伤害。若你没赢，他/她对你造成1点伤害。每阶段限一次",
	["yinhe"] = "银河",
	[":yinhe"] = "限定技，出牌阶段，你可以弃三枚星标记让一名角色失去1点体力上限",
	["@yinh"] = "银",
	["$yinhe"] = "--银河--",
	["mohua"] = "魔化",
	[":mohua"] = "主公技，出牌阶段，可以要求一名其他圣势力角色对你攻击范围内的一个目标使用任意数量的杀(视为你使用)，造成的伤害+1。回合结束后你们俩乖乖地将自己武将牌翻面",
	["mohua-ask"] = "你的主公已经魔化了，疯狂的杀戮欲望吞噬了他，是否出一张杀提供给他？",
	["#MohuaBuff"] = "%from 狂性大发，本次伤害从 %arg 点上升至 %arg2 点",

--deathmask迪斯马斯克——4——圣——巨蟹星座
	["deathmask"] = "迪斯马斯克", 
	["mingbo"] = "冥波",
	[":mingbo"] = "当你的杀造成伤害后，可翻开牌堆顶的一张牌，若为黑桃2-9再增加2点伤害并变为雷电属性；为红桃1-10则加1点伤害并变为火焰属性",
	["$Mingbo"] = "%from 翻开牌堆顶的一张牌，为 %card",

--aiolia艾奥利亚—4血——圣——狮子星座
	["aiolia"] = "艾奥利亚", 
	["dian2guang"] = "电光",
	[":dian2guang"] = "锁定技，你的伤害均带有雷属性",
	["lizi"] = "离子",
	[":lizi"] = "当你的杀造成伤害时，你可弃一张雷杀、闪或闪电让此伤害+1",
	["@lizi"] = "你可以弃掉一张雷杀、闪或闪电让此伤害+1",

--shaka沙加—4血——圣——处女星座
	["shaka"] = "沙加", 
	["designer:shaka"] = "StarmK32、宇文天启",
	["xuechi"] = "血池",
	[":xuechi"] = "你每受到1点伤害，可立即弃掉一张杀，视为使用了一张【南蛮入侵】",
	["@xuechi-slash"] = "你可以立即弃掉一张杀，视为使用了一张【南蛮入侵】",
	["sixd"] = "六道",
	[":sixd"] = "回合开始阶段，可进行一次判定，若为红桃1-6，则本回合产生的所有伤害+1",
	["#Sixd1"] = "%from 判定成功^_^",
	["#Sixd2"] = "%from 判定失败-_-",
	["#SixdBuff"] = "%from 的【%arg】技能被触发",
	["jiangmo"] = "降魔",
	[":jiangmo"] = "天启技，游戏开始时，自动获得技能“血池”和“六道”；回合开始阶段，若你没有手牌，可以减1点体力上限，然后获得技能“宝轮”和“莲落”，同时失去“血池”和“六道”\
-----------------------\
★血池：你每受到1点伤害，可立即弃掉一张杀，视为使用了一张【南蛮入侵】\
★六道：回合开始阶段，可进行一次判定，若为红桃1-6，则本回合产生的所有伤害+1\
★宝轮：当你受到伤害时，可令伤害来源进行一次判定，将判定牌扣于武将牌上称为“封”，该角色与“封”相同花色相同类型的牌无法使用，当集齐4种花色时该角色直接死亡\
★莲落：限定技，令所有角色失去1点体力和1点体力上限。本回合结束阶段摸X（X为存活角色数，最多为5）张牌，然后将自己的武将牌翻面",
	["bao2lun"] = "宝轮",
	[":bao2lun"] = "当你受到伤害时，可令伤害来源进行一次判定，将判定牌扣于武将牌上称为“封”，该角色与“封”相同花色相同类型的牌无法使用，当集齐4种花色时该角色直接死亡",
	["$Baolun"] = "%from 将 %card 放在 %to 的武将牌上，%to 受到的不利影响将持续到游戏结束",
	["feng"] = "封",
	["#BFeng"] = "%from 被宝轮封印，%arg2 花色的 %arg 无法使用",
	["#BaolunDie"] = "%from 集齐了四种花色的 %arg 牌，这货死定了",
	["lianluo"] = "莲落",
	[":lianluo"] = "限定技，令所有角色失去1点体力和1点体力上限。本回合结束阶段摸X（X为存活角色数，最多为5）张牌，然后将自己的武将牌翻面",
	["@lianl"] = "莲",
	["$lianluo"] = "--莲落--",

--dohko童虎—4血——天平星座
	["dohko"] = "童虎",
	["shengqi"] = "圣器",
	[":shengqi"] = "回合开始阶段，你可以选择一名角色，然后弃一张手牌，若如此做，该角色的装备不能在其回合外彻底失去，直到你下回合开始阶段\
★不能彻底失去，指的是在结算完毕后会失而复得",
	["$Shengqi"] = "%from 受到【圣器】的影响，%card 失而复得",
	["@sqv"] = "回",
	["bailong"] = "百龙",
	[":bailong"] = "当你使用【杀】时，（在结算前）可弃一张手牌，然后弃掉目标角色的一张牌",
	["longxiang"] = "龙翔",
	[":longxiang"] = "觉醒技，回合开始阶段，当你的体力值不大于场上任一角色，须回复1点体力或者摸两张牌，然后减1点体力上限，并永久获得“连破”技能",
	["longxiang:recover"] = "回复1点体力",
	["longxiang:draw"] = "摸两张牌",
	["#LongxiangWake"] = "%from 的体力值(%arg)为场上为低，觉醒技【%arg2】触发",

--milo米罗…天蝎星座…圣…4血…
	["milo"] = "米罗", 
	["designer:milo"] = "StarmK32、宇文天启",
	["duzhen"] = "毒针",
	[":duzhen"] = "锁定技，你每受到或造成一点伤害，伤害来源或受伤角色获得一枚“针”标记，获得X枚“针”标记的角色必须进行一次判定：\
	X=5 若判定牌为基本牌，随机失去1~2点体力\
	X=10 若判定牌为红色，随机失去0~2点体力上限\
	X=15 若判定牌为桃或酒，弃置所有标记和所有牌，其他结果则直接死亡",
	["#Duzhen"] = "%from 受到【%arg】的影响，进行一次判定",
	["@needle"] = "针",

--aiolos艾奥里斯…射手座…3血…圣……
	["aiolos"] = "艾奥里斯",
	["renma"] = "人马",
	[":renma"] = "锁定技，当你未装备马时，你始终视为装备一匹+1马",
	["shesha"] = "射杀",
	[":shesha"] = "你可以令你的普通杀无法闪避",

--shura修罗—4血——圣——山羊星座
	["shura"] = "修罗",
	["designer:shura"] = "游神ViVA",
	["sheng2jian"] = "圣剑",
	[":sheng2jian"] = "回合开始阶段，若你已受伤且损失了X点体力值，可以执行下列两项中的一项，每回合限一次：\
	1.立即视为你对与你距离X以内的一名角色使用了一张【杀】；\
	2.立即对与你距离大于X的一名角色使用一张【杀】",
	["@shengjian"] = "你可以立即拔出【圣剑】",

--camus卡妙…4血…圣…水瓶星座…
	["camus"] = "卡妙", 
	["designer:camus"] = "宇文天启",
	["dongqi"] = "冻气",
	[":dongqi"] = "锁定技，当你造成一次伤害时，进行一次判定，若为红桃或黑桃，则抵消此次伤害，改为弃掉对方两张牌（包括判定区的牌）",
	["dongqi:damage"] = "我是杀人犯",
	["dongqi:discard"] = "我是拆迁办",
	["binggui"] = "冰柜",
	[":binggui"] = "天启技，当有角色死亡时，你可以减去1点体力上限，将自己的“冻气”技能改为不用判定自由选择的非锁定技",
	["#Binggui"] = "%from 可以自由发动【%arg】技能了",
	["shushu"] = "曙恕",
	[":shushu"] = "锁定技，杀死你的角色失去自己的所有技能按钮并获得技能“冻气”",

--aphrodite阿布罗狄…3血…圣…双鱼星座
	["aphrodite"] = "阿布罗狄",
	["designer:aphrodite"] = "宇文天启",
	["meigui"] = "玫瑰",
	[":meigui"] = "当你受到伤害时，可以弃掉所有牌，然后从牌堆摸取等量的牌",
	["mOgOng"] = "魔宫",
	[":mOgOng"] = "锁定技，当你没装备防具时，始终视为你装备着【八卦阵】，当你发动自带八卦阵效果时，若判定牌为方块牌、黑色基本牌或装备牌，必须将其放到任意一名角色的判定区内",
}

local goldst = {"mu", "aldebaran", "saga", "deathmask", "aiolia", "dohko", "aiolos"}

for _, seinto in ipairs(goldst) do
	t["designer:" .. seinto] = t["designer:goldseinto"]
end

return t