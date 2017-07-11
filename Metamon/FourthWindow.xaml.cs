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
using Microsoft.Win32; // DLL support


namespace Metamon
{
    /// <summary>
    /// MainWindow.xaml에 대한 상호 작용 논리
    /// </summary>
    public partial class FourthWindow : Window
    {
        //[DllImport("C:/Lecture/OpenCv/FindLetter/{out}/Win32/Release/FindLetter.dll")]
        //public static extern void Loading();
        //[DllImport("C:/Lecture/OpenCv/FindLetter/{out}/Win32/Release/FindLetter.dll")]
        //public static extern void LabelImage(string filename);
        //[DllImport("C:/Lecture/OpenCv/FindLetter/{out}/Win32/Release/FindLetter.dll")]
        //public static extern void OptimizeLabels();
        //[DllImport("C:/Lecture/OpenCv/FindLetter/{out}/Win32/Release/FindLetter.dll")]
        //public static extern IntPtr FindLetters();

        [DllImport("FindLetter.dll")]
        public static extern void Loading();
        [DllImport("FindLetter.dll")]
        public static extern void LabelImage(string filename);
        [DllImport("FindLetter.dll")]
        public static extern void OptimizeLabels();
        [DllImport("FindLetter.dll")]
        public static extern IntPtr FindLetters();
        
        string fileName;
        public FourthWindow()
        {
            InitializeComponent();
            Loading();
        }
        
        //이미지 고르기
        private void Button_Click(object sender, RoutedEventArgs e)
        {
            btn_label.IsEnabled = false;
            btn_optimize.IsEnabled = false;
            btn_find.IsEnabled = false;

            OpenFileDialog file = new OpenFileDialog();
            file.ShowDialog();
            fileName = file.FileName;
            try
            {
                BitmapImage bmImage = new BitmapImage();
                bmImage.BeginInit();
                bmImage.UriSource = new Uri(fileName);
                bmImage.EndInit();
                img_result.Source = bmImage;

            }
            catch
            {
                MessageBox.Show("이미지를 선택하십시오.");
                img_result.Source = null;
                return;
            }
            btn_label.IsEnabled = true;
        }

        //레이블링
        private void Button_Click_2(object sender, RoutedEventArgs e)
        {
            LabelImage(fileName);
            BitmapImage bmImage = new BitmapImage();
            bmImage.BeginInit();
            bmImage.CacheOption = BitmapCacheOption.OnLoad;
            bmImage.UriSource = new Uri("labeled.jpg", UriKind.Relative);
            bmImage.EndInit();
            img_result.Source = bmImage;
            btn_optimize.IsEnabled = true;
            
        }

        //레이블링 최적화
        private void Button_Click_3(object sender, RoutedEventArgs e)
        {
            btn_optimize.IsEnabled = false;
            OptimizeLabels();
            try
            {
                BitmapImage bmImage = new BitmapImage();
                bmImage.BeginInit();
                bmImage.CacheOption = BitmapCacheOption.OnLoad;
                bmImage.UriSource = new Uri("optimized.jpg", UriKind.Relative);
                bmImage.EndInit();
                img_result.Source = bmImage;
            }
            catch
            {
            }
            btn_find.IsEnabled = true;  
        }

        //문자 인식
        private void Button_Click_4(object sender, RoutedEventArgs e)
        {
            btn_find.IsEnabled = false;
            string result = System.Runtime.InteropServices.Marshal.PtrToStringAnsi(FindLetters());
            if (result.Contains("?"))
                MessageBox.Show("i 와 l 은 ? 으로 표현될 확률이 높습니다");
            txtBox.Text = result;
        }
    }
}
