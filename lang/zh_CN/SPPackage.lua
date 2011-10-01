-- translation for SP Package

return {
	["sp"] = "SP", 
	["gongsunzan"] = "公孙瓒", 
	["yicong"] = "义从", 
	[":yicong"] = "锁定技，只要你的体力值大于2点，你计算与其他角色的距离时，始终-1；只要你的体力值为2点或更低，其他角色计算与你的距离时，始终+1。",
	
	["yuanshu"] = "袁术",
	["yongsi"] = "庸肆",
	[":yongsi"] = "锁定技，摸牌阶段，你额外摸X张牌，X为场上现存势力数。弃牌阶段，你至少弃掉等同于场上现存势力数的牌（不足则全弃）",
	["weidi"] = "伪帝",
	[":weidi"] = "锁定技，你视为拥有当前主公的主公技。",
	
	["tuoqiao"] = "脱壳",
	["xuwei"] = "续尾",
	[":tuoqiao"] = "金蝉脱壳，变身为SP貂蝉",
	[":xuwei"] = "狗尾续貂，变身为原版貂蝉",
	
	["#YongsiGood"] = "%from 的锁定技【庸肆】被触发，额外摸了 %arg 张牌",
	["#YongsiBad"] = "%from 的锁定技【庸肆】被触发，必须至少弃掉 %arg 张牌",
	["#YongsiWorst"] = "%from 的锁定技【庸肆】被触发，弃掉了所有的装备和手牌（共 %arg 张）",
	
	["taichen"] = "抬榇",
	[":taichen"] = "出牌阶段，你可以自减1点体力或弃一张武器牌，弃掉你攻击范围内的一名角色处（手牌、装备区、判定区）的两张牌，每回合中，你可以多次使用抬榇",
	["$taichen"] = "良将不惧死以苟免，烈士不毁节以求生",
	["~sp_pangde"] = "吾宁死于刀下，岂降汝乎",
	
	["cv:gongsunzan"] = "",
	["cv:yuanshu"] = "名将三国",
	["cv:sp_sunshangxiang"] = "",
	["cv:sp_diaochan"] = "",
	["cv:sp_pangde"] = "Glory",
	
	["designer:sp_pangde"] = "太阳神上",
	
	["$yongsi1"] = "嘿呀，还不错",
	["$yongsi2"] = "呐~哈哈哈哈哈",
	["$yongsi3"] = "呀呀呀呀呀呀",
	["$yongsi4"] = "嗙啪~呜哈哈哈哈哈",
	["~yuanshu"] = "呃呀~~~~~~~",
	
	["~gongsunzan"] = "如今阵败吾已无颜苟活于世",
        
	["sp_sunshangxiang"] = "SP孙尚香",
	["sp_pangde"] = "SP庞德",
	["sp_diaochan"] = "SP貂蝉",
        
	["shenlvbu1"] = "神吕布(1)",
	["shenlvbu2"] = "神吕布(2)",
	["xiuluo"] = "修罗",
	[":xiuluo"] = "回合开始阶段，你可以弃一张手牌来弃置你判定区里的一张延时类锦囊(必须花色相同)。",
	["shenwei"] = "神威",
	[":shenwei"] = "<b>锁定技</b>，摸牌阶段，你额外摸两张牌，你的手牌上限+2。",	
	["shenji"] = "神戟",
	[":shenji"] = "没装备武器时，你使用的杀可指定至多3名角色为目标。",
        
	["#Reforming"] = "%from 进入重整状态",
	["#ReformingRecover"] = "%from 在重整状态中回复了1点体力",
	["#ReformingRevive"] = "%from 从重整状态中复活!",
	["draw_1v3"] = "重整摸牌",
	["weapon_recast"] = "武器重铸",
	["hulaopass"] = "虎牢关模式",
	
	["sp_guanyu"] = "SP关羽",
	["danji"] = "单骑",
	[":danji"] = "<b>觉醒技</b>，回合开始阶段，若你的手牌数大于你当前的体力值，且本局主公为曹操时，你须减1点体力上限并永久获得技能“马术”。",
	["#DanjiWake"] = "%from 的手牌数(%arg)多于体力值(%arg2)，且本局主公为曹操，达到【单骑】的觉醒条件",
	
	["sp_diaochan"] = "SP貂蝉",
	["tuoqiao"] = "脱壳",
	["xuwei"] = "续尾",
	[":tuoqiao"] = "金蝉脱壳，变身为SP貂蝉",
	[":xuwei"] = "狗尾续貂，变身为原版貂蝉",
	
	["OEgodzhangliao"] = "神张辽",
	["pozhen"] = "破阵",
	["Pozhen"] = "破阵",
	["$pozhen"] = "没想到吧",
	["$Pozhen"] = "没想到吧",
	[":Pozhen"] = "摸牌阶段，你可以放弃摸牌，然后从至多三名（至少一名）角色的手牌里各抽取一张牌。",
	["@Pozhen-card"] = "您是否想发动【破阵】技能？",
	["Zhangliao_shenwei"] = "神威",
	["$Zhangliao_shenwei"] = "能进能退乃真正法器",
	[":Zhangliao_shenwei"] = "出牌阶段可将【闪】当【杀】打出。",
	["changdao"] = "长刀",
	[":changdao"] = "<b>锁定技</b>，你没装备武器时，攻击范围为3。",
	["~OEgodzhangliao"] = "真没想到",
	
	["OEsunquanXYJ"] = "孙权(逍遥津)",
	["xiaozhiheng"] = "小制衡",
	[":xiaozhiheng"] = "出牌阶段,你可以弃掉至多3张牌,然后摸取等量的牌.每回合限用一次",
	["~OEsunquanXYJ"] = "父亲，大哥，仲谋溃矣……",
	
	["OEganningXYJ"] = "甘宁(逍遥津)",
	["xiaoqixi"] = "小奇袭",
	[":xiaoqixi"] = "出牌阶段，你可以将你的任意黑色牌当【过河拆桥】使用，每回合限用2次",
	["~OEganningXYJ"] = "二十年后，又是一条好汉。",
	
	["ganlingdebuhe"] = "甘凌的不和",
	[":ganlingdebuhe"] = "锁定技，凌统每使甘宁回复1点体力，甘宁失去1点体力。",
	["linggandebuhe"] = "凌甘的不和",
	[":linggandebuhe"] = "锁定技，甘宁每使凌统回复1点体力，凌统失去1点体力。",
	
	["@qiaoxiao"] = "桥小难渡",
	
	["sp_cards"] = "SP卡牌包",
	["sp_moonspear"] = "SP银月枪", 
	[":sp_moonspear"] = "你的回合外，若打出了一张黑色的牌，你可以立即指定攻击范围内的一名角色打出一张【闪】，否则受到你对其造成的1点伤害", 
	["@moon-spear-jink"] = "受到SP银月枪技能的影响，你必须打出一张【闪】", 
}
