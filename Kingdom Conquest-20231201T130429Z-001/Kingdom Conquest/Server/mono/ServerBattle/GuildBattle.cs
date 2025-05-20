using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace TinyWar
{
    public class GuildBattle
    {
        #region Field
        IList<Warrior> _warriorLeft, _warriorRight, _warriorAll;

        Dictionary<Position, TagType> _posMap;              //占领标示列表，见Warrior.GridMap 
        TinyWar.Random _rand;                               //所有随机数的生成实例 
        int _flag = 0;                                      //记录主tick循环次数 
        IDictionary<int, IList<TickArgs>> _tickAction;      //每tick需要做的行动列表
        private bool _isWin;                                //左侧是否胜利
        public string BattleLog;
        public bool ExceedMaxTick;                          //是否是超时返回
        #endregion

        #region Event
        public delegate void TickHandler(int tick);
        public event TickHandler OnTick;
        #endregion

        #region Property
        /// <summary>
        /// 所有士兵列表，包括敌方和我方的
        /// </summary>
        public IList<Warrior> WarriorAll { get { return this._warriorAll; } }
        public IList<Warrior> WarriorLeft { get { return this._warriorLeft; } }
        public IList<Warrior> WarriorRight { get { return this._warriorRight; } }
        public bool InnerWin { get { return this._isWin; } }
        public float AttackEnhanceRatio {
            set { Config.ATK_ENHANCE_RATIO = value; }
        }
        #endregion

        /// <summary>
        /// 初始化各种变量
        /// </summary>
        /// <param name="seed">随机数种子</param>
        public void Prepare(int seed)
        {
            this._rand = new TinyWar.Random(seed);
            this._tickAction = new Dictionary<int, IList<TickArgs>>();

            Warrior.GridMap = new Dictionary<Position, TagType>(new PositionEqual());
            this._posMap = Warrior.GridMap;
            this._warriorLeft = new List<Warrior>();
            this._warriorRight = new List<Warrior>();
            this._warriorAll = new List<Warrior>();
        }
        private int _tickNum = 0;
        //游戏主要逻辑
        public bool Tick()
        {
            _tickNum++;
            if (Config.RECORD_LOG) { 
                if (sb.Length > 1)
                    sb.Append("\n");
                sb.Append("tick num        "+ this._tickNum);
                if (_tickNum == 1) {
                    sb.Append("\nleft\n");
                    for (int i = 0; i < this._warriorLeft.Count; ++i) {
                        sb.Append(this._warriorLeft[i].ToString()+"\n");
                    }
                    sb.Append("\nright\n");
                    for (int i = 0; i < this._warriorRight.Count; ++i)
                    {
                        sb.Append(this._warriorRight[i].ToString() + "\n");
                    }
                }
            }
            
            if (CheckGameOver())
            {
                this.BattleLog =  this.sb.ToString();
                return true;
            }
            //每次循环交换team中战士的左右两队的顺序，防止总是一方先判断
            if (this._flag % 2 == 0)
            {
                this._warriorAll = this._warriorLeft.Concat(this._warriorRight).ToList<Warrior>();
            }
            else
            {
                this._warriorAll = this._warriorRight.Concat(this._warriorLeft).ToList<Warrior>();
            }
            this.CoordinateData(this._flag);

            int allLength = this._warriorAll.Count;

            for (int i = 0; i < allLength; ++i)
            {
                Warrior cur = this._warriorAll[i];
                AttackEnemyCollection nearEnemyList = new AttackEnemyCollection(cur);//最近可以攻击的敌人列表

                bool enemyInAttackGrid = false;//是否有敌人下一步进入攻击范围

                //如果是远程攻击，先把所有攻击范围内的找出来，然后选择最近的攻击
                Warrior _nearEnemy = null;
                if (cur.AtkDistance > 1)
                {
                    AttackEnemyCollection remoteAtkList = new AttackEnemyCollection(1, cur);
                    for (int j = 0; j < allLength; ++j)
                    {
                        Warrior compare = this._warriorAll[j];
                        if (cur.CanAttack(compare.Pos) && cur.Tag != compare.Tag && cur.isAlive && compare.isAlive && !cur.isAttack && !cur.isMove)
                        {
                            remoteAtkList.Push(compare);
                        }
                    }
                    Warrior  top = remoteAtkList.Pop();
                    if (top != null && top.Pos.y == cur.Pos.y)
                    {
                        _nearEnemy = top;
                    }
                    
                    remoteAtkList.Clear();
                }

                for (int j = 0; j < allLength; ++j)
                {
                    Warrior compare = this._warriorAll[j];
                    if (cur.Tag != compare.Tag && cur.isAlive && compare.isAlive)
                    {
                        //本次可以攻击
                        if (cur.CanAttack(compare.Pos) && !cur.isAttack && !cur.isMove)
                        {
                            if (cur.AtkDistance > 1 && _nearEnemy != null && _nearEnemy != compare)
                            {
                                continue;
                            }
                            float damage = cur.Attack(compare);

                            int tickNo;
                            //if (cur.AtkDistance == 1)
                            //{
                            //    tickNo = 1 + this._flag;
                            //}
                            //else
                            //{
                            //    tickNo = (int)Math.Ceiling(Config.WEAPON_MOVE_DURATION / Config.BATTLE_LOGIC_DURATION) + this._flag;
                            //}
                            tickNo = 1 + this._flag;

                            if (!this._tickAction.ContainsKey(tickNo))
                            {
                                this._tickAction[tickNo] = new List<TickArgs>();
                            }
                            this._tickAction[tickNo].Add(new VitalityTickArgs
                            {
                                Damage = damage,
                                Attack = cur,
                                BeAttack = compare
                            });
                            break;
                        }
                        //如果下一次就能打到敌人
                        if (cur.CanAttack(compare.AimPos))
                        {
                            enemyInAttackGrid = true;
                        }
                        nearEnemyList.Push(compare);
                    }
                }
                
                if (enemyInAttackGrid)
                {   //如果有下轮就能攻击到的敌人，本轮就不动。
                    cur.StandBy();
                    continue;
                }
                
                //在可攻击列表中逐个寻找可以攻击的敌人，除非全部不能攻击，则本轮结束；有能攻击的，就break
                if (!cur.isAttack && !cur.isMove)
                {
                    Warrior nearEnemy;
                    while ((nearEnemy = nearEnemyList.Pop()) != null)
                    {
                        if (this.MoveWarrior(cur, nearEnemy))
                        {
                            nearEnemyList.Clear();
                            break;
                        }
                    }
                }

                if(!cur.isAttack && !cur.isMove && cur.isAlive)
                    cur.StandBy();
            }
            if (this.OnTick != null) {
                this.OnTick(this._flag);
            }
            _flag++;
            return false;
        }

        public void ConvertToWarriorList(List<BattleWarrior> list, TagType tag) {
            if (list == null) {
                return;
            }

            if (Config.SORT_WARRIOR_DATA) {
                list.Sort(this.WarriorSort);
            }
            int half = Config.GRID_COLUMN_COUNT / 2;
            int minX, maxX;
            if (tag == TagType.Left)
            {
                maxX = half - Config.MIN_HALF_SPACE;
                minX = maxX - Config.MAX_WARRIOR_SPACE;
            }
            else {
                minX = half + Config.MIN_HALF_SPACE;
                maxX = minX + Config.MAX_WARRIOR_SPACE;
            }
            IList<Warrior> result = new List<Warrior>(list.Count);
            for (int i = 0; i < list.Count; ++i) {
                BattleWarrior cur = list[i];
                int x = (tag == TagType.Left) ? maxX - cur.TeamID : minX + cur.TeamID - 1;
                Position pos = new Position(x, Config.GRID_ROW_COUNT - cur.Sequence);
                Warrior w = new Warrior {
                    HP = cur.Force,
                    Atk = 0,
                    Def = 0,
                    Pos = pos,
                    AimPos = pos,
                    AtkDistance = (cur.ArmType == WarriorType.Archers || cur.ArmType == WarriorType.Catapults ? 2 : 1),
                    Tag = tag,
                    Face = tag == TagType.Left ? Direction.Right : Direction.Left,
                    ArmType = cur.ArmType,
                    Name = tag.ToString() + i.ToString(),
                    ID = cur.ID,
                    isAlive = true,
                    isAttack = false,
                    Force = cur.Force,
                    isMove = false
                };
                w.OnMove += this.WarriorOnMove;
                if (tag == TagType.Left)
                {
                    this._warriorLeft.Add(w);
                }
                else {
                    this._warriorRight.Add(w);
                }
            }
        }

        /// <summary>
        /// 计算士兵的坐标
        /// </summary>
        private void CalculateWarriorPosition()
        {
            //注意创建顺序，添加顺寻会影响寻路顺序，按照从里到外、从下到上的顺序创建
            int maxY = Config.GRID_ROW_COUNT - 1;
            int[] val = new CalculateWarriorCoords(this.CalculateLeftCoords)(this._warriorLeft);
            int minX = val[0];
            int maxX = val[1];
            int length = this._warriorLeft.Count;

            for (int i = maxX; i >= minX; i--)
            {
                for (int j = maxY; j >= 0; j--)
                {
                    int index = Config.GRID_ROW_COUNT * (i - minX) + j;

                    if (index > length - 1)
                    {
                        continue;
                    }
                    Warrior w = this._warriorLeft[index];
                    w.Pos = new Position(i, j);
                    w.OnMove += this.WarriorOnMove;
                }
            }
            this._warriorLeft = this._warriorLeft.Reverse().ToList<Warrior>();

            val = new CalculateWarriorCoords(this.CalculateRightCoords)(this._warriorRight);
            minX = val[0];
            maxX = val[1];
            length = this._warriorRight.Count;

            for (int i = minX; i <= maxX; i++)
            {
                for (int j = maxY; j >= 0; j--)
                {
                    int index = Config.GRID_ROW_COUNT * (maxX - i) + j;
                    if (index > length - 1)
                    {
                        continue;
                    }
                    Warrior w = this._warriorRight[index];
                    w.Pos = new Position(i, j);
                    w.OnMove += this.WarriorOnMove;
                }
            }
            this._warriorRight = this._warriorRight.Reverse().ToList<Warrior>();
        }
        /// <summary>
        /// 移动勇士
        /// </summary>
        /// <param name="attack">攻击方</param>
        /// <param name="beAttack">被攻击方</param>
        /// <returns></returns>
        private bool MoveWarrior(Warrior attack, Warrior beAttack)
        {
            bool hasMoved = false;
            Position aimAt = new Position(0, 0);
            Position p1 = attack.Pos;
            Position p2 = beAttack.AimPos;
            MoveAction[] actions = new MoveAction[] { MoveAction.Stand, MoveAction.Stand };//默认用户有两个移动方向的选择来靠近敌人，左右或者上下
            //如果y轴不一致，优先考虑y轴
            bool isChief = Util.isType(attack, typeof(Chief));
            if (!hasMoved && !isChief)
            {
                if (p1.y > p2.y)
                {
                    aimAt = new Position(p1.x, p1.y - 1);
                    actions[0] = MoveAction.Down;
                    if (CanMoveTo(aimAt))
                    {
                        hasMoved = true;
                        attack.TurnFace(beAttack.Pos);
                        attack.Move(aimAt);
                    }
                }
                else if (p1.y < p2.y)
                {
                    aimAt = new Position(p1.x, p1.y + 1);
                    actions[0] = MoveAction.Up;
                    if (CanMoveTo(aimAt))
                    {
                        hasMoved = true;
                        attack.TurnFace(beAttack.Pos);
                        attack.Move(aimAt);
                    }
                }
            }
            if (!hasMoved)
            {//x方向尽量往前走
                if (p1.x > p2.x)
                {
                    aimAt = new Position(p1.x - 1, p1.y);
                    actions[1] = MoveAction.Backward;
                    if (CanMoveTo(aimAt))
                    {
                        hasMoved = true;
                        attack.Move(aimAt);
                    }
                }
                else if (p1.x < p2.x)
                {
                    aimAt = new Position(p1.x + 1, p1.y);
                    actions[1] = MoveAction.Forward;
                    if (CanMoveTo(aimAt))
                    {
                        hasMoved = true;
                        attack.Move(aimAt);
                    }
                }
                else
                {//如果对方左侧有人就向右走，反之向左右
                    Position p2Left = new Position { x = p2.x - 1, y = p1.y };
                    Position p2Right = new Position { x = p2.x + 1, y = p1.y };
                    if (!this._posMap.ContainsKey(p2Left))
                    {
                        if (CanMoveTo(p2Left))
                        {
                            hasMoved = true;
                            attack.Move(p2Left);
                        }
                    }
                    else if (!this._posMap.ContainsKey(p2Right))
                    {
                        if (CanMoveTo(p2Right))
                        {
                            hasMoved = true;
                            attack.Move(p2Right);
                        }
                    }
                }
            }

            //如果到这里，说明最近的点去不了，那就找其他方向前进，都去不了就原地不动一轮
            if (!hasMoved && !attack.isAttack)
            {
                MoveAction move = this.GetRandomMoveAction(attack, actions);

                switch (move)
                {
                    case MoveAction.Forward:
                        aimAt = new Position(p1.x + 1, p1.y);
                        break;
                    case MoveAction.Backward:
                        aimAt = new Position(p1.x - 1, p1.y);
                        break;
                    case MoveAction.Up:
                        aimAt = new Position(p1.x, p1.y + 1);
                        break;
                    case MoveAction.Down:
                        aimAt = new Position(p1.x, p1.y - 1);
                        break;
                }
                if (move != MoveAction.Stand && CanMoveTo(aimAt))
                {
                    hasMoved = true;
                    attack.Move(aimAt);
                }
            }
            return hasMoved;
        }

        /// <summary>
        /// 判断是否可以移动勇士到某个点
        /// </summary>
        /// <param name="w"></param>
        /// <param name="des"></param>
        /// <returns></returns>
        private bool CanMoveTo(Position des)
        {
            if (des.x < 0 || des.y < 0 || des.x > Config.GRID_COLUMN_COUNT - 1 || des.y > Config.GRID_ROW_COUNT - 1)
            {
                return false;
            }
            if (_posMap.ContainsKey(des))
            {
                return false;
            }
            return true;
        }

        private bool CheckGameOver()
        {
            //如果tick数量过多，则有可能出现bug，对比战力强制战斗结束
            if (_tickNum > Config.MAX_TICK_NUM) {
                float leftForce = this.CalAliveForce(this._warriorLeft);
                float rightForce = this.CalAliveForce(this._warriorRight);
                if (leftForce >= rightForce)
                {
                    this._isWin = true;
                }
                else {
                    this._isWin = false;
                }
                this.ExceedMaxTick = true;
                return true;
            }
            this.ExceedMaxTick = false;
            if (this._warriorLeft.Count == 0 || this._warriorRight.Count == 0) {
                this.CalAliveForce(this._warriorAll);
                if (this._warriorLeft.Count == 0)
                {
                    this._isWin = false;
                }
                else {
                    this._isWin = true;
                }
                
                return true;
            }
            bool allLeftDie = AllWarriorDie(this._warriorLeft);
            bool allRightDie = AllWarriorDie(this._warriorRight);
            if (allLeftDie || allRightDie)
            {
                this.CalAliveForce(this._warriorAll);
                if (allLeftDie)
                {
                    this._isWin = false;
                }
                else {
                    this._isWin = true;
                }
                return true;
            }
            return false;
        }

        private float CalAliveForce(IList<Warrior> list) {
            float totalForce = 0;
            for (int i = 0; i < list.Count; ++i)
            {
                Warrior w = list[i];
                if (w.isAlive)
                {
                    totalForce += w.HP;
                }
            }
            Util.Print("force left:" + totalForce);
            return totalForce;
        }
        private StringBuilder sb = new StringBuilder();
        private void CoordinateData(int tickNo)
        {
            if (this._tickAction.ContainsKey(tickNo))
            {
                IList<TickArgs> list = this._tickAction[tickNo];
                int count = list.Count;
                for (int i = 0; i < count; ++i)
                {
                    TickArgs args = list[i];
                    if (Config.RECORD_LOG)
                    {
                        if (sb.Length > 1) sb.Append("\n");
                        sb.Append(args.ToString());
                    }
                    
                    if (args is VitalityTickArgs)
                    {
                        VitalityTickArgs data = args as VitalityTickArgs;
                        float damage = data.Damage;
                        Warrior w1 = data.Attack;
                        Warrior w2 = data.BeAttack;

                        if (w2.isAlive)
                        {

                            if (Warrior.Counter(w2, w1))
                            {
                                w2.HP = (w2.HP * Config.ATK_ENHANCE_RATIO - damage) / Config.ATK_ENHANCE_RATIO;
                            }
                            else
                            {
                                w2.HP -= damage;
                            }

                            if (w2.HP - 0.0001f <= 0)
                            {
                                w2.HP = 0;
                                w2.Die();
                            }
                        }

                        w1.isAttack = false;
                    }
                    else
                    {
                        
                        PositionTickArgs data = args as PositionTickArgs;
                        Position pos = data.Position;
                        Warrior w = data.Warrior;

                        if (w.isAlive)
                        {
                            w.Pos = pos;
                            if (data.HasArrive)
                            {
                                w.isMove = false;
                            }
                            else {
                                w.isMove = true;
                            }
                        }
                    }


                }
                this._tickAction.Remove(tickNo);
            }
        }

        /// <summary>
        /// 检查勇士列表中是否有活着的
        /// </summary>
        /// <param name="list"></param>
        /// <returns></returns>
        private bool AllWarriorDie(IList<Warrior> list)
        {
            for (int i = 0; i < list.Count; ++i)
            {
                if (list[i].isAlive)
                {
                    return false;
                }
            }
            return true;
        }

        /// <summary>
        /// 根据可以行进的方向推算下次行进方向
        /// </summary>
        /// <param name="warrior"></param>
        /// <param name="actions">本次应该行进的方向，但是由于被占领无法行进</param>
        /// <returns></returns>
        private MoveAction GetRandomMoveAction(Warrior warrior, MoveAction[] actions)
        {
            float[] map = Util.isType(warrior, typeof(Warrior)) ? Config.COMMON_WARRIOR_PATHFIND_RATIO : Config.CHIEF_WARRIOR_PATHFIND_RATIO;
            //加权值策略，stand用加法，上下左右用乘法；比如，如果朝上的话，则stand*3，而下是+0.5, 左右都是*3；stand的话，只给stand*3，其他方向权重不变
            float[] tempMap = new float[] { 0, 0, 0, 0, 0 };
            int length = actions.Length;
            for (int i = 0; i < length; ++i)
            {
                MoveAction action = actions[i];
                float[] f;
                if (i == 0)
                {
                    f = map;
                }
                else
                {
                    f = tempMap;
                }
                tempMap = f.Select<float, float>(delegate(float v, int index)
                {
                    int intAction = (int)action;

                    if (index == (int)MoveAction.Stand)
                    {
                        return v * 3f;
                    }
                    else
                    {
                        if (intAction / 2 == index / 2)//如果是同一个轴上，横向或者纵向
                        {
                            if (intAction == index)
                            {//相同方向的概率为0
                                return 0f;
                            }
                            return v + .5f;//相反方向的概率加0.5，也就是是否朝回走的概率
                        }
                        else
                        {
                            return v * 3f;
                        }
                    }

                }).ToArray();
            }
            length = tempMap.Length;
            float[] newMap = new float[length + 1];//聚合后的区间概率
            newMap[0] = 0;
            float total = tempMap.Sum();

            for (int i = 0; i < length; i++)
            {
                newMap[i + 1] = (float)tempMap[i] / total;
            }

            for (int i = 1; i < newMap.Length; i++)
            {
                newMap[i] += newMap[i - 1];
            }

            float val = (float)_rand.NextDouble();
            
            for (int i = 0; i < length - 1; i++)
            {
                if (val >= newMap[i] && val < newMap[i + 1])
                {
                    
                    return (MoveAction)i;
                }
            }
            return MoveAction.Stand;
        }

        private void WarriorOnMove(MoveEventArgs args)
        {
            //目标tick的时候
            //int tickNo = (int)Math.Ceiling(Config.WARRIOR_MOVE_DURATION / Config.BATTLE_LOGIC_DURATION) + this._flag;
            int tickNo = 2 + this._flag;
            if (!this._tickAction.ContainsKey(tickNo))
            {
                this._tickAction[tickNo] = new List<TickArgs>();
            }
            this._tickAction[tickNo].Add(new PositionTickArgs
            {
                Position = args.To,
                Warrior = args.Warrior,
                HasArrive = true
            });
            
            //下一个tick就把坐标改了，防止坐标不变在移动的时候仍然被攻击
            int nextTick = this._flag + 1;
            if (!this._tickAction.ContainsKey(nextTick))
            {
                this._tickAction[nextTick] = new List<TickArgs>();
            }
            //float x = (args.From.x + args.To.x) / 2;
            //float y = (args.From.y + args.To.y) / 2;
            int x = 1000 + args.From.x;
            int y = 1000 + args.From.y;
            this._tickAction[nextTick].Add(new PositionTickArgs
            {
                Position = new Position(x, y),
                HasArrive = false,
                Warrior = args.Warrior
            });
        }

        private int _tag = 0;
        private TestData getTestData(WarriorType? t, int? force)
        {
            float atk, def, hp;
            int f = 0;
            //WarriorType type = WarriorType.Catapults;
            WarriorType type = t == null ? (WarriorType)_rand.Next(0, 5) : t.Value;
            if (t == null)
            {
                int data = _tag++ / 5;
                switch (data)
                {
                    case 0:
                    case 1:
                    case 2:
                    case 3:
                    case 4:
                    case 5:
                    case 6:
                    case 7:
                    case 8:
                    case 9:
                        TinyWar.Random r = new TinyWar.Random(this._rand.Next(1,100) * data);
                        type = (WarriorType)r.Next(0, 5);
                        break;
                }
            }
            else
            {
                type = t.Value;
            }
            if (Config.USE_FORCE)
            {
                //force = _rand.Next(1500, 2501);
                f = force.Value;
                atk = 0;
                def = 0;
                hp = f;
            }
            else
            {
                hp = _rand.Next(10, 20);
                atk = _rand.Next(20, 25);
                def = _rand.Next(15, 20);
            }


            int atkDist = (type == WarriorType.Archers || type == WarriorType.Catapults ? 2 : 1);
            return new TestData
            {
                hp = hp,
                atk = atk,
                def = def,
                atkDist = atkDist,
                type = type,
                force = hp
            };
        }

        #region 计算双方的队伍坐标
        private delegate int[] CalculateWarriorCoords(IList<Warrior> list);

        /// <summary>
        /// 计算左侧队伍的起止坐标
        /// </summary>
        /// <param name="list"></param>
        /// <returns>返回2个元素的数组，分别代表队伍开始x，结束x</returns>
        private int[] CalculateLeftCoords(IList<Warrior> list)
        {
            int[] result = new int[2];
            int maxX = Config.GRID_COLUMN_COUNT / 2;
            int minX;
            maxX -= Config.MIN_HALF_SPACE;
            int warriorSpace = (int)Math.Ceiling((double)list.Count / Config.GRID_ROW_COUNT);
            minX = maxX - warriorSpace;
            result[0] = minX;
            result[1] = maxX;
            return result;
        }

        /// <summary>
        /// 计算过右侧队伍的起止坐标
        /// </summary>
        /// <param name="list"></param>
        /// <returns>返回2个元素的数组，分别代表队伍开始x，结束x</returns>
        private int[] CalculateRightCoords(IList<Warrior> list)
        {
            int[] result = new int[2];
            int minX = Config.GRID_COLUMN_COUNT / 2 - 1;
            int maxX;
            minX += Config.MIN_HALF_SPACE;
            int warriorSpace = (int)Math.Ceiling((double)list.Count / Config.GRID_ROW_COUNT);
            maxX = minX + warriorSpace;
            result[0] = minX;
            result[1] = maxX;
            return result;
        }

        private int WarriorSort(BattleWarrior w1, BattleWarrior w2) {
            int pos1 = (w1.TeamID - 1) * Config.MAX_WARRIOR_SPACE + w1.Sequence - 1;
            int pos2 = (w2.TeamID - 1) * Config.MAX_WARRIOR_SPACE + w2.Sequence - 1;
            if (pos1 > pos2) { 
                return 1;
            }
            else if (pos1 < pos2) {
                return -1;
            }
            return 0;
        }
        #endregion
    }

    public struct TestData
    {
        public float hp;
        public float atk;
        public int atkDist;
        public float def;
        public float force;
        public WarriorType type;
    }
}
