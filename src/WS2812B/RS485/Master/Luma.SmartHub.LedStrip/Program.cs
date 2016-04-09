using System.Drawing;
using System.Linq;
using System.Threading;

namespace Luma.SmartHub.LedStrip
{
    class Program
    {
        private static ModuleConnection _moduleConnection;
        private static int _ledCount = 12;

        static void Main(string[] args)
        {
            try
            {
                _moduleConnection = new ModuleConnection("COM5", 74880);

                _moduleConnection.Connect();

                while (true)
                {
                    // Some example procedures showing how to display to the pixels:
                    ColorWipe(Color.Red, 50);
                    ColorWipe(Color.Green, 50);
                    ColorWipe(Color.Blue, 50);

                    // Send a theater pixel chase in...
                    TheaterChase(Color.White.WithBrightness(127), 50);
                    TheaterChase(Color.Red.WithBrightness(127), 50);
                    TheaterChase(Color.Blue.WithBrightness(127), 50);

                    Rainbow(0);
                    RainbowCycle(0);
                    TheaterChaseRainbow(50);

                    Detonate(Color.White, 1000);
                }
            }
            finally
            {
                _moduleConnection?.Dispose();
            }
        }

        private static void ColorWipe(Color color, int wait)
        {
            for (var j = 0; j <= _ledCount; j++)
            {
                var colors = Enumerable.Range(0, j).Select(range => color)
                    .Concat(Enumerable.Range(j, _ledCount).Select(range => Color.Black)).ToArray();

                _moduleConnection.WriteColors(colors);

                Thread.Sleep(wait);
            }
        }

        private static void TheaterChase(Color color, int wait)
        {
            for (var j = 0; j < 10; j++) // do 10 cycles of chasing
            {  
                for (var q = 0; q < 3; q++)
                {
                    var colors = Enumerable.Range(0, _ledCount).Select(i =>
                    {
                        if (i % 3 == q)
                        {
                            return color;
                        }

                        return Color.Black;
                    }).ToArray();

                    _moduleConnection.WriteColors(colors);

                    Thread.Sleep(wait);
                }
            }
        }

        private static void TheaterChaseRainbow(int wait)
        {
            for (var j = 0; j < 256; j++) // cycle all 256 colors in the wheel
            {
                for (var q = 0; q < 3; q++)
                {
                    var colors = Enumerable.Range(0, _ledCount).Select(i =>
                    {
                        if (i % 3 == q)
                        {
                            return Wheel((byte)((i + j) & 255));
                        }

                        return Color.Black;
                    }).ToArray();

                    _moduleConnection.WriteColors(colors);

                    Thread.Sleep(wait);
                }
            }
        }

        private static void Rainbow(int wait)
        {
            for (var j = 0; j < 256; j++)
            {
                var colors = Enumerable.Range(0, _ledCount).Select(i => Wheel((byte)((i + j) & 255))).ToArray();

                _moduleConnection.WriteColors(colors);

                Thread.Sleep(wait);
            }
        }

        private static void RainbowCycle(int wait)
        {
            for (var j = 0; j < 256 * 5; j++) // 5 cycles of all colors on wheel
            {
                var colors = Enumerable.Range(0, _ledCount).Select(i => Wheel((byte)(((i * 256 / _ledCount) + j) & 255))).ToArray();

                _moduleConnection.WriteColors(colors);

                Thread.Sleep(wait);
            }
        }

        private static void Detonate(Color color, int sleep)
        {
            _moduleConnection.WriteColor(Color.Black);
            Thread.Sleep(sleep);

            while (sleep > 0)
            {
                _moduleConnection.WriteColor(color); // Flash the color 
                _moduleConnection.WriteColor(Color.Black);

                sleep = (sleep * 4) / 5; // delay between flashes is halved each time until zero

                Thread.Sleep(sleep);
            }

            // Then we fade to black....
            for (byte brightness = 255; brightness > 0; brightness--)
            {
                _moduleConnection.WriteColor(color.WithBrightness(brightness));
            }

            _moduleConnection.WriteColor(Color.Black);
        }

        // Input a value 0 to 255 to get a color value.
        // The colours are a transition r - g - b - back to r.
        private static Color Wheel(byte position)
        {
            position = (byte)(byte.MaxValue - position);

            if (position < 85)
            {
                return Color.FromArgb(255 - position * 3, 0, position * 3);
            }

            if (position < 170)
            {
                position -= 85;
                return Color.FromArgb(0, position * 3, 255 - position * 3);
            }

            position -= 170;
            return Color.FromArgb(position * 3, 255 - position * 3, 0);
        }
    }
}
