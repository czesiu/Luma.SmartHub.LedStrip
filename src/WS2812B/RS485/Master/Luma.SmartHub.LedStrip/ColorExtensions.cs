using System.Drawing;

namespace Luma.SmartHub.LedStrip
{
    public static class ColorExtensions
    {
        public static Color WithBrightness(this Color color, byte brightness)
        {
            return Color.FromArgb((color.R * brightness) / 256, (color.G * brightness) / 256, (color.B * brightness) / 256);
        }
    }
}
