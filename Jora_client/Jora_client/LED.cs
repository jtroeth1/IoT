using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

using Android.App;
using Android.Content;
using Android.OS;
using Android.Runtime;
using Android.Views;
using Android.Widget;

namespace Jora_client
{

    public static class LED
    {
        public static bool isOn = false;

        public static string LedChangeState()
        {
            if (isOn)
            {
                isOn = false;
                return "L0";
            }
            else
            {
                isOn = true;
                return "L1";
            }       
        }


    }
}