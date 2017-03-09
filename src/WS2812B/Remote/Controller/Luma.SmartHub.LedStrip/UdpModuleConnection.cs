using System;
using System.Diagnostics;
using System.Drawing;
using System.Linq;
using System.Net.Sockets;
using System.Text;
using System.Threading.Tasks;
using System.Timers;

namespace Luma.SmartHub.LedStrip
{
    public class UdpModuleConnection : IModuleConnection
    {
        private readonly string _url;

        private UdpClient _udpClient;
        private readonly Timer _timer;
        private int _requests;
        private readonly int _port;

        public UdpModuleConnection(string url, int port)
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
            _udpClient = new UdpClient();
            _udpClient.Connect(_url, _port);
        }

        public void Disconnect()
        {
            _udpClient?.Close();
        }

        public void WriteColor(Color color)
        {
            WriteColors(color);
        }

        public void WriteColors(params Color[] colors)
        {
            string result;

            var errorCount = 0;
            do
            {
                result = WriteColorsInternal(colors);

                //Console.WriteLine("Result: " + result);

                errorCount += (result != "OK") ? 1 : 0;

                if (errorCount > 3)
                {
                    Disconnect();
                    Connect();
                }
            }
            while (result != "OK" && ErrorMode == ErrorMode.RetryOnError);

            _requests++;
        }

        private string WriteColorsInternal(Color[] colors)
        {
            var sw = new Stopwatch();
            sw.Start();

            var bytes = colors.SelectMany(c => new[] { c.G, c.R, c.B }).ToArray();

            var sendBytes = _udpClient.Send(bytes, bytes.Length);

            if (ErrorMode == ErrorMode.SendAndForget)
            {
                Console.WriteLine("Sent {0} bytes in {1} ms", sendBytes, sw.ElapsedMilliseconds);
                return null;
            }

            var udpReceiveResult = Task.WhenAny(_udpClient.ReceiveAsync(), Task.Delay(33)).Result as Task<UdpReceiveResult>;

            if (udpReceiveResult != null)
            {
                var result = Encoding.ASCII.GetString(udpReceiveResult.Result.Buffer);

                Console.WriteLine("Sent {0} bytes in {1} ms", sendBytes, sw.ElapsedMilliseconds);

                return result;
            }

            return null;
        }

        public void Dispose()
        {
            _timer.Dispose();

            Disconnect();
        }
    }
}
