using System;

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

            //var moduleConnection = new HttpModuleConnection("http://192.168.0.103/led-strip")
            //{
            //    ErrorMode = ErrorMode.SendAndForget,
            //    LedCount = 12
            //};

            var moduleConnection = new UdpModuleConnection("192.168.137.236", 8888)
            {
                LedCount = 20,
                ErrorMode = ErrorMode.RetryOnError
            };

            moduleConnection.Connect();

            using (moduleConnection)
            using (var audioAnimation = new StroboscopeAnimation(moduleConnection))
            {
                audioAnimation.Start();

                Console.ReadKey();
            }
        }
    }
}
