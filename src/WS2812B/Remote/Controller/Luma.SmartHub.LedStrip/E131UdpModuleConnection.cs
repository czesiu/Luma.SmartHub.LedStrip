using System;
using System.Diagnostics;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Net.Sockets;
using System.Timers;
using Be.IO;

namespace Luma.SmartHub.LedStrip
{
    public class E131UdpModuleConnection : IModuleConnection
    {

        private readonly string _url;

        private UdpClient _udpClient;
        private readonly Timer _timer;
        private int _requests;
        private readonly int _port;
        private byte _sequenceNumber;

        public E131UdpModuleConnection(string url, int port = 5568)
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
            _sequenceNumber = 0;
        }

        public ErrorMode ErrorMode
        {
            get { return ErrorMode.SendAndForget; }
            set
            {
                throw new InvalidOperationException("E131 protocol does not support response. Only 'ErrorMode.SendAndForget' is supported");
            }
        }

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
            var sw = new Stopwatch();
            sw.Start();

            var bytes = PrepareBytes(colors);

            var sendBytes = _udpClient.Send(bytes, bytes.Length);

            Console.WriteLine("Sent {0} bytes in {1} ms", sendBytes, sw.ElapsedMilliseconds);

            _requests++;
            _sequenceNumber++;
        }

        private byte[] PrepareBytes(Color[] colors)
        {
            var colorValues = colors
                .SelectMany(c => new[] { c.B, c.R, c.G })
                .ToArray();

            var packet = new E131Packet(_sequenceNumber, 1, colorValues);
            var memoryStream = new MemoryStream();
            var binarys = new BeBinaryWriter(memoryStream);

            /* Root Layer */
            binarys.Write(packet.preamble_size);
            binarys.Write(packet.postamble_size);
            binarys.Write(packet.acn_id);
            binarys.Write(packet.root_flength);
            binarys.Write(packet.root_vector);
            binarys.Write(packet.cid.ToByteArray());

            /* Frame Layer */
            binarys.Write(packet.frame_flength);
            binarys.Write(packet.frame_vector);
            binarys.Write(packet.source_name);
            binarys.Write(packet.priority);
            binarys.Write(packet.reserved);
            binarys.Write(packet.sequence_number);
            binarys.Write(packet.options);
            binarys.Write(packet.universe);

            /* DMP Layer */
            binarys.Write(packet.dmp_flength);
            binarys.Write(packet.dmp_vector);
            binarys.Write(packet.type);
            binarys.Write(packet.first_address);
            binarys.Write(packet.address_increment);
            binarys.Write(packet.property_value_count);
            binarys.Write(packet.property_values);

            return memoryStream.GetBuffer();
        }

        private class E131Packet
        {
            public UInt16 preamble_size;
            public UInt16 postamble_size;
            public Byte[] acn_id;
            public UInt16 root_flength;
            public UInt32 root_vector;
            public Guid cid;

            /* Frame Layer */
            public UInt16 frame_flength;
            public UInt32 frame_vector;
            public Byte[] source_name;
            public Byte priority;
            public UInt16 reserved;
            public Byte sequence_number;
            public Byte options;
            public UInt16 universe;

            /* DMP Layer */
            public UInt16 dmp_flength;
            public Byte dmp_vector;
            public Byte type;
            public UInt16 first_address;
            public UInt16 address_increment;
            public UInt16 property_value_count;
            public Byte[] property_values;

            public E131Packet(byte sequenceNumber, UInt16 universe, byte[] propertyValues)
            {
                this.acn_id = new byte[] { 0x41, 0x53, 0x43, 0x2d, 0x45, 0x31, 0x2e, 0x31, 0x37, 0x00, 0x00, 0x00 };
                this.source_name = new byte[64];

                this.root_vector = 4;
                this.frame_vector = 2;
                this.dmp_vector = 2;

                if (propertyValues.Length > 512)
                    throw new ArgumentException("Max 'propertyValues' length is 512");

                this.sequence_number = sequenceNumber;
                this.universe = universe;

                this.property_value_count = (UInt16)propertyValues.Length;
                this.property_values = propertyValues;

            }
        }
        public void Dispose()
        {
            _timer.Dispose();

            Disconnect();
        }
    }
}
