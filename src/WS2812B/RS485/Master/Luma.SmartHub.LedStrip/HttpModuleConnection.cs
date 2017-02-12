using System;
using System.Drawing;
using System.Linq;
using System.Net;
using System.Timers;

namespace Luma.SmartHub.LedStrip
{
    public class HttpModuleConnection : IModuleConnection
    {
        private readonly string _url;
        private readonly string _method;

        private WebClient _webClient;
        private readonly Timer _timer;
        private int _requests;

        public HttpModuleConnection(string url, string method = "POST")
        {
            _timer = new Timer(1000);
            _timer.Elapsed += TimerOnElapsed;
            _timer.Start();

            _requests = 0;
            _url = url;
            _method = method;
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
            _webClient = new WebClient();
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
            try
            {
                var bytes = colors.SelectMany(c => new[] { c.R, c.G, c.B }).ToArray();

                var base64String = Convert.ToBase64String(bytes);

                _webClient.UploadString(new Uri(_url), _method, base64String);

                _requests++;
            }
            catch
            {
            }
        }

        public void Dispose()
        {
            _timer.Dispose();

            Disconnect();
        }
    }
}
