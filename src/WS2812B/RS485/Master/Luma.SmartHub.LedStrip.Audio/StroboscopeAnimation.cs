using NAudio.CoreAudioApi;
using System;
using System.Drawing;
using System.Threading;
using System.Threading.Tasks;

namespace Luma.SmartHub.LedStrip.Audio
{
    public class StroboscopeAnimation : IDisposable
    {
        private readonly IModuleConnection _moduleConnection;
        private readonly MMDevice _device;
        private CancellationTokenSource _cancellationTokenSource;

        public StroboscopeAnimation(IModuleConnection moduleConnection)
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
                    Console.WriteLine("Master peak value: " + _device.AudioMeterInformation.MasterPeakValue);

                    var color = _device.AudioMeterInformation.MasterPeakValue > 0.86 ? Color.White : Color.Black;

                    _moduleConnection.WriteColors(color);

                    if (color == Color.White)
                    {
                        Thread.Sleep(13);
                        _moduleConnection.WriteColors(Color.Black);
                    }
                    
                }
            }, _cancellationTokenSource.Token);
        }

        public void Dispose()
        {
            _cancellationTokenSource?.Dispose();
        }
    }
}
