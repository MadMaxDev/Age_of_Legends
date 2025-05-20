using System;
using System.Collections;
using System.Collections.Generic;
namespace TinyWar
{
    public enum CounterType { 
        Counter,
        BeCounter
    }
    /// <summary>
    /// 勇士tag类型，目前只有左右两种
    /// </summary>
    public enum TagType
    {
        Left = 0,
        Right
    }

    /// <summary>
    /// 勇士类型
    /// </summary>
    public enum WarriorType { 
        Archers,
        Cavalrys,
        Catapults,
        Lancers,
        Swordsmen,
        LeaderSwordsmen
    }

    /// <summary>
    /// 勇士攻击效果，目前只有弓箭手和投石器带攻击效果，分别是弓箭和抛石
    /// </summary>
    public enum AttackStyle { 
        Arrow,
        Rock
    }

    /// <summary>
    /// 勇士的朝向，朝向和移动方向一致
    /// </summary>
    public enum Direction { 
        Left = 0,
        Right
    }

    /// <summary>
    /// 勇士行动类型
    /// </summary>
    public enum Action { 
        Attack,
        Move,
        Die
    }

    /// <summary>
    /// 寻路类型
    /// </summary>
    public enum MoveAction { 
        Forward = 0,
        Backward,
        Up,
        Down,
        Stand
    }

    /// <summary>
    /// 格子坐标
    /// </summary>
    public struct Position
    {
        public Position(int x, int y) {
            this.x = x;
            this.y = y;
        }

        public static bool operator ==(Position p1, Position p2)
        {

            return p1.x == p2.x && p1.y == p2.y;
        }

        public static bool operator !=(Position p1, Position p2)
        {

            return p1.x != p2.x || p1.y != p2.y;
        }

        public override bool Equals(object obj)
        {
            if (obj is Position)
            {
                return this == (Position)obj;
            }
            return false;
        }

        public override string ToString()
        {
            return string.Format("x:{0},y:{1}", this.x, this.y);
        }

        public override int GetHashCode()
        {
            return this.x.GetHashCode() + this.y.GetHashCode();
        }
        /// <summary>
        /// 格子x坐标
        /// </summary>
        public int x;
        /// <summary>
        /// 格子y坐标
        /// </summary>
        public int y;
    }

    public class PositionEqual : IEqualityComparer<Position> {
        bool IEqualityComparer<Position>.Equals(Position x, Position y)
        {
            return x == y;
        }

        int IEqualityComparer<Position>.GetHashCode(Position obj)
        {
            return obj.GetHashCode();
        }
    }

    /// <summary>
    /// 主tick判断勇士当前tick类型，生命值还是位置
    /// </summary>
    public enum TickAction { 
        Vitality,   //生命值变化
        Position    //位置移动
    }

    public class TickArgs {
    }

    public class VitalityTickArgs : TickArgs {
        public Warrior Attack;
        public Warrior BeAttack;
        public float Damage;
        public override string ToString()
        {
            return string.Format("attack:{0}\t\t\tbeAttack:{1}\t\t\tdamage:{2}", this.Attack, this.BeAttack, this.Damage);
        }
    }

    public class PositionTickArgs : TickArgs
    {
        public Warrior Warrior;
        //当前位置
        public Position Position;
        //是否已经到达
        public bool HasArrive;
        public override string ToString()
        {
            return string.Format("warrior:{0}\t\t\tposition:{1}\t\t\tarriave:{2}", this.Warrior, this.Position, this.HasArrive);
        }
    }

    public class BattleWarrior {
        public ulong ID;
        public string Name;
        /// <summary>
        /// 第几队
        /// </summary>
        public int TeamID;
        /// <summary>
        /// 序号
        /// </summary>
        public int Sequence;
        /// <summary>
        /// 战力
        /// </summary>
        public float Force;
        public WarriorType ArmType;
        public override string ToString()
        {
            return string.Format("Warrior:TeamID-{0},\tSequence-{1},\tForce-{2},Type-\t{3}", TeamID, Sequence, Force, ArmType.ToString());
        }
    }
}