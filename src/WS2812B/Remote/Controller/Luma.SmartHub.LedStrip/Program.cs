using System.Drawing;
using System.Linq;
using System.Threading;

namespace Luma.SmartHub.LedStrip
{
    class Program
    {
        private static IModuleConnection _moduleConnection;

        static void Main(string[] args)
        {
            try
            {
                var wait = 0;
                //_moduleConnection = new SerialPortModuleConnection("COM3", 250000)
                //{
                //    LedCount = 159,
                //    ErrorMode = ErrorMode.RetryOnError
                //};

                //_moduleConnection = new HttpModuleConnection("http://192.168.0.103/led-strip")
                //{
                //    LedCount = 12
                //};

                //_moduleConnection = new TcpModuleConnection("192.168.137.134", 23)
                //{
                //    LedCount = 159,
                //    ErrorMode = ErrorMode.SendAndForget
                //};

                _moduleConnection = new UdpModuleConnection("192.168.137.76", 8888)
                {
                    LedCount = 159,
                    ErrorMode = ErrorMode.RetryOnError
                };

                _moduleConnection.Connect();

                while (true)
                {
                    // Some example procedures showing how to display to the pixels:
                    ColorWipe(Color.Red, _moduleConnection.LedCount, wait);
                    ColorWipe(Color.Green, _moduleConnection.LedCount, wait);
                    ColorWipe(Color.Blue, _moduleConnection.LedCount, wait);

                    // Send a theater pixel chase in...
                    TheaterChase(Color.White.WithBrightness(127), wait);
                    TheaterChase(Color.Red.WithBrightness(127), wait);
                    TheaterChase(Color.Blue.WithBrightness(127), wait);

                    Rainbow(0);
                    RainbowCycle(0);
                    TheaterChaseRainbow(wait);

                    Detonate(Color.White, 1000);
                }
            }
            finally
            {
                _moduleConnection?.Dispose();
            }
        }

        private static void ColorWipe(Color color, int leds, int wait)
        {
            for (var j = 0; j <= leds; j++)
            {
                ColorWipeInternal(color, j);

                Thread.Sleep(wait);
            }
        }

        private static void ColorWipeInternal(Color color, int leds)
        {
            var colors = Enumerable.Range(0, leds).Select(range => color)
                .Concat(Enumerable.Range(leds, _moduleConnection.LedCount - leds).Select(range => Color.Black)).ToArray();

            _moduleConnection.WriteColors(colors);
        }

        private static void TheaterChase(Color color, int wait)
        {
            for (var j = 0; j < 10; j++) // do 10 cycles of chasing
            {  
                for (var q = 0; q < 3; q++)
                {
                    var colors = Enumerable.Range(0, _moduleConnection.LedCount).Select(i =>
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
                    var colors = Enumerable.Range(0, _moduleConnection.LedCount).Select(i =>
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
                var colors = Enumerable.Range(0, _moduleConnection.LedCount).Select(i => Wheel((byte)((i + j) & 255))).ToArray();

                _moduleConnection.WriteColors(colors);

                Thread.Sleep(wait);
            }
        }

        private static void RainbowCycle(int wait)
        {
            for (var j = 0; j < 256 * 5; j++) // 5 cycles of all colors on wheel
            {
                var colors = Enumerable.Range(0, _moduleConnection.LedCount).Select(i => Wheel((byte)(((i * 256 / _moduleConnection.LedCount) + j) & 255))).ToArray();

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
