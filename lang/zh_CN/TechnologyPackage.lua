-- translation for Technology Package

local te = {
	["technology"] = "方技传", 
	["designer:technology"] = "淬毒",

	
}

local techer = {"zhouxuan", "zhujianping", "guanlu", "dukui"}

for _, player in ipairs(techer) do
	te["designer:" .. player] = te["designer:technology"]
end

return te
