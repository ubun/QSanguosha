
-- this script to store the basic configuration for game program itself
-- and it is a little different from config.ini

config = {
        version = "20120308",
        version_name = "玉女",
        mod_name = "Moli-ShineWay",
        kingdoms = { "wei", "shu", "wu", "qun", "god"},
        package_names = {
        "StandardCard",
        "StandardExCard",
        "Maneuvering",
        "SPCard",
        "YitianCard",
        "Nostalgia",
        "Joy",
        "Disaster",
        "JoyEquip",
        "KusoCard",

        "Standard",
        "Wind",
        "Fire",
        "Thicket",
        "Mountain",
        "God",
        "SP",
        "YJCM",
        "YJCM2012",
        "Special3v3",
        "BGM",
        "NostalGeneral",
        "Yitian",
        "Wisdom",

-- ShineWay
        "Red",
        "Cyan",
        "Green",
        "Purple",
        "Kuso",
        "Technology",
        "Peasa",

        "Test";
        },

        scene_names = {
        "Guandu",
        "Fancheng",
        "Couple",
        "Zombie",
        "Impasse",
        "Custom",
	},

        color_wei = "#547998",
        color_shu = "#D0796C",
        color_wu = "#4DB873",
        color_qun = "#8A807A",
        color_god = "#96943D",
}

for i=1, 21 do
        local scene_name = ("MiniScene_%02d"):format(i)
        table.insert(config.scene_names, scene_name)
end

