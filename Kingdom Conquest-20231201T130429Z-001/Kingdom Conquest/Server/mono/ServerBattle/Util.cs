using System;
using System.Collections;
using System.Collections.Generic;

namespace TinyWar {
    public delegate void MsgGetHandler(string str);
    public class Util
    {
        /// <summary>
        /// 计算两个格子间的距离
        /// </summary>
        /// <param name="p1"></param>
        /// <param name="p2"></param>
        /// <returns></returns>
        public static int Distance(Position p1, Position p2){
            return (int)(Math.Abs(p1.x - p2.x) + Math.Abs(p1.y - p2.y));
          }

        public static bool isType(object obj, Type t) { 
            return obj.GetType() == t;
        }

        public static event MsgGetHandler OnMsgGet;

        public static void Print(params object[] objs){
            string result = "";
            for (int i = 0; i < objs.Length; ++i) {
                result += objs[i].ToString() +"\t\t\t";
            }
            Console.WriteLine(result);
            if(Util.OnMsgGet != null){
                Util.OnMsgGet(result);
            }    
        }

        public static WarriorType ConvertSoldierSort2WarriorType(int t)
        {
            switch (t)
            {
                case 4:
                    return WarriorType.Archers;
                case 3:
                    return WarriorType.Swordsmen;
                case 2:
                    return WarriorType.Lancers;
                case 1:
                    return WarriorType.Cavalrys;
                case 5:
                    return WarriorType.Catapults;
                default:
                    return WarriorType.LeaderSwordsmen;
            }
        }
    }
}
