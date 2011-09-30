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
    General *RLliubei = new General(this, "RLliubei$", "shu", 4);

    General *RLguanyu = new General(this, "RLguanyu", "shu", 4);

    General *RLzhangfei = new General(this, "RLzhangfei", "shu", 4);

    General *RLzhugeliang = new General(this, "RLzhugeliang", "shu", 3);

    General *RLzhaoyun = new General(this, "RLzhaoyun", "shu", 4);

    General *RLmachao = new General(this, "RLmachao", "shu", 4);

    General *RLhuangyueying = new General(this, "RLhuangyueying", "shu", 3, false);

    //SHU 008-015
    General *RLliushan = new General(this, "RLliushan$", "shu", 3);

    General *RLmenghuo = new General(this, "RLmenghuo$", "shu", 4);

    General *RLzhurong = new General(this, "RLzhurong$", "shu", 4, false);

    General *RLhuangzhong = new General(this, "RLhuangzhong", "shu", 4);

    General *RLwolong = new General(this, "RLwolong", "shu", 3);

    General *RLpangtong = new General(this, "RLpangtong", "shu", 3);

    General *RLweiyan = new General(this, "RLweiyan", "shu", 4);

    General *RLjiangwei = new General(this, "RLjiangwei", "shu", 4);

    //SHU 101-103
    General *RLEXxushu = new General(this, "RLEXxushu", "shu", 3);

    General *RLEXfazheng = new General(this, "RLEXfazheng", "shu", 3);

    General *RLEXmasu = new General(this, "RLEXmasu", "shu", 3);

    //WEI 001-007
    General *RLcaocao = new General(this, "RLcaocao$", "wei", 4);

    General *RLsimayi = new General(this, "RLsimayi", "wei", 3);

    General *RLxiahoudun = new General(this, "RLxiahoudun", "wei", 4);

    General *RLzhangliao = new General(this, "RLzhangliao", "wei", 4);

    General *RLxuchu = new General(this, "RLxuchu", "wei", 4);

    General *RLguojia = new General(this, "RLguojia", "wei", 3);

    General *RLzhenji = new General(this, "RLzhenji", "wei", 3, false);

    //WEI 008-015
    General *RLcaopi = new General(this, "RLcaopi$", "wei", 3);

    General *RLxunyu = new General(this, "RLxunyu", "wei", 3);

    General *RLcaoren = new General(this, "RLcaoren", "wei", 4);

    General *RLdianwei = new General(this, "RLdianwei", "wei", 4);

    General *RLxiahouyuan = new General(this, "RLxiahouyuan", "wei", 4);

    General *RLxuhuang = new General(this, "RLxuhuang", "wei", 4);

    General *RLzhanghe = new General(this, "RLzhanghe", "wei", 4);

    General *RLjinxuandi = new General(this, "RLjinxuandi$", "wei", 3);

    //WEI 101-105
    General *RLEXzhangchunhua = new General(this, "RLEXzhangchunhua$", "wei", 3, false);

    General *RLEXcaozhi = new General(this, "RLEXcaozhi", "wei", 3);

    General *RLEXyujin = new General(this, "RLEXyujin", "wei", 4);

    General *RLEXdengai = new General(this, "RLEXdengai", "wei", 4);

    General *RLEXyangxiu = new General(this, "RLEXyangxiu", "wei", 3);

    //WU 001-007
    General *RLsunquan = new General(this, "RLsunquan$", "wu", 4);

    General *RLzhouyu = new General(this, "RLzhouyu", "wu", 3);

    General *RLlvmeng = new General(this, "RLlvmeng", "wu", 4);

    General *RLluxun = new General(this, "RLluxun", "wu", 3);

    General *RLganning = new General(this, "RLganning", "wu", 4);

    General *RLhuanggai = new General(this, "RLhuanggai", "wu", 4);

    General *RLdaqiao = new General(this, "RLdaqiao", "wu", 3, false);

    //WU 008-015
    General *RLsunce = new General(this, "RLsunce$", "wu", 4);

    General *RLsunjian = new General(this, "RLsunjian$", "wu", 4);

    General *RLsunshangxiang = new General(this, "RLsunshangxiang", "wu", 3, false);

    General *RLlusu = new General(this, "RLlusu", "wu", 3);

    General *RLxiaoqiao = new General(this, "RLxiaoqiao", "wu", 3, false);

    General *RLtaishici = new General(this, "RLtaishici", "wu", 4);

    General *RLzhangzhaozhanghong = new General(this, "RLzhangzhaozhanghong", "wu", 3);

    General *RLzhoutai = new General(this, "RLzhoutai", "wu", 4);

    //WU 001-003
    General *RLEXwuguotai = new General(this, "RLEXwuguotai$", "wu", 3, false);

    General *RLEXxusheng = new General(this, "RLEXxusheng", "wu", 4);

    General *RLEXlingtong = new General(this, "RLEXlingtong", "wu", 4);

    //QUN 001-007
    General *RLhuatuo = new General(this, "RLhuatuo", "qun", 3);

    General *RLlvbu = new General(this, "RLlvbu", "qun", 4);

    General *RLdiaochan = new General(this, "RLdiaochan", "qun", 3, false);

    General *RLyuanshao = new General(this, "RLyuanshao$", "qun", 4);

    General *RLyanliangwenchou = new General(this, "RLyanliangwenchou", "qun", 4);

    General *RLzhangjiao = new General(this, "RLzhangjiao$", "qun", 3);

    General *RLyuji = new General(this, "RLyuji", "qun", 3);

    //QUN 008-015
    General *RLdongzhuo = new General(this, "RLdongzhuo$", "qun", 8);

    General *RLjiaxu = new General(this, "RLjiaxu", "qun", 3);

    General *RLpangde = new General(this, "RLpangde", "qun", 4);

    General *RLcaiwenji = new General(this, "RLcaiwenji", "qun", 3, false);

    General *RLzuoci = new General(this, "RLzuoci", "qun", 3);

    General *RLchengong = new General(this, "RLchengong", "qun", 3);

    General *RLgaoshun = new General(this, "RLgaoshun", "qun", 4);

    General *RLgongsunzan = new General(this, "RLgongsunzan$", "qun", 4);

    //QUN 101
    General *RLEXyuanshu = new General(this, "RLEXyuanshu", "qun", 3);
}

ADD_PACKAGE(ReturnOfLegend);
