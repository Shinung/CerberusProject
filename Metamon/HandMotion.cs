using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.InteropServices;
using System.Windows.Media.Imaging;

namespace Metamon
{
    class HandMotion
    {
        public delegate void ShowCamCallBack(string str_path);

        #region PointOfHandDetectionDll

        [DllImport("PointOfHandDetectionDll.dll")]
        public static extern int InitPointHandDetection(ref int out_delay, ref int out_width, ref int out_height,
            ShowCamCallBack scCallBack);

        [DllImport("PointOfHandDetectionDll.dll")]
        public static extern void ReleasePointHandDetection();

        [DllImport("PointOfHandDetectionDll.dll")]
        public static extern int ShowWebCam(ref double out_x, ref double out_y);

        #endregion

        #region HandMotionDll
        //[DllImport("HandMotionDll.dll")]
        //public static extern int InitHandDetection(ref int out_delay, ref int out_width, ref int out_height,
        //    ShowCamCallBack scCallBack);

        //[DllImport("HandMotionDll.dll")]
        //public static extern void ReleaseHandDetection();

        //[DllImport("HandMotionDll.dll")]
        //public static extern int ShowInitCam();

        //[DllImport("HandMotionDll.dll")]
        //public static extern int TrackingINIT();

        //[DllImport("HandMotionDll.dll")]
        //public static extern int RunHandDetection();
        #endregion

        public static int delay;
        public static int camWidth, camHeight;
        public static ShowCamCallBack SCcb;
    }
}
