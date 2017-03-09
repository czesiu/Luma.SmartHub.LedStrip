using NAudio.CoreAudioApi;
using System;
using System.Drawing;
using System.Linq;
using System.Threading;
using System.Threading.Tasks;

namespace Luma.SmartHub.LedStrip.Audio
{
    public class AudioAnimation : IDisposable
    {
        private readonly IModuleConnection _moduleConnection;
        private readonly MMDevice _device;
        private CancellationTokenSource _cancellationTokenSource;

        public AudioAnimation(IModuleConnection moduleConnection)
        {
            _moduleConnection = moduleConnection;

            var deviceEnumerator = new MMDeviceEnumerator();

            var devices = deviceEnumerator.EnumerateAudioEndPoints(DataFlow.All, DeviceState.Active);

            _device = devices[0];
        }

        public void Start()
        {
            _cancellationTokenSource = new CancellationTokenSource();

            var maxPeek = _moduleConnection.LedCount * 0.7;
            Task.Run(() =>
            {
                while (!_cancellationTokenSource.IsCancellationRequested)
                {
                    var leds = (int)(Math.Round(_moduleConnection.LedCount * _device.AudioMeterInformation.MasterPeakValue));

                    var colors = Enumerable.Range(0, leds).Select(range => Color.FromArgb(60, range > maxPeek ? 0 : 60, 0))
                        .Concat(Enumerable.Range(leds, _moduleConnection.LedCount - leds).Select(range => Color.Black)).ToArray();

                    _moduleConnection.WriteColors(colors);

                    Thread.Sleep(20);
                }
            }, _cancellationTokenSource.Token);
        }

        public void Dispose()
        {
            _cancellationTokenSource?.Dispose();
        }
    }
}
