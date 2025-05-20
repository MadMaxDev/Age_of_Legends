using System;
using System.Runtime;
using System.Runtime.InteropServices;

namespace TinyWar
{
    [Serializable]
    ///Mono÷–µƒrandom¿‡
    public class Random
    {
        //
        private int[] SeedArray;
        private int inext;
        private int inextp;

        public Random(int Seed)
        {
            this.SeedArray = new int[56];
            int num;
            if (Seed == -2147483648)
            {
                num = 161803398 - Math.Abs(-2147483647);
            }
            else
            {
                num = 161803398 - Math.Abs(Seed);
            }
            this.SeedArray[55] = num;
            int num2 = 1;
            for (int i = 1; i < 55; i++)
            {
                int num3 = 21 * i % 55;
                this.SeedArray[num3] = num2;
                num2 = num - num2;
                if (num2 < 0)
                {
                    num2 += 2147483647;
                }
                num = this.SeedArray[num3];
            }
            for (int j = 1; j < 5; j++)
            {
                for (int k = 1; k < 56; k++)
                {
                    this.SeedArray[k] -= this.SeedArray[1 + (k + 30) % 55];
                    if (this.SeedArray[k] < 0)
                    {
                        this.SeedArray[k] += 2147483647;
                    }
                }
            }
            this.inext = 0;
            this.inextp = 31;
        }

        public Random()
            : this(Environment.TickCount)
        {
        }

        //
        // Methods
        //

        public virtual int Next(int maxValue)
        {
            if (maxValue < 0)
            {
                throw new ArgumentOutOfRangeException("Max value is less than min value.");
            }
            return (int)(this.Sample() * (double)maxValue);
        }

        public virtual int Next(int minValue, int maxValue)
        {
            if (minValue > maxValue)
            {
                throw new ArgumentOutOfRangeException("Min value is greater than max value.");
            }
            uint num = (uint)(maxValue - minValue);
            if (num <= 1u)
            {
                return minValue;
            }
            return (int)((ulong)((uint)(this.Sample() * num)) + (ulong)((long)minValue));
        }

        public virtual int Next()
        {
            return (int)(this.Sample() * 2147483647.0);
        }

        public virtual void NextBytes(byte[] buffer)
        {
            if (buffer == null)
            {
                throw new ArgumentNullException("buffer");
            }
            for (int i = 0; i < buffer.Length; i++)
            {
                buffer[i] = (byte)(this.Sample() * 256.0);
            }
        }

        public virtual double NextDouble()
        {
            return this.Sample();
        }

        protected virtual double Sample()
        {
            if (++this.inext >= 56)
            {
                this.inext = 1;
            }
            if (++this.inextp >= 56)
            {
                this.inextp = 1;
            }
            int num = this.SeedArray[this.inext] - this.SeedArray[this.inextp];
            if (num < 0)
            {
                num += 2147483647;
            }
            this.SeedArray[this.inext] = num;
            return (double)num * 4.6566128752457969E-10;
        }
    }
}