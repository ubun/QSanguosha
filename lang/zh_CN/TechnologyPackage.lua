-- translation for Technology Package

local te = {
	["technology"] = "方技传", 
	["designer:technology"] = "淬毒",

	["guanlu"] = "管辂",
	["tuiyan"] = "推演",
	[":tuiyan"] = "出牌阶段，你使用任意一张牌比在该阶段使用的前一张牌点数大，则这张牌可以指定任一角色为目标（无视规则）",
	["tianji"] = "天机",
	[":tianji"] = "若你在出牌阶段，使用的牌都符合“推演”，且不少于三张，则回合结束后，进行一个额外的回合",
	["mingfa"] = "命罚",
	[":mingfa"] = "锁定技，你因“天机”获得额外回合开始前，从牌堆亮出一张牌，若黑桃花色，则失去2点体力",
	["$Tuiyan"] = "%from 将 %card 的目标改为了 %to",
	["$Mingfa"] = "%from 翻开牌堆顶的一张牌。为 %card",
	["#Mingfa"] = "%from 的锁定技【%arg】被触发",
	
}

local techer = {"zhouxuan", "zhujianping", "guanlu", "dukui"}

for _, player in ipairs(techer) do
	te["designer:" .. player] = te["designer:technology"]
end

return te
