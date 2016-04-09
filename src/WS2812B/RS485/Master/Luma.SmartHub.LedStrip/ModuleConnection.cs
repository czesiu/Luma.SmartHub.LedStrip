using System;
using System.Drawing;
using System.IO;
using System.IO.Ports;
using System.Linq;
using Nito.KitchenSink.CRC;

namespace Luma.SmartHub.LedStrip
{
    class ModuleConnection : IDisposable
    {
        private readonly SerialPort _serialPort;

        public ModuleConnection(string serialPortName, int serialPortBaudRate)
        {
            _serialPort = new SerialPort(serialPortName, serialPortBaudRate);
        }

        public void Connect()
        {
            _serialPort.Open();
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
            while (result != "OK\r");
        }

        private string WriteColorsInternal(Color[] colors)
        {
            var streamWriter = new BinaryWriter(_serialPort.BaseStream);
            var bytes = colors.SelectMany(c => new[] { c.R, c.G, c.B }).ToArray();
            var crc32 = new CRC32().ComputeHash(bytes);

            streamWriter.Write((byte)bytes.Length);
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
