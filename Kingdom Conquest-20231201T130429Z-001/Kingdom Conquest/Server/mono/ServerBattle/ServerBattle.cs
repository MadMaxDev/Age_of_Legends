using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using TinyWar;
using System.IO;

namespace ServerBattle
{
    class Program
    {
        //static void Main(string[] args)
        //{
        //    BattleCal bc = new BattleCal();
        //    bc.Init(0, 1.2f, "89302350951350686*2*1*9548*4,89302349944979690*1*1*9404*4,89302483630677754*3*1*5350*5,89302483629891287*4*1*1690*1", "10001*1*1*2025*2,20001*1*2*2025*2,30001*1*3*2025*2,40001*1*4*2025*2,50001*1*5*2025*2,10002*2*1*2025*1,20002*2*2*2025*1,30002*2*3*2025*1,40002*2*4*2025*1,50002*2*5*2025*1,10003*3*1*2025*5,20003*3*2*2025*5,30003*3*3*2025*5,40003*3*4*2025*5,50003*3*5*2025*5,10004*4*1*2025*5,20004*4*2*2025*5,30004*4*3*2025*5,40004*4*4*2025*5,50004*4*5*2025*5,10005*5*1*2025*5,20005*5*2*2025*5,30005*5*3*2025*5,40005*5*4*2025*5,50005*5*5*2025*5");
        //    Console.WriteLine(bc.GetResult());
        //    Console.ReadKey();
        //}
    }

    public class BattleCal { 
        private int seed;
        private float atkRatio;
        private List<BattleWarrior> leftList;
        private List<BattleWarrior> rightList;

        public void Init(int sed, float attackEnhanceRatio, string str1, string str2){
            this.seed = sed;
            this.atkRatio = attackEnhanceRatio;
            this.leftList = new List<BattleWarrior>();
            this.rightList = new List<BattleWarrior>();
            if (str1 != "")
            {
                string[] arr1 = str1.Split(',');
                for (int i = 0; i < arr1.Length; ++i)
                {
                    leftList.Add(ConvertToBW(arr1[i]));
                }
            }
            if (str2 != "")
            {
                string[] arr2 = str2.Split(',');

                for (int i = 0; i < arr2.Length; ++i)
                {
                    rightList.Add(ConvertToBW(arr2[i]));
                }
            }
        }

        public string GetResult(){
            try { 
                GB gb = new GB(seed, atkRatio, leftList, rightList);
                return gb.Start();
            }
            catch (Exception e)
            {
                return "0," + e.Message + "--" + e.StackTrace;
            }
        }

        private BattleWarrior ConvertToBW(string str)
        {
            string[] arr = str.Split('*');
            if (arr.Length < 5)
            {
                Console.WriteLine("team str contains 6 properties");
                return null;
            }
            return new TinyWar.BattleWarrior
            {
                ID = ulong.Parse(arr[0]),
                Name = arr[0],
                TeamID = int.Parse(arr[2]),
                Sequence = int.Parse(arr[1]),
                Force = float.Parse(arr[3]),
                ArmType =  TinyWar.Util.ConvertSoldierSort2WarriorType(int.Parse(arr[4]))
            };
        }
    }

    public class GB
    {
        GuildBattle _battle;
        public GB(int seed, float atkRatio, List<BattleWarrior> ll, List<BattleWarrior> rl)
        {
            _battle = new GuildBattle();
            _battle.Prepare(seed);
            _battle.AttackEnhanceRatio = atkRatio;
            _battle.ConvertToWarriorList(ll, TinyWar.TagType.Left);
            _battle.ConvertToWarriorList(rl, TinyWar.TagType.Right);
        }

        public string Start()
        {
            while (true)
            {
                bool gameover = _battle.Tick();
                if (gameover)
                {
                    if (Config.RECORD_LOG) { 
                    StreamWriter sw = new StreamWriter("result.text");
                    sw.Write(_battle.BattleLog);
                    sw.Close();    
                    }
                    
                    if (_battle.ExceedMaxTick) {
                        return string.Format("0,maxTick-{0},{1},{2}",Config.MAX_TICK_NUM,BattleResult(_battle.WarriorLeft, TagType.Left), BattleResult(_battle.WarriorRight, TagType.Right));

                    }
                    return string.Format("{0},{1},{2}", _battle.InnerWin ? 1 : 2, BattleResult(_battle.WarriorLeft, TagType.Left), BattleResult(_battle.WarriorRight, TagType.Right));
                }
            }
        }

        /// <summary>
        /// 战斗结果返回
        /// </summary>
        /// <param name="list"></param>
        /// <returns></returns>
        public string BattleResult(IList<Warrior> list, TagType type)
        {
            StringBuilder sb = new StringBuilder();
            for (int i = 0; i < list.Count; ++i)
            {
                if (sb.Length > 0)
                {
                    sb.Append(",");
                }
                Warrior bw = list[i];
                sb.Append(string.Format("{0}*{1}*{2}", bw.ID, (int)bw.HP, type == TagType.Left ? "1" : "2"));
            }
            return sb.ToString();
        }
    }
}
