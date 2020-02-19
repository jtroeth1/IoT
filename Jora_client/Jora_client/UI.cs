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
using Android.Support.V7.App;

namespace Jora_client
{
    public static class UI
    {
        public static TextView connectionStatus;
        public static Button connectButton;
        public static Button ledButton;
        public static TextView ledStatus;
        public static EditText gatewayIp;
        public static EditText nodeAddress;
    }
}