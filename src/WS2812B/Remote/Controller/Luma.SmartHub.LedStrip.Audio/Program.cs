using System;
using System.Drawing;

namespace Luma.SmartHub.LedStrip.Audio
{
    class Program
    {
        static void Main(string[] args)
        {
            //var moduleConnection = new SerialPortModuleConnection("COM5", 74880)
            //{
            //    ErrorMode = ErrorMode.SendAndForget,
            //    LedCount = 12
            //};

            //var moduleConnection = new HttpModuleConnection("http://192.168.0.108/led-strip")
            //{
            //    ErrorMode = ErrorMode.SendAndForget,
            //    LedCount = 100
            //};

            var moduleConnection = new UdpModuleConnection("192.168.0.108", 8888)
            {
                LedCount = 15,
                ErrorMode = ErrorMode.RetryOnError
            };

            //var moduleConnection = new E131UdpModuleConnection("192.168.0.106")
            //{
            //    LedCount = 11
            //};

            moduleConnection.Connect();

            using (moduleConnection)
            using (var audioAnimation = new AudioAnimation(moduleConnection))
            {
                audioAnimation.Start();

                Console.ReadKey();
            }
        }
    }
}
