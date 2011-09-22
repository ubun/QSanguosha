-- translation for GoldSeintoPrePackage

local t = {
	["goldseintopre"] = "冥神篇", 
	["designer:goldseintopre"] = "游神ViVA",

	["aries"] = "史昂", 
	["jingqiang"] = "晶墙", 
	[":jingqiang"] = "回合结束阶段，你可以指定一名角色，直到你的下个回合开始阶段为止，其他角色与其计算距离时，始终+1",
	["jq-choose"] = "请选择需要晶墙庇护的一名角色",
	["zhuyi"] = "铸衣", 
	[":zhuyi"] = "出牌阶段，你可以弃掉一张装备牌，令包括你在内的两名角色各摸一张牌",
	["zhuanling"] = "转灵", 
	[":zhuanling"] = "主公技，觉醒技，回合开始阶段若你没有手牌，则回复1点体力或摸两张牌，然后减1点体力上限，并永久获得技能“教皇”（圣势力角色可以在他们各自的回合内从手牌给你一张装备牌）",
	["zhuanling:recover"] = "回复1点体力",
	["zhuanling:draw"] = "摸两张牌",
	["#ZhuanlingWake"] = "%from 的手牌数为 %arg，达到【转灵】的觉醒条件",
	["jiaohuang"] = "教皇", 
	[":jiaohuang"] = "圣势力角色可以在他们各自的回合内给你一张未装备的装备牌",
	["jiaohuangv"] = "献给教皇",
	["@jq"] = "水晶墙",

	["taurus"] = "阿鲁迪巴", 
	["haojiao"] = "号角", 
	[":haojiao"] = "回合结束阶段，你可以将你的武将牌翻面，然后视为你对攻击范围内的至多两名角色各使用了一张【杀】；当你受到伤害时，若你的武将牌背面朝上，可以将你的武将牌翻面",
	["@@haojiao"] = "是否吹响【号角】？（会翻面哦~）",

--	["gemini"] = "阿斯普洛斯",
	["gemini"] = "德弗特洛斯", 
	["disui"] = "地碎",
	[":disui"] = "出牌阶段，你使用属性【杀】时，目标角色需连续使用两张【闪】才能抵消",
	["@disui-jink-1"] = "请打出一张【闪】以闪避【杀】",
	["@disui-jink-2"] = "请再打出一张【闪】",
	["ciyuan"] = "次元", 
	[":ciyuan"] = "觉醒技，当场上出现火焰伤害时，你须从牌堆里将7张牌移出游戏称之为“星”，然后减1点体力上限，并永久获得技能“七星”与“大雾”",
	["#CiyuanWake"] = "%from 感觉到场上的火焰气息，看来【次元】的觉醒条件已经到了",

	["cancer"] = "马尼戈特", 
	["shiqi"] = "尸气",
	[":shiqi"] = "出牌阶段，你可以弃两张牌并指定任意一名角色和一个出牌阶段以外的阶段，你的回合结束后令该角色立即执行该阶段。每回合限一次",
	["@shiqi"] = "尸气",
	["#ShiqiEffect"] = "%from 对 %to 使用的技能【%arg】被触发，%to 立即执行 %arg2 阶段",
	["shiqi:start"] = "开始阶段",
	["shiqi:judge"] = "判定阶段",
	["shiqi:draw"] = "摸牌阶段",
	["shiqi:discard"] = "弃牌阶段",
	["shiqi:finish"] = "结束阶段",

	["leo"] = "雷古鲁斯",
	["leiguang"] = "雷光", 
	[":leiguang"] = "你可以将你的普通【杀】当做【雷杀】使用",
	["juexiao"] = "绝啸", 
	[":juexiao"] = "觉醒技，回合开始阶段，若你的体力是全场最少的（或之一），你须减1点体力上限，然后恢复全部体力，并获得“绝啸&超越标记卡”上的一个技能\
	★“绝啸&超越标记卡”的技能列表就是冥神篇所有非限定技、觉醒技和主公技的技能集合",
	["#JuexiaoWake"] = "%from 的体力值(%arg)为场上为低，觉醒技【绝啸】触发",
	["chaoyue"] = "超越", 
	[":chaoyue"] = "你的觉醒技发动后，每受到1点伤害，可以获得“绝啸&超越标记卡”上的一个技能",
	["jxxkudyx"] = "绝啸&超越标记卡",

	["virgo"] = "阿释密达", 
	["tianmo"] = "天魔", 
	[":tianmo"] = "你每使用一张手牌【闪】可以立即摸一张牌，或从牌堆顶展示两张牌移出游戏，称之为“果”",
	["tianmo:draw"] = "摸一张牌",
	["tianmo:guo"] = "获得两张果牌",
	["tianmo:cancel"] = "不发动",
	["guo"] = "果果",
	["liudao"] = "六道", 
	[":liudao"] = "觉醒技，回合开始阶段，若你的“果”牌不少于6张时，须减去1点体力上限，然后你每次摸牌阶段可以挑选一张“果”加入手牌",
	["#LiudaoWake"] = "%from 已经拥有足够的果果，收获的时候到了~（口水ing）",
	["baolun"] = "宝轮", 
	[":baolun"] = "你的觉醒技发动后，出牌阶段，可令一名角色舍弃所有手牌，同时从“果”中补给等量的手牌",

	["libra"] = "童虎",
	["designer:libra"] = "游神ViVA，宇文天启",
	["longba"] = "龙霸",
	[":longba"] = "当你使用的红色【杀】造成伤害时，可以弃掉对方的一张牌",
	["longfei"] = "龙飞",
	[":longfei"] = "觉醒技，回合开始阶段，若你装备区的装备数量为三件或者更多时，你须回复1点体力或者摸两张牌，然后减1点体力上限，并永久获得技能“龙胆”与“直谏”",
	["longfei:recover"] = "回复1点体力",
	["longfei:draw"] = "摸两张牌",
	["#LongfeiWake"] = "%from 的装备数已满足【龙飞】的觉醒条件",

	["scorpio"] = "卡路狄亚", 
	["zhiyan"] = "炙焱",
	[":zhiyan"] = "出牌阶段，你可以自减1点体力上限，令任意一名角色受到1点火焰伤害",

	["sagittarius"] = "希绪弗斯", 
	["jingong"] = "金弓",
	[":jingong"] = "锁定技，回合内，若你没有装备武器，你的攻击范围无限",
	["shanyao"] = "闪耀",
	[":shanyao"] = "觉醒技，回合开始阶段，若你的体力值为1，你须减1点体力上限，并永久获得技能“烈弓”与“不屈”",
	["#ShanyaoWake"] = "%from 的体力值太低，【闪耀】自动觉醒！",

	["capricorn"] = "艾尔熙德",
	["designer:capricorn"] = "宇文天启",
	["shengjian"] = "圣剑",
	[":shengjian"] = "出牌阶段，你可以弃掉一张下列花色的锦囊牌并召唤对应的武器牌并自动装备：黑桃~青龙偃月刀/丈八蛇矛；红桃~麒麟弓/寒冰剑；梅花~青釭剑/方天画戟；方片~贯石斧/雌雄双股剑。你不能召唤已经召唤过的武器，每回合限一次",
	["$Shengjian"] = "%from 召唤出了 %card",
	["duanbi"] = "断臂",
	[":duanbi"] = "隐藏技，觉醒技，判定阶段判定前，若你使用【圣剑】累计达到8次，你须弃掉当前所有牌并永久获得技能“咆哮”",
	["#DuanbiHide"] = "%from 的圣剑被反弹并切断了自己的右手，%from 被激怒了！",
	["#DuanbiWake"] = "%from 领悟到最强力量，【断臂】觉醒！",
	
	["aquarius"] = "笛捷尔",
	["jiguang"] = "极光",
	[":jiguang"] = "你每造成或受到1点伤害，可以将目标或者伤害来源的一张手牌扣置于其武将牌上，称之为“气”",
	["wangqi"] = "望气",
	[":wangqi"] = "判定阶段你可以进行一次判定，若为红色，则你获得所有的气并可以将其任意分配给任何角色，否则气返回其原拥有者的手牌",
	["gas"] = "气",

	["pisces"] = "雅柏菲卡", 
	["mogong"] = "魔宫",
	[":mogong"] = "每当一名其他角色使用了一张【杀】指定了一个或多个目标时，你可以展示一张梅花花色的手牌，使该【杀】取消其余目标并更改成以你为目标（无视距离）",
	["mogongshow"] = "你可以发动【魔宫】：展示一张梅花手牌，将杀的目标指向自己",
	["jingji"] = "荆棘",
	[":jingji"] = "你每受到1点伤害，可以摸两张牌或者令伤害来源弃掉两张牌",
	["jingji:draw"] = "自己摸两张牌",
	["jingji:throw"] = "令伤害来源弃两张牌",
	["jingji:cancel"] = "不发动",

}

local goldstp = {"aries", "taurus", "gemini", "cancer", "leo", "virgo", "scorpio", "sagittarius", "aquarius", "pisces"}

for _, seintopre in ipairs(goldstp) do
	t["designer:" .. seintopre] = t["designer:goldseintopre"]
end

return t