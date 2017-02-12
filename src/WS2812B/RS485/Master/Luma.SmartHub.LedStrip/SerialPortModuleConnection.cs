using System;
using System.Drawing;
using System.IO;
using System.IO.Ports;
using System.Linq;
using System.Timers;
using Nito.KitchenSink.CRC;

namespace Luma.SmartHub.LedStrip
{
    public class SerialPortModuleConnection : IModuleConnection
    {
        private readonly SerialPort _serialPort;
        private readonly Timer _timer;
        private int _requests;

        public ErrorMode ErrorMode { get; set; }

        public int LedCount { get; set; }

        public SerialPortModuleConnection(string serialPortName, int serialPortBaudRate)
        {
            _timer = new Timer(1000);
            _timer.Elapsed += TimerOnElapsed;
            _timer.Start();

            _serialPort = new SerialPort(serialPortName, serialPortBaudRate);
        }

        private void TimerOnElapsed(object sender, ElapsedEventArgs elapsedEventArgs)
        {
            Console.WriteLine("Req/s: " + _requests);
            _requests = 0;
        }

        public void Connect()
        {
            _serialPort.Open();

            var readed = "";

            do
            {
                readed += _serialPort.ReadExisting();
            } while (!readed.Contains("READY"));
        }

        public void Write(string s)
        {
            _serialPort.WriteLine(s);
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

                Console.WriteLine("Result: " + result);
            }
            while (!result.Contains("OK\r") && ErrorMode == ErrorMode.RetryOnError);

            _requests++;
        }

        private string WriteColorsInternal(Color[] colors)
        {
            var streamWriter = new BinaryWriter(_serialPort.BaseStream);
            var bytes = colors.SelectMany(c => new[] { c.R, c.G, c.B }).ToArray();
            var crc32 = new CRC32().ComputeHash(bytes);

            streamWriter.Write((ushort)bytes.Length);
            streamWriter.Write(bytes);
            streamWriter.Write(crc32);
            streamWriter.Flush();

            return _serialPort.ReadLine();
        }

        public void Disconnect()
        {
            _serialPort?.Close();
        }

        public void Dispose()
        {
            Disconnect();
        }
    }
}
