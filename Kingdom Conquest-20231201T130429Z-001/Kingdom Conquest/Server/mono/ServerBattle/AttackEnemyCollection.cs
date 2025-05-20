using System;
using System.Collections;
using System.Collections.Generic;

namespace TinyWar
{
    /// <summary>
    /// 可攻击敌人列表
    /// </summary>
    public class AttackEnemyCollection
    {
        private List<Warrior> _list;
        private int _capacity;//最大数量
        private Warrior _warrior;

        public AttackEnemyCollection(int capacity, Warrior warrior) {
            this._capacity = capacity;
            this._list = new List<Warrior>();
            this._warrior = warrior;
        }

        public AttackEnemyCollection(Warrior warrior) : this(Config.NEAR_ENEMY_COUNT, warrior) {
        }

        private void _sort() {
            this._list.Sort(this._compare);
        }

        /// <summary>
        /// 添加一个敌人，如果超过总容量就只保留总容量范围内的数据
        /// </summary>
        /// <param name="w"></param>
        public void Push(Warrior w) {
            this._list.Add(w);
            this._sort();
            int totalCount = this._list.Count;
            if (totalCount > this._capacity) {
                this._list.RemoveAt(this._capacity);
            }
        }

        /// <summary>
        /// 返回攻击列表中距离最近的敌人
        /// </summary>
        /// <returns></returns>
        public Warrior Pop() {
            if (this._list.Count > 0) {
                Warrior result = this._list[0];
                this._list.RemoveAt(0);
                return result;
            }
            return null;
        }

        /// <summary>
        /// 清空列表
        /// </summary>
        public void Clear() {
            this._list.Clear();
            this._list = null;
        }

        private int _compare(Warrior x, Warrior y) {
            int dist1 = Util.Distance(this._warrior.Pos, x.Pos);
            int dist2 = Util.Distance(this._warrior.Pos, y.Pos);

            if (dist1 > dist2)
            {
                return 1;
            }
            else if (dist1 < dist2)
            {
                return -1;
            }
            else
            {
                if (x.Pos.x > y.Pos.x)
                {
                    return 1;
                }
                else if (x.Pos.x < y.Pos.x)
                {
                    return -1;
                }
                else
                {
                    if (x.Pos.y > y.Pos.y)
                    {
                        return 1;
                    }
                    else if (x.Pos.y < y.Pos.y)
                    {
                        return -1;
                    }
                }
            }
            return 0;
        }
    }
}
