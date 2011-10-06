#include "returnoflegend.h"
#include "standard-skillcards.h"
#include "general.h"
#include "skill.h"
#include "engine.h"
#include "standard.h"
#include "carditem.h"
#include "generaloverview.h"
#include "clientplayer.h"
#include "client.h"
#include "maneuvering.h"
#include "room.h"
#include "ai.h"

#include <QCommandLinkButton>

ReturnOfLegendPackage::ReturnOfLegendPackage()
    :Package("returnoflegendpackage")
{
    //SHU 001-007
    General *RLliubei = new General(this, 1101, "RLliubei$", "shu", 4);

    General *RLguanyu = new General(this, 1102, "RLguanyu", "shu", 4);

    General *RLzhangfei = new General(this, 1103, "RLzhangfei", "shu", 4);

    General *RLzhugeliang = new General(this, 1104, "RLzhugeliang", "shu", 3);

    General *RLzhaoyun = new General(this, 1105, "RLzhaoyun", "shu", 4);

    General *RLmachao = new General(this, 1106, "RLmachao", "shu", 4);

    General *RLhuangyueying = new General(this, 1107, "RLhuangyueying", "shu", 3, false);

    //SHU 008-015
    General *RLliushan = new General(this, 1108, "RLliushan$", "shu", 3);

    General *RLmenghuo = new General(this, 1109, "RLmenghuo$", "shu", 4);

    General *RLzhurong = new General(this, 1110, "RLzhurong$", "shu", 4, false);

    General *RLhuangzhong = new General(this, 1111, "RLhuangzhong", "shu", 4);

    General *RLwolong = new General(this, 1112, "RLwolong", "shu", 3);

    General *RLpangtong = new General(this, 1113, "RLpangtong", "shu", 3);

    General *RLweiyan = new General(this, 1114, "RLweiyan", "shu", 4);

    General *RLjiangwei = new General(this, 1115, "RLjiangwei", "shu", 4);

    //SHU 101-103
    General *RLEXxushu = new General(this, 1601, "RLEXxushu", "shu", 3);

    General *RLEXfazheng = new General(this, 1602, "RLEXfazheng", "shu", 3);

    General *RLEXmasu = new General(this, 1603, "RLEXmasu", "shu", 3);

    //WEI 001-007
    General *RLcaocao = new General(this, 1201, "RLcaocao$", "wei", 4);

    General *RLsimayi = new General(this, 1202, "RLsimayi", "wei", 3);

    General *RLxiahoudun = new General(this, 1203, "RLxiahoudun", "wei", 4);

    General *RLzhangliao = new General(this, 1204, "RLzhangliao", "wei", 4);

    General *RLxuchu = new General(this, 1205, "RLxuchu", "wei", 4);

    General *RLguojia = new General(this, 1206, "RLguojia", "wei", 3);

    General *RLzhenji = new General(this, 1207, "RLzhenji", "wei", 3, false);

    //WEI 008-015
    General *RLcaopi = new General(this, 1208, "RLcaopi$", "wei", 3);

    General *RLxunyu = new General(this, 1209, "RLxunyu", "wei", 3);

    General *RLcaoren = new General(this, 1210, "RLcaoren", "wei", 4);

    General *RLdianwei = new General(this, 1211, "RLdianwei", "wei", 4);

    General *RLxiahouyuan = new General(this, 1212, "RLxiahouyuan", "wei", 4);

    General *RLxuhuang = new General(this, 1213, "RLxuhuang", "wei", 4);

    General *RLzhanghe = new General(this, 1214, "RLzhanghe", "wei", 4);

    General *RLjinxuandi = new General(this, 1215, "RLjinxuandi$", "wei", 3);

    //WEI 101-105
    General *RLEXzhangchunhua = new General(this, 1701, "RLEXzhangchunhua$", "wei", 3, false);

    General *RLEXcaozhi = new General(this, 1702, "RLEXcaozhi", "wei", 3);

    General *RLEXyujin = new General(this, 1703, "RLEXyujin", "wei", 4);

    General *RLEXdengai = new General(this, 1704, "RLEXdengai", "wei", 4);

    General *RLEXyangxiu = new General(this, 1705, "RLEXyangxiu", "wei", 3);

    //WU 001-007
    General *RLsunquan = new General(this, 1301, "RLsunquan$", "wu", 4);

    General *RLzhouyu = new General(this, 1302, "RLzhouyu", "wu", 3);

    General *RLlvmeng = new General(this, 1303, "RLlvmeng", "wu", 4);

    General *RLluxun = new General(this, 1304, "RLluxun", "wu", 3);

    General *RLganning = new General(this, 1305, "RLganning", "wu", 4);

    General *RLhuanggai = new General(this, 1306, "RLhuanggai", "wu", 4);

    General *RLdaqiao = new General(this, 1307, "RLdaqiao", "wu", 3, false);

    //WU 008-015
    General *RLsunce = new General(this, 1308, "RLsunce$", "wu", 4);

    General *RLsunjian = new General(this, 1309, "RLsunjian$", "wu", 4);

    General *RLsunshangxiang = new General(this, 1310, "RLsunshangxiang", "wu", 3, false);

    General *RLlusu = new General(this, 1311, "RLlusu", "wu", 3);

    General *RLxiaoqiao = new General(this, 1312, "RLxiaoqiao", "wu", 3, false);

    General *RLtaishici = new General(this, 1313, "RLtaishici", "wu", 4);

    General *RLzhangzhaozhanghong = new General(this, 1314, "RLzhangzhaozhanghong", "wu", 3);

    General *RLzhoutai = new General(this, 1315, "RLzhoutai", "wu", 4);

    //WU 001-003
    General *RLEXwuguotai = new General(this, 1801, "RLEXwuguotai$", "wu", 3, false);

    General *RLEXxusheng = new General(this, 1802, "RLEXxusheng", "wu", 4);

    General *RLEXlingtong = new General(this, 1803, "RLEXlingtong", "wu", 4);

    //QUN 001-007
    General *RLhuatuo = new General(this, 1401, "RLhuatuo", "qun", 3);

    General *RLlvbu = new General(this, 1402, "RLlvbu", "qun", 4);

    General *RLdiaochan = new General(this, 1403, "RLdiaochan", "qun", 3, false);

    General *RLyuanshao = new General(this, 1404, "RLyuanshao$", "qun", 4);

    General *RLyanliangwenchou = new General(this, 1405, "RLyanliangwenchou", "qun", 4);

    General *RLzhangjiao = new General(this, 1406, "RLzhangjiao$", "qun", 3);

    General *RLyuji = new General(this, 1407, "RLyuji", "qun", 3);

    //QUN 008-015
    General *RLdongzhuo = new General(this, 1408, "RLdongzhuo$", "qun", 8);

    General *RLjiaxu = new General(this, 1409, "RLjiaxu", "qun", 3);

    General *RLpangde = new General(this, 1410, "RLpangde", "qun", 4);

    General *RLcaiwenji = new General(this, 1411, "RLcaiwenji", "qun", 3, false);

    General *RLzuoci = new General(this, 1412, "RLzuoci", "qun", 3);

    General *RLchengong = new General(this, 1413, "RLchengong", "qun", 3);

    General *RLgaoshun = new General(this, 1414, "RLgaoshun", "qun", 4);

    General *RLgongsunzan = new General(this, 1415, "RLgongsunzan$", "qun", 4);

    //QUN 101
    General *RLEXyuanshu = new General(this, 1901, "RLEXyuanshu", "qun", 3);
}

ADD_PACKAGE(ReturnOfLegend);
