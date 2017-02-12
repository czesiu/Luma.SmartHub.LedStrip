using System;
using System.Drawing;

namespace Luma.SmartHub.LedStrip
{
    public interface IModuleConnection : IDisposable
    {
        ErrorMode ErrorMode { get; }

        int LedCount { get; }

        void Connect();
        void Disconnect();
        void WriteColor(Color color);
        void WriteColors(params Color[] colors);
    }
}