using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Shapes;

namespace RenderToyEditor
{
    /// <summary>
    /// Interaction logic for RendererWindow.xaml
    /// </summary>
    public partial class RendererWindow : Window
    {
        public RendererWindow()
        {
            InitializeComponent();
            this.Loaded += OnWindowLoaded;
        }

        private void OnWindowLoaded(object sender, RoutedEventArgs e)
        {
            var windowHelper = new System.Windows.Interop.WindowInteropHelper(this);
            var hWnd = windowHelper.Handle;

            
            bool succ = RendererInterop.InitializeRenderer(hWnd);
        }
    }
}
