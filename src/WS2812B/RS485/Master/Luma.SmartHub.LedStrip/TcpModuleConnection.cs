using System;
using System.Diagnostics;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Timers;
using Nito.KitchenSink.CRC;

namespace Luma.SmartHub.LedStrip
{
    public class TcpModuleConnection : IModuleConnection
    {
        private readonly string _url;

        private TcpClient _tcpClient;
        private readonly Timer _timer;
        private int _requests;
        private readonly int _port;

        public TcpModuleConnection(string url, int port)
        {
            _timer = new Timer(1000);
            _timer.Elapsed += TimerOnElapsed;
            _timer.Start();

            _requests = 0;
            _url = url;
            _port = port;
        }

        private void TimerOnElapsed(object sender, ElapsedEventArgs elapsedEventArgs)
        {
            Console.WriteLine("Req/s: " + _requests);
            _requests = 0;
        }

        public ErrorMode ErrorMode { get; set; }
        public int LedCount { get; set; }

        public void Connect()
        {
            _tcpClient = new TcpClient();
            _tcpClient.Connect(_url, _port);
        }

        public void Disconnect()
        {
        }

        public void WriteColor(Color color)
        {
            WriteColors(color);
        }

        public void WriteColors(params Color[] colors)
        {
            string result;

            do
            {
                result = WriteColorsInternal(colors);

                //Console.WriteLine("Result: " + result);
            }
            while (result != "OK" && ErrorMode == ErrorMode.RetryOnError);

            _requests++;
        }

        private string WriteColorsInternal(Color[] colors)
        {
            var sw = new Stopwatch();
            sw.Start();
            var streamWriter = new BinaryWriter(_tcpClient.GetStream());
            var bytes = colors.SelectMany(c => new[] { c.R, c.G, c.B }).ToArray();
            var crc32 = new CRC32().ComputeHash(bytes);

            streamWriter.Write((short)bytes.Length);
            streamWriter.Write(bytes);
            streamWriter.Write(crc32);
            streamWriter.Flush();

            var sendBytes = bytes.Length * 3 + 6;

            Console.WriteLine("Sent {0} bytes in {1} ms", sendBytes, sw.ElapsedMilliseconds);

            return null;
        }

        public void Dispose()
        {
            _timer.Dispose();

            Disconnect();
        }
    }
}
