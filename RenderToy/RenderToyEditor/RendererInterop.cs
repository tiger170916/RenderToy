using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.InteropServices;

namespace RenderToyEditor
{
    public class RendererInterop
    {
        [DllImport("RenderToy.dll")]
        public static extern bool InitializeRenderer(IntPtr hwnd);

        [DllImport("RenderToy.dll")]
        public static extern bool StartRendering();

        [DllImport("RenderToy.dll")]
        public static extern bool FinalizeRenderer();
    }
}
