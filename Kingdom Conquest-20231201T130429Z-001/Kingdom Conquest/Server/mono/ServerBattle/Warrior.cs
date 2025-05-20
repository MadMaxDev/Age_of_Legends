using System.Collections;
using System.Collections.Generic;
using System;

namespace TinyWar {

    public class Warrior
    {
        #region Field
        /// <summary>
        /// 士兵真实的位置信息
        /// </summary>
        protected Position _pos;
        protected IList<Position> _attackGrid;
        private float _hp = 0;
        #endregion

        #region Properties
        /// <summary>
        /// 攻击力
        /// </summary>
        public float Atk;
        /// <summary>
        /// 防御力
        /// </summary>
        public float Def;
        /// <summary>
        /// 攻击距离
        /// <remarks>只能向前方攻击，攻击后方需要转身</remarks>
        /// </summary>
        public int AtkDistance;
        /// <summary>
        /// 血量
        /// </summary>
        public float HP {
            get { return this._hp; }
            set {
                if (this._hp != 0 && this._hp != value && this.OnHPChanged != null)
                {
                    this.OnHPChanged(value);
                }
                this._hp = value;
            }
        }
        /// <summary>
        /// 名称
        /// </summary>
        public string Name;
        /// <summary>
        /// ID
        /// </summary>
        public ulong ID;
        /// <summary>
        /// 战力
        /// </summary>
        public float Force;
        /// <summary>
        /// 格子坐标，x轴从左到右依次增加，y轴从上到下依次增加
        /// </summary>
        public Position Pos { get; set; }
        /// <summary>
        /// 目标格子坐标
        /// </summary>
        public Position AimPos {
            get { return this._pos; }
            set
            {
                this._placeDiscard(this._pos);
                this._pos = value;
                this._placeHold(value);
            }
        }
        /// <summary>
        /// 是否正在攻击
        /// </summary>
        public bool isAttack;
        /// <summary>
        /// 是否正在移动
        /// </summary>
        public bool isMove;
        /// <summary>
        /// 是否存活
        /// </summary>
        public bool isAlive;
        /// <summary>
        /// 目前的朝向
        /// </summary>
        public Direction Face;
        /// <summary>
        /// 兵种
        /// </summary>
        public WarriorType ArmType;
        /// <summary>
        /// 队伍类型，左边还是右边
        /// </summary>
        public TagType Tag;
        /// <summary>
        /// 士兵占领记录表，表明某个Vector2节点是否已经被占领
        /// </summary>
        public static Dictionary<Position, TagType> GridMap { get; set; }

        /// <summary>
        /// 士兵的可攻击的格子
        /// </summary>
        public IList<Position> AttackGrid {
            get { return this._attackGrid; }
        }

        public Warrior Attacked { get; set; }
        #endregion

        #region Method
        public Warrior() {
            this._attackGrid = new List<Position>();
        }
        /// <summary>
        /// 攻击
        /// </summary>
        /// <param name="enemy"></param>
        public float Attack(Warrior enemy) {
            this.TurnFace(enemy.Pos);
            this.isAttack = true;
            this.isMove = false;
            
            float damage = this.Atk;
            bool loged = (this.Name == "Left1" && enemy.Name == "Right15") || (this.Name == "Right15" && enemy.Name == "Left1");
            int dist = Util.Distance(this.Pos, enemy.Pos);
            
            if (this.Attacked == null || (this.Attacked !=null && this.Attacked != enemy)) {
                bool b1 = Warrior.Counter(this, enemy);
                bool b2 = Warrior.Counter(enemy, this);

                //如果要攻击的敌人正在和别人对战，则本次攻击为0
                if (enemy.Attacked != null && enemy.Attacked.isAlive && enemy.Attacked.Attacked == enemy)
                {
                    damage = 0;
                    goto pass;
                }
                this.Attacked = enemy;
                enemy.Attacked = this;

                float actualForce = 0f;
                int ratio = 4;
                float minHp = Math.Min(this.HP, enemy.HP);
                //if (loged) { 
                //    Util.Print(string.Format("0hp is {0}, 1hp is {1}", this.HP, enemy.HP));
                //    Util.Print("minhp is "+ minHp);
                //}
                
                //if (b1 && this.HP <= enemy.HP)
                //{
                //    float m1 = this.HP * Config.ATK_ENHANCE_RATIO;
                //    ratio = this._calculateAttackRatio(m1, enemy.HP);
                //}
                //else if (b2 && enemy.HP <= this.HP)
                //{
                //    float m2 = enemy.HP * Config.ATK_ENHANCE_RATIO;
                //    ratio = this._calculateAttackRatio(m2, this.HP);
                //}
                //else {
                //    ratio = this._calculateAttackRatio(this.HP, enemy.HP);
                //}
                //damage = minHp / ratio;
                //if(loged)
                //    Util.Print("damage1 is " + damage);
                //if (b1 || b2) {
                //    damage *= Config.ATK_ENHANCE_RATIO;
                //}

                ////如果我方的hp不到原来战力的1/4，并且我克制以后也不到对方战力的1/4，那么本轮就死亡
                //if (this.HP <= this.Force / 4)
                //{
                //    if (b1)
                //    {
                //        actualForce = this.HP * Config.ATK_ENHANCE_RATIO;
                //        if (actualForce <= enemy.HP / 4)
                //        {
                //            damage = actualForce;
                //        }
                //        else
                //        {
                //            damage = actualForce / ratio;
                //        }
                //    }
                //    else
                //    {
                //        damage = this.HP;
                //    }
                //}
                //if (loged)
                //    Util.Print("damage2 is " + damage);
                //if (enemy.HP <= enemy.Force / 4)
                //{
                //    if (b2)
                //    {
                //        actualForce = enemy.HP * Config.ATK_ENHANCE_RATIO;
                //        if (actualForce <= this.HP / 4)
                //        {
                //            damage = actualForce;
                //        }
                //        else
                //        {
                //            damage = actualForce / ratio;
                //        }
                //    }
                //    else
                //    {
                //        damage = enemy.HP;
                //    }
                //}
                //if (loged)
                //    Util.Print("damage3 is " + damage);
                float myForce = this.HP, eneyForce = enemy.HP;
                if (b1) {
                    myForce *= Config.ATK_ENHANCE_RATIO;
                }
                if (b2) {
                    eneyForce *= Config.ATK_ENHANCE_RATIO;
                }
                ratio = this._calculateAttackRatio(myForce, eneyForce);
                
                actualForce = minHp/ratio;//实际伤害
                if (minHp == this.HP && b1) {
                    actualForce *= Config.ATK_ENHANCE_RATIO;
                }
                if (minHp == enemy.HP && b2) {
                    actualForce *= Config.ATK_ENHANCE_RATIO;
                }

                if (this.HP <= enemy.HP / 10  && this.HP <= this.Force / 5) { //秒杀
                    actualForce = this.HP;
                    if (b1) { 
                        actualForce *= Config.ATK_ENHANCE_RATIO;
                    }
                }
                if (enemy.HP <= this.HP / 10 && enemy.HP <= enemy.Force / 5) { 
                    actualForce = enemy.HP;
                    if (b2) { 
                        actualForce *= Config.ATK_ENHANCE_RATIO;
                    }
                }

                damage = actualForce;
            }
            this.Atk = damage;
            enemy.Atk = damage;
            pass:
            //如果是远程的，并且敌人在一格以外，则攻击力变弱
            if (dist > 1 && enemy.AtkDistance == 1)
            {
                damage *= Config.REMOTE_ATTACK_RATIO;
            }
            
            if (this.OnAttackBegin != null) {
                this.OnAttackBegin(new AttackEventArgs (this, enemy));
            }


            return damage;
        }

        /// <summary>
        /// 休息
        /// </summary>
        public void StandBy() {
            if (this.OnStandBy != null) {
                this.OnStandBy();
            }
        }
        /// <summary>
        /// 死亡
        /// </summary>
        public void Die() {
            this.isAttack = false;
            this.isMove = false;
            this.isAlive = false;
            this._placeDiscard(this.AimPos);
            if (this.OnDie != null) {
                this.OnDie();
            }
        }
        
        /// <summary>
        /// 移动，移动的时候当前Pos不变，只是更新AimPos，等移动完毕以后再更新Pos
        /// </summary>
        /// <param name="pos">目的地</param>
        public void Move(Position pos) {
            this.isMove = true;
            this.isAttack = false;
            this.TurnFace(pos);
            if (this.OnMove != null) {
                this.OnMove(new MoveEventArgs(this, this.Pos, pos));
            }
            this.AimPos = pos;
        }

        /// <summary>
        /// 改变行军朝向
        /// </summary>
        /// <param name="pos">要攻击目标的方向</param>
        public void TurnFace(Position pos) {
            Direction newDirection = this.Face;
            if (this.Pos.x < pos.x) { 
                newDirection = Direction.Right;
            }
            else if (this.Pos.x > pos.x) {
                newDirection = Direction.Left;
            }
            if (newDirection != this.Face) {
                this.Face = newDirection;
                if (this.OnFaceChanged != null) {
                    this.OnFaceChanged(newDirection);
                }
            }
        }

        public override string ToString()
        {
            return string.Format(
                "Warrior= name:{0}, tag:{1}, face:{2}, isalive:{3}, isattack:{4}, ismove:{5}, pos:{6}",
                this.Name, this.Tag, this.Face, this.isAlive, this.isAttack, this.isMove, this.Pos
            );
        }

        /// <summary>
        /// 判断是否可以攻击到某个人
        /// </summary>
        /// <param name="pos">被判断的坐标</param>
        /// <returns></returns>
        public virtual bool CanAttack(Position pos) {
            for (int i = 0; i < this._attackGrid.Count; ++i) {
                if (pos == this._attackGrid[i]) {
                    return true;
                }
            }
            return false;
        }

        /// <summary>
        /// w1是否克制w2
        /// </summary>
        /// <param name="w1"></param>
        /// <param name="w2"></param>
        /// <returns></returns>
        public static bool Counter(Warrior w1, Warrior w2)
        {
            int typeInt = (int)w1.ArmType;
            WarriorType[] counterMap = Config.WARRIOR_COUNTER_MAP[typeInt];
            for (int i = 0; i < counterMap.Length; i++)
            {
                if (counterMap[i] == w2.ArmType)
                {
                    return true;
                }
            }
            return false;
        }

        private int _calculateAttackRatio(float hp1, float hp2) { 
            int m1 = (int)(hp1 / hp2);
            int m2 = (int)(hp2 / hp1);
            int m = Math.Max(m1, m2);
            if (m <= 2  && m > 0) {
                return 4;
            }
            else if (m > 2 && m <= 4)
            {
                return 3;
            }
            else {
                return 2;
            }
        }

        /// <summary>
        /// 占据格子
        /// </summary>
        /// <param name="pos"></param>
        private void _placeHold(Position pos){
            if (!Warrior.GridMap.ContainsKey(pos)) {
                Warrior.GridMap.Add(pos, this.Tag);
            }
            else
                Warrior.GridMap[pos] = this.Tag;
            this._updateAttackGrid();
        }

        /// <summary>
        /// 放弃格子
        /// </summary>
        /// <param name="pos"></param>
        private void _placeDiscard(Position pos) {
            if (Warrior.GridMap.ContainsKey(pos))
            {
                Warrior.GridMap.Remove(pos);
            }
        }

        /// <summary>
        /// 计算可攻击格子，普通士兵只能攻击一条直线上 AtkDistance内的敌人
        /// </summary>
        /// <returns></returns>
        protected virtual void _updateAttackGrid() {
            this._attackGrid.Clear();
            for (int i = 0; i < this.AtkDistance; ++i) {//前后都要计算
                this._attackGrid.Add(new Position(this._pos.x + (i + 1), this._pos.y));
                this._attackGrid.Add(new Position(this._pos.x - (i + 1), this._pos.y));
            }
        }
        #endregion

        #region Event
        /// <summary>
        /// 朝向发生变化
        /// </summary>
        public event OnFaceChangedDelegate OnFaceChanged;
        /// <summary>
        /// 攻击开始
        /// </summary>
        public event OnAttackBeginDelegate OnAttackBegin;
        /// <summary>
        /// 死亡
        /// </summary>
        public event OnDieDelegate OnDie;
        /// <summary>
        /// 开始移动
        /// </summary>
        public event OnMoveDelegate OnMove;
        /// <summary>
        /// 巡逻
        /// </summary>
        public event OnStandByDelegate OnStandBy;
        /// <summary>
        /// HP发生变化
        /// </summary>
        public event OnHPChangedDelegate OnHPChanged;
        #endregion
    }

    #region Delegate and EventArgs
    public delegate void OnFaceChangedDelegate(Direction args);
    public delegate void OnAttackBeginDelegate(AttackEventArgs args);
    public delegate void OnDieDelegate();
    public delegate void OnStandByDelegate();
    public delegate void OnMoveDelegate(MoveEventArgs args);
    public delegate void OnHPChangedDelegate(float hp);

    public class AttackEventArgs
    {
        public AttackEventArgs(Warrior attacked, Warrior beAttacked) {
            this.Attacked = attacked;
            this.BeAttacked = beAttacked;
        }
        /// <summary>
        /// 攻击方
        /// </summary>
        public Warrior Attacked { get; set; }
        /// <summary>
        /// 被攻击方
        /// </summary>
        public Warrior BeAttacked { get; set; }

    }

    public struct MoveEventArgs {
        public Position From;
        public Position To;
        public Warrior Warrior;
        public MoveEventArgs(Warrior w, Position from, Position to) {
            this.Warrior = w;
            this.From = from;
            this.To = to;
        }
    }
    #endregion
}
