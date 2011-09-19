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


}

local goldst = {"mu", "aldebaran", "saga", "deathmask", "aiolia", "shaka", "dohko", "milo", "aiolos", "shura", "camus", "aphrodite"}

for _, seinto in ipairs(goldst) do
	t["designer:" .. seinto] = t["designer:goldseinto"]
end

return t