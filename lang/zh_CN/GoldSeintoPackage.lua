-- translation for GoldSeintoPackage

local t = {
	["goldseinto"] = "圣斗士", 
	["st"] = "圣", 
	["designer:goldseinto"] = "StarmK32",

	["mu"] = "穆", 
	["aldebaran"] = "阿鲁迪巴", 
	["saga"] = "撒加",
	["deathmask"] = "迪斯马斯克", 
	["aiolia"] = "艾奥利亚", 
	["shaka"] = "沙加", 
	["dohko"] = "童虎",
	["milo"] = "米罗", 
	["aiolos"] = "艾奥里斯", 
	["shura"] = "修罗",
	["camus"] = "卡妙", 
	["aphrodite"] = "阿布罗狄",

	["xiufu"] = "修复",
	[":xiufu"] = "出牌阶段，你可弃掉一张红桃牌，视为使用了一张【桃】，每回合限一次",
	["xingmie"] = "星灭",
	[":xingmie"] = "限定技，当你造成伤害后可以弃1张黑牌让不在你攻击范围的所有角色受到1点同属性伤害",
	["hao2jiao"] = "号角",
	[":hao2jiao"] = "当你使用【杀】指定一名角色为目标后，可以进行判定，若结果为红色，此【杀】不可被闪避，且当结果为方块时，伤害+1，为红桃时，你获得判定牌",
	["huan2long"] = "幻胧", 
	[":huan2long"] = "出牌阶段，你可以与一名角色拼点，若你赢，目标角色对其攻击范围内，你指定的另一名角色（可以是被幻胧的角色本身）造成1点伤害。若你没赢，他/她对你造成1点伤害。每回合限用一次",
	["yinhe"] = "银河",
	[":yinhe"] = "限定技，出牌阶段，你可以弃三枚星标记让一名角色失去1点体力上限",
	["mingbo"] = "冥波",
	[":mingbo"] = "当你的杀造成伤害后，可翻开牌堆顶的一张牌，若为黑桃2-9再增加2点伤害并变为雷电属性；为红桃1-10则加1点伤害并变为火焰属性",
	["$Mingbo"] = "%from 翻开牌堆顶的一张牌，为 %card",
	["dian2guang"] = "电光",
	[":dian2guang"] = "锁定技，你的伤害均带有雷属性",
	["#Dian2guang"] = "%from 的锁定技【%arg】被触发",
	["lizi"] = "离子",
	[":lizi"] = "当你的杀造成伤害时，你可弃一张雷杀、闪或闪电让此伤害+1",
	["@lizi"] = "你可以弃掉一张雷杀、闪或闪电让此伤害+1",
	["#Lizi"] = "%from 发动了技能【%arg】",
	["xuechi"] = "血池",
	[":xuechi"] = "你每受到1点伤害，可立即弃掉一张杀，视为使用了一张【南蛮入侵】",
	["@xuechi-slash"] = "你可以立即弃掉一张杀，视为使用了一张【南蛮入侵】",
	["sixd"] = "六道",
	[":sixd"] = "回合开始阶段，可进行一次判定，若为红色1-6，则本回合产生的所有伤害+1",
	["#Sixd1"] = "%from 判定成功^_^",
	["#Sixd2"] = "%from 判定失败-_-",
	["#SixdBuff"] = "%from 的【%arg】技能被触发",
	["jiangmo"] = "降魔",
	[":jiangmo"] = "天启技，游戏开始时，自动获得技能“血池”和“六道”；回合开始阶段，若你没有手牌，可以减1点体力上限，然后获得技能“宝轮”和“莲落”，同时失去“血池”和“六道”",

}

local goldst = {"mu", "aldebaran", "saga", "deathmask", "aiolia", "shaka", "dohko", "milo", "aiolos", "shura", "camus", "aphrodite"}

for _, seinto in ipairs(goldst) do
	t["designer:" .. seinto] = t["designer:goldseinto"]
end

return t