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
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.Runtime.InteropServices;
using System.Threading;

namespace Metamon
{
    /// <summary>
    /// MainWindow.xaml에 대한 상호 작용 논리
    /// </summary>
    public partial class SecondWindow : Window
    {
        public SecondWindow()
        {
            InitializeComponent();
            InitMR();
        }
        
        [DllImport("MoireRemove.dll")]
        public static extern int MoireStart(string fileName);
        [DllImport("MoireRemove.dll")]
        public static extern void Save(string fileName);
        [DllImport("MoireRemove.dll")]
        public static extern void InitMR();
        [DllImport("MoireRemove.dll")]
        public static extern void CarNumberStart(string fileName);

        public bool fileLoad = false;

        private void btn_Moire_Click(object sender, RoutedEventArgs e)
        {
            Microsoft.Win32.OpenFileDialog dlg = new Microsoft.Win32.OpenFileDialog();

            dlg.DefaultExt = ".jpg";
            dlg.Filter = "JPG Files (*.jpg)|*.jpg|JPEG Files (*.jpeg)|*.jpeg|PNG Files (*.png)|*.png|GIF Files (*.gif)|*.gif";

            Nullable<bool> result = dlg.ShowDialog();

            if(result == true)
            {
                tb_Moire.Text = dlg.FileName;
            }

            int res = MoireStart(tb_Moire.Text);
            if (res < 0)
                MessageBox.Show("MoireStart error");

            fileLoad = true;

            Thread.Sleep(10);
        }

        private void btn_Save_Click(object sender, RoutedEventArgs e)
        {
            if (tb_Save.Text != "" && fileLoad)
            {
                Save(tb_Save.Text + ".jpg");
                MessageBox.Show("이미지저장 성공");
            }
            else if(fileLoad)
                MessageBox.Show("파일명을 입력하세요");
            if(!fileLoad)
                MessageBox.Show("저장 할 파일이 없습니다. 파일을 불러오세요.");
        }

        private void btn_car_Click(object sender, RoutedEventArgs e)
        {
            Microsoft.Win32.OpenFileDialog dlg = new Microsoft.Win32.OpenFileDialog();

            dlg.DefaultExt = ".jpg";
            dlg.Filter = "JPG Files (*.jpg)|*.jpg|JPEG Files (*.jpeg)|*.jpeg|PNG Files (*.png)|*.png|GIF Files (*.gif)|*.gif";

            Nullable<bool> result = dlg.ShowDialog();

            if (result == true)
            {
                tb_car.Text = dlg.FileName;
            }

            CarNumberStart(tb_car.Text);

            fileLoad = true;
        }
    }
}
