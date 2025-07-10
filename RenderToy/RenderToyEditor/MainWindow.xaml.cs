using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Interop;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using AvalonDock.Controls;
using AvalonDock.Layout;

namespace RenderToyEditor
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        private RendererWindow _rendererWindow;

        public MainWindow()
        {
            InitializeComponent();

            _rendererWindow = new RendererWindow();
            _rendererWindow.Show();
        }
    }
}