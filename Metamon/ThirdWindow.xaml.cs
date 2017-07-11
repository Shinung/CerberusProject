using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
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
using System.Diagnostics;
using System.Threading;
using System.Windows.Threading;
using System.ComponentModel;
using System.IO;

namespace Metamon
{
    /// <summary>
    /// MainWindow.xaml에 대한 상호 작용 논리
    /// </summary>    
    public partial class ThirdWindow : Window
    {
        [DllImport("CamFuncsDll")]
        extern public static int VideoCam(int type);
        [DllImport("CamFuncsDll")]
        extern public static int RecordF(int type);
        [DllImport("CamFuncsDll")]
        extern public static int Pics(int type);
        bool bRec = false;
        bool bStart = false;
        bool bStop = false;

        Thread ImageUpdateThread;
        delegate void DelegateImageUpdate();
        private bool _active = true;

        void ImageUpdate()
        {
            if (ImageUpdateThread == null)
            {
                ImageUpdateThread = new Thread(UpdateImages);
                ImageUpdateThread.Start();

            }
        }

        void UpdateImages()
        {
            while (_active == true)
            {
                Dispatcher.Invoke(DispatcherPriority.Normal, new DelegateImageUpdate(target));
                Thread.Sleep(200);
            }
        }

        private void target()
        {
            // temp는 상대주소를 위한 것
            string temp = AppDomain.CurrentDomain.BaseDirectory+@"Resource\snap.jpg";

            BitmapImage img = new BitmapImage();
            img.BeginInit();
            img.CacheOption = BitmapCacheOption.OnLoad;
            img.CreateOptions = BitmapCreateOptions.IgnoreImageCache;
            img.UriSource = new Uri(temp);
            img.EndInit();
            Debug.WriteLine("갱신 " + img);
            image1.Source = img;
        }

        public ThirdWindow()
        {

            InitializeComponent();
            this.Loaded += new RoutedEventHandler(MainWindow_Loaded);

        }

        void MainWindow_Loaded(object sender, RoutedEventArgs e)
        {
            ImageUpdate();
        }


        private void comboBox1_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            if (bStop == true)
                return;
            ComboBoxItem choice = ((sender as ComboBox).SelectedItem as ComboBoxItem);

            ComboBoxItem choice1 = ((sender as ComboBox).SelectedItem as ComboBoxItem);
            string str = choice1.Name as string;
            if (str == "None")
            {
                this.maskComboBox.SelectedIndex = 0;
                VideoCam(0);
            }
            else if (str == "iron1")
            {
                this.maskComboBox.SelectedIndex = 1;
                VideoCam(1);
            }
            else if (str == "caw1")
            {
                this.maskComboBox.SelectedIndex = 2;
                VideoCam(2);
            }
            else
            {
                this.maskComboBox.SelectedIndex = 0;
                VideoCam(0);
            }
            bStart = true;
        }

        private void btnRec_Click(object sender, RoutedEventArgs e)
        {
            bStart = true;
            if (bStop == true)
                return;
            if (bRec == false)
            {
                bRec = true;
                MessageBox.Show("녹화시작");
                RecordF(1);
            }
            else if (bRec == true)
            {
                RecordF(0);
                bRec = false;
                MessageBox.Show("녹화저장");
            }
        }

        private void bntPic_Click(object sender, RoutedEventArgs e)
        {
            bStart = true;
            if (bStop == true)
                return;
            Pics(1);
            MessageBox.Show("사진저장");
        }

        private void btnChange_Click_1(object sender, RoutedEventArgs e)
        {
            if (bStop == true)
                return;
            bStart = true;
            VideoCam(3);
        }

        private void btnExit_Click(object sender, RoutedEventArgs e)
        {
            MessageBoxResult msg = MessageBox.Show("종료하시겠습니까?", "Confirmation", MessageBoxButton.YesNo);
            if (msg == MessageBoxResult.Yes)
            {
                VideoCam(-1);
                bStop = true;
                Application.Current.Shutdown();
            }
        }

        private void image1_SourceUpdated(object sender, DataTransferEventArgs e)
        {

        }
        private void Window_Closing(object sender, CancelEventArgs e)
        {
            MessageBoxResult msg = MessageBox.Show("종료하시겠습니까?", "Confirmation", MessageBoxButton.YesNo);
            if (msg == MessageBoxResult.No)
            {
                e.Cancel = true;
            }
            else
            {
                ImageUpdateThread.Abort();
                VideoCam(-1);
                bStop = true;
            }
        }
    }
}
