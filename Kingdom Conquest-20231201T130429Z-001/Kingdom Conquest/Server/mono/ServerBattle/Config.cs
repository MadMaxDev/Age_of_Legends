using System;
using System.Collections;
using System.Collections.Generic;
using System.IO;

namespace TinyWar
{
    public class Config
    {
        /// <summary>
        /// 普通士兵寻找路径的权重,分别为 [前、后、上、下、不动]
        /// </summary>
        public static float[] COMMON_WARRIOR_PATHFIND_RATIO = new float[] { 1, 1, 1, 1, 20 };
        /// <summary>
        /// 将领寻找路径的权重,分别为 [前、后、上、下、不动]
        /// </summary>
        public static float[] CHIEF_WARRIOR_PATHFIND_RATIO = new float[] { 1, 1, 0, 0, 20 };

        public static float[] GRID_ORIGIN = new float[] { -160f, -135f };
        //最小半屏幕间隔
        public const int MIN_HALF_SPACE = 3;
        //士兵最大列数
        public const int MAX_WARRIOR_SPACE = 5;
        public static float GRID_WIDTH = 50.0f;
        public static float GRID_HEIGHT = 50.0f;
        public static int GRID_ROW_COUNT = 5;
        public static int GRID_COLUMN_COUNT = 16;
        //网格和分辨率比值，1像素相当于多少世界坐标
        public const float GRID_RESOLUTION = 0.00625f;
        public const int MAX_TICK_NUM = 10000;
        //勇士移动动画时间
        public const float WARRIOR_MOVE_DURATION = 1.4f;
        //弓箭移动时间,这个是1格的时间
        public const float WEAPON_MOVE_DURATION = 0.15f;
        //战斗判定时间间隔
        public const float BATTLE_LOGIC_DURATION = 0.7f;
        //最近可攻击敌人容量
        public const int NEAR_ENEMY_COUNT = 2;
        //战斗被被盯上以后不动的概率
        public const float LOCKED_MOVE_RATIO = 1f;
        //判断是否可以攻击时Y轴距离，小于这个距离就可以攻击，这个距离是实际坐标，不是格子坐标
        public const float MIN_ATTACK_Y_DISTANCE = 0.00005f;
        //是否展示攻击效果
        public const bool SHOW_WEAPON_EFFECT = true;
        //是否显示debug内容
        public const bool SHOW_DEBUG_CONTENT = false;
        //是否对进入的数据排序
        public const bool SORT_WARRIOR_DATA = true;
        //是否记录日志
        public const bool RECORD_LOG = false;
        //各兵种初始位置和缩放大小
        public static float[][] WARRIOR_DEFAULT_TRANSFORM = new float[6][] {//依次记录左侧位置xy，左边缩放xy，右边位置xy，右边缩放xy
            new float[]{0, -4f, 1f, 1f, 0, -4f, 1f, 1f},                    //Archers
            new float[]{9f, 0f, 1f, 1f, 9f, 0f, 1f, 1f},                    //Cavalrys
            new float[]{3f, 6f, 0.875f, 0.875f, 3f, 6f, 0.875f, 0.875f},    //Catapults
            new float[]{12f, 2f, 1f, 1f, 12f, 2f, 1f, 1f},                  //Lancers
            new float[]{0, 0, 1f, 1f, 0, 0f, 1f, 1f},                       //Swordsmen
            new float[]{0, 22f, 2f, 2f, 5.2f, 15f, 1.75f, 1.75f}            //Chief
        };
        public static WarriorType[][] WARRIOR_COUNTER_MAP = new WarriorType[5][] { 
            new WarriorType[]{WarriorType.Lancers, WarriorType.Cavalrys},       //Archers
            new WarriorType[]{WarriorType.Swordsmen, WarriorType.Catapults},    //Cavalrys
            new WarriorType[]{WarriorType.Lancers, WarriorType.Archers},        //Catapults
            new WarriorType[]{WarriorType.Cavalrys, WarriorType.Swordsmen},     //Lancers
            new WarriorType[]{WarriorType.Archers, WarriorType.Catapults}       //Swordmen
        };
        //克制攻击加强倍率
        public static float ATK_ENHANCE_RATIO = 1.5f;
        //远程类攻击削弱
        public static float REMOTE_ATTACK_RATIO = 0.05f;
        //是否根据战力生成
        public static bool USE_FORCE = true;
        //投石器石头初始Y轴位置
        public const float ROCK_DEFAULT_POSITION_Y = 10f;
    }
}
