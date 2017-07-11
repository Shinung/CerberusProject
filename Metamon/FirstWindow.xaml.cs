using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Shapes;

namespace Metamon
{
    #region ForPointOfHandDetection

    /// <summary>
    /// Interaction logic for FirstWindow.xaml
    /// </summary>
    public partial class FirstWindow : Window
    {
        BackgroundWorker bgWorker;
        AutoResetEvent bgWorkerEvent;
        bool flagStopBGW;

        enum ShowWindow { Non, Second, Third, Fourth, Exit };

        ShowWindow sw;

        Rect gridSecRect, gridThirdRect, gridFourthRect, gridExitRect;

        public FirstWindow()
        {
            InitializeComponent();

            App.firWindow = this;
            HandMotion.SCcb = this.CallBackShowCam;

            flagStopBGW = false;
            bgWorker = new BackgroundWorker();
            bgWorker.WorkerSupportsCancellation = true;
            bgWorker.DoWork += BgWorker_DoWork;
            bgWorker.RunWorkerCompleted += BgWorker_RunWorkerCompleted;
        }

        private void BgWorker_RunWorkerCompleted(object sender, RunWorkerCompletedEventArgs e)
        {
            int i = 0;
            switch (sw)
            {
                case ShowWindow.Second:
                    this.Visibility = Visibility.Hidden;

                    App.secWindow = new SecondWindow();
                    App.secWindow.ShowDialog();

                    this.Visibility = Visibility.Visible;
                    break;
                case ShowWindow.Third:
                    this.Visibility = Visibility.Hidden;

                    App.thrWindow = new ThirdWindow();
                    App.thrWindow.ShowDialog();

                    this.Visibility = Visibility.Visible;
                    break;
                case ShowWindow.Fourth:
                    this.Visibility = Visibility.Hidden;

                    App.forWindow = new FourthWindow();
                    App.forWindow.ShowDialog();

                    this.Visibility = Visibility.Visible;
                    break;
                case ShowWindow.Exit:
                    flagStopBGW = true;
                    bgWorkerEvent.WaitOne();
                    Application.Current.Shutdown();
                    break;
            }
        }

        private ShowWindow CheckLocation(double x, double y)
        {
            Point pos = new Point(x, y);
            if(gridSecRect.Contains(pos))
                return ShowWindow.Second;
            if (gridThirdRect.Contains(pos))
                return ShowWindow.Third;
            if (gridFourthRect.Contains(pos))
                return ShowWindow.Fourth;
            if (gridExitRect.Contains(pos))
                return ShowWindow.Exit;

            return ShowWindow.Non;
        }

        private void BgWorker_DoWork(object sender, DoWorkEventArgs e)
        {
            while (true)
            {
                if (flagStopBGW)
                {
                    //MessageBox.Show("bgWorker stop");
                    break;
                }

                double x = - 1, y =  -1;
                int res;
                res = HandMotion.ShowWebCam(ref x, ref y);
                if (res < 0)
                {
                    MessageBox.Show("ShowInitCam Error : " + res.ToString());
                    break;
                }

                if(x > 0 && y > 0)
                {
                    if (this.IsVisible)
                    {
                        Dispatcher.BeginInvoke(new Action(
                            () =>
                            {
                                Canvas.SetLeft(ellipsePoint, x);
                                Canvas.SetTop(ellipsePoint, y);
                            }
                            ), System.Windows.Threading.DispatcherPriority.Background).Wait(TimeSpan.FromSeconds(0.2));
                    }

                    sw = CheckLocation(x, y);
                    if(sw != ShowWindow.Non)
                        break;
                }

                Thread.Sleep(HandMotion.delay);
            }
            bgWorkerEvent.Set();
        }

        private void CallBackShowCam(string str_path)
        {
            if (!this.IsVisible)
                return;

            using (Stream sr = File.Open(str_path, FileMode.Open))
            {
                BitmapImage bitImg = new BitmapImage();
                bitImg.BeginInit();
                bitImg.StreamSource = sr;
                bitImg.EndInit();
                WriteableBitmap wb = new WriteableBitmap(bitImg);

                wb.Freeze();
                Dispatcher.BeginInvoke(new Action(() => imgCam.Source = wb),
                    System.Windows.Threading.DispatcherPriority.Background).Wait(TimeSpan.FromSeconds(0.2));
            }
        }

        private void Window_IsVisibleChanged(object sender, DependencyPropertyChangedEventArgs e)
        {
            if (!this.IsVisible)
            {
                if (bgWorker.IsBusy)
                {
                    flagStopBGW = true;
                    bgWorkerEvent.WaitOne();
                }
                //MessageBox.Show("out of while");

                HandMotion.ReleasePointHandDetection();
            }
            else
            {
                int res;
                res = HandMotion.InitPointHandDetection(ref HandMotion.delay, ref HandMotion.camWidth, ref HandMotion.camHeight, 
                    HandMotion.SCcb);
                if(res < 0)
                {
                    MessageBox.Show("WebCam Init Error!");
                    Application.Current.Shutdown();
                }

                //MessageBox.Show("WebCam Id:" + res);

                this.Width = imgCam.Width = HandMotion.camWidth;
                this.Height = imgCam.Height = HandMotion.camHeight;

                this.Width += 25;
                this.Height += 50;

                sw = ShowWindow.Non;
                flagStopBGW = false;
                bgWorkerEvent = new AutoResetEvent(false);
                bgWorker.RunWorkerAsync();
            }
        }

        private void gridSecondWin_MouseLeftButtonUp(object sender, MouseButtonEventArgs e)
        {
            this.Visibility = Visibility.Hidden;

            App.secWindow = new SecondWindow();
            App.secWindow.ShowDialog();

            this.Visibility = Visibility.Visible;
        }

        private void gridExit_MouseLeftButtonUp(object sender, MouseButtonEventArgs e)
        {
            flagStopBGW = true;
            bgWorkerEvent.WaitOne();
            Application.Current.Shutdown();
        }

        private void gridThirdWin_MouseLeftButtonUp(object sender, MouseButtonEventArgs e)
        {
            this.Visibility = Visibility.Hidden;

            App.thrWindow = new ThirdWindow();
            App.thrWindow.ShowDialog();

            this.Visibility = Visibility.Visible;
        }

        private void gridFourthWin_MouseLeftButtonUp(object sender, MouseButtonEventArgs e)
        {
            this.Visibility = Visibility.Hidden;

            App.forWindow = new FourthWindow();
            App.forWindow.ShowDialog();

            this.Visibility = Visibility.Visible;
        }

        private void Window_Loaded(object sender, RoutedEventArgs e)
        {
            //MessageBox.Show("loaded");

            Point topLeft;

            topLeft = gridSecondWin.TransformToAncestor(fwCanvas).Transform(new Point(0, 0));
            gridSecRect = new Rect(topLeft.X, topLeft.Y, gridSecondWin.ActualWidth, gridSecondWin.ActualHeight);

            topLeft = gridThirdWin.TransformToAncestor(fwCanvas).Transform(new Point(0, 0));
            gridThirdRect = new Rect(topLeft.X, topLeft.Y, gridThirdWin.ActualWidth, gridThirdWin.ActualHeight);

            topLeft = gridFourthWin.TransformToAncestor(fwCanvas).Transform(new Point(0, 0));
            gridFourthRect = new Rect(topLeft.X, topLeft.Y, gridFourthWin.ActualWidth, gridFourthWin.ActualHeight);

            topLeft = gridExit.TransformToAncestor(fwCanvas).Transform(new Point(0, 0));
            gridExitRect = new Rect(topLeft.X, topLeft.Y, gridExit.ActualWidth, gridExit.ActualHeight);
        }

        private void Window_Closed(object sender, EventArgs e)
        {
            Application.Current.Shutdown();
        }
    }

    #endregion

    #region ForHandMotion
    /// <summary>
    /// Interaction logic for FirstWindow.xaml
    /// </summary>
    //public partial class FirstWindow : Window
    //{
    //    BackgroundWorker bgWorker = new BackgroundWorker();
    //    AutoResetEvent bgWorkerEvent;
    //    bool flagStopBGW;

    //    BackgroundWorker bgwHandMotion = new BackgroundWorker();
    //    AutoResetEvent bgwHandEvent;
    //    bool flagStopbgwHand;

    //    public FirstWindow()
    //    {
    //        InitializeComponent();

    //        App.firWindow = this;

    //        flagStopBGW = false;
    //        bgWorker.DoWork += BgWorker_DoWork;

    //        flagStopbgwHand = false;
    //        bgwHandMotion.DoWork += BgwHandMotion_DoWork;
    //    }

    //    private void BgwHandMotion_DoWork(object sender, DoWorkEventArgs e)
    //    {
    //        while (true)
    //        {
    //            if (flagStopbgwHand)
    //            {
    //                break;
    //            }

    //            if (HandMotion.RunHandDetection() < 0)
    //            {
    //                MessageBox.Show("RunHandDetection Error");
    //                break;
    //            }

    //            Thread.Sleep(HandMotion.delay);
    //        }

    //        bgwHandEvent.Set();
    //    }

    //    private void BgWorker_DoWork(object sender, DoWorkEventArgs e)
    //    {
    //        while (true)
    //        {
    //            if (flagStopBGW)
    //            {
    //                //MessageBox.Show("bgWorker stop");
    //                break;
    //            }

    //            int res = HandMotion.ShowInitCam();
    //            if (res < 0)
    //            {
    //                MessageBox.Show("ShowInitCam Error : " + res.ToString());
    //                break;
    //            }

    //            Thread.Sleep(HandMotion.delay);
    //        }

    //        bgWorkerEvent.Set();
    //    }

    //    private void ShowCam(string str_path)
    //    {
    //        if (!this.IsVisible)
    //            return;

    //        using (Stream sr = File.Open(str_path, FileMode.Open))
    //        {
    //            BitmapImage bitImg = new BitmapImage();
    //            bitImg.BeginInit();
    //            bitImg.StreamSource = sr;
    //            bitImg.EndInit();
    //            WriteableBitmap wb = new WriteableBitmap(bitImg);

    //            wb.Freeze();
    //            imgCam.Dispatcher.BeginInvoke(new Action(() => imgCam.Source = wb));
    //        }
    //    }

    //    private void Window_IsVisibleChanged(object sender, DependencyPropertyChangedEventArgs e)
    //    {
    //        if (!this.IsVisible)
    //        {
    //            if (bgWorker.IsBusy)
    //            {
    //                flagStopBGW = true;
    //                bgWorkerEvent.WaitOne();

    //            }
    //            if (bgwHandMotion.IsBusy)
    //            {
    //                flagStopbgwHand = true;
    //                bgwHandEvent.WaitOne();
    //            }
    //            //MessageBox.Show("out of while");

    //            HandMotion.ReleaseHandDetection();
    //        }
    //        else
    //        {
    //            HandMotion.SCcb = ShowCam;
    //            HandMotion.InitHandDetection(ref HandMotion.delay, ref HandMotion.camWidth, ref HandMotion.camHeight, HandMotion.SCcb);

    //            this.Width = imgCam.Width = HandMotion.camWidth;
    //            this.Height = imgCam.Height = HandMotion.camHeight;

    //            this.Width += 25;
    //            this.Height += 50;

    //            gridSecondWin.Visibility = Visibility.Hidden;
    //            gridThirdWin.Visibility = Visibility.Hidden;
    //            gridFourthWin.Visibility = Visibility.Hidden;

    //            flagStopBGW = false;
    //            bgWorkerEvent = new AutoResetEvent(false);
    //            bgWorker.RunWorkerAsync();
    //        }
    //    }

    //    private void imgCam_MouseLeftButtonUp(object sender, MouseButtonEventArgs e)
    //    {
    //        if (bgWorker.IsBusy)
    //        {
    //            flagStopBGW = true;
    //            bgWorkerEvent.WaitOne();

    //            if (HandMotion.TrackingINIT() < 0)
    //            {
    //                MessageBox.Show("TrackingINIT Error");
    //                return;
    //            }

    //            flagStopbgwHand = false;
    //            bgwHandEvent = new AutoResetEvent(false);
    //            bgwHandMotion.RunWorkerAsync();

    //            gridSecondWin.Visibility = Visibility.Visible;
    //            gridThirdWin.Visibility = Visibility.Visible;
    //            gridFourthWin.Visibility = Visibility.Visible;
    //        }
    //        else
    //        {
    //            if (bgwHandMotion.IsBusy)
    //            {
    //                flagStopbgwHand = true;
    //                bgwHandEvent.WaitOne();
    //            }

    //            gridSecondWin.Visibility = Visibility.Hidden;
    //            gridThirdWin.Visibility = Visibility.Hidden;
    //            gridFourthWin.Visibility = Visibility.Hidden;

    //            flagStopBGW = false;
    //            bgWorkerEvent = new AutoResetEvent(false);
    //            bgWorker.RunWorkerAsync();
    //        }
    //    }

    //    private void gridSecondWin_MouseLeftButtonUp(object sender, MouseButtonEventArgs e)
    //    {
    //        this.Visibility = Visibility.Hidden;

    //        App.secWindow = new SecondWindow();
    //        App.secWindow.ShowDialog();

    //        this.Visibility = Visibility.Visible;
    //    }

    //    private void Window_Closed(object sender, EventArgs e)
    //    {
    //        Application.Current.Shutdown();
    //    }
    //}
    #endregion
}
