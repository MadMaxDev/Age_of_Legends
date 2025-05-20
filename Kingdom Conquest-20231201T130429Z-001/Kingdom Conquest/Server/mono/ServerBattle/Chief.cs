using System;
using System.Collections.Generic;

namespace TinyWar
{
    public class Chief : Warrior
    {
        /// <summary>
        /// 将领攻击，可以攻击上面一行和同一行AtkDistance内的敌人
        /// </summary>
        protected override void _updateAttackGrid()
        {
            if (this._attackGrid == null)
            {
                this._attackGrid = new List<Position>();
            }
            else
            {
                this._attackGrid.Clear();
            }
            for (int i = 0; i < this.AtkDistance; ++i)
            {
                this._attackGrid.Add(new Position(this._pos.x + (i + 1), this._pos.y));
                this._attackGrid.Add(new Position(this._pos.x - (i + 1), this._pos.y));
                this._attackGrid.Add(new Position(this._pos.x + (i + 1), this._pos.y + 1));
                this._attackGrid.Add(new Position(this._pos.x - (i + 1), this._pos.y - 1));
            }
        }
    }
}
