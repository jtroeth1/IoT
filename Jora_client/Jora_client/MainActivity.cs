using Android.App;
using Android.Widget;
using Android.OS;
using Android.Support.V7.App;
using Android.Runtime;

namespace Jora_client
{
    [Activity(Label = "@string/app_name", Theme = "@style/AppTheme", MainLauncher = true)]
    public class MainActivity : AppCompatActivity
    {
        protected override void OnCreate(Bundle savedInstanceState)
        {
            base.OnCreate(savedInstanceState);

            // Set our view from the "main" layout resource
            SetContentView(Resource.Layout.activity_main);

            // Get our UI controls from the loaded layout       
            UI.connectionStatus = FindViewById<TextView>(Resource.Id.connectionStatus);
            UI.connectButton = FindViewById<Button>(Resource.Id.connectButton);
            UI.ledButton = FindViewById<Button>(Resource.Id.ledButton);
            UI.ledStatus = FindViewById<TextView>(Resource.Id.ledStatus);
            UI.gatewayIp = FindViewById<EditText>(Resource.Id.gatewayIp);
            UI.nodeAddress = FindViewById<EditText>(Resource.Id.nodeAddress);
            
            // Connect to socket on click
            UI.connectButton.Click += (sender, e) =>
            {
                if(SocketClass.socket != null && SocketClass.socket.Connected)
                {
                    SocketClass.Close();
                }
                else
                {
                    SocketClass.Open();
                }
                
            };

            //send LED ON/OFF CMD
            UI.ledButton.Click += (sender, e) =>
            {
                string msg = "$" + UI.nodeAddress.Text;
                msg += "@" + LED.LedChangeState();
                SocketClass.Send(msg);

                if (LED.isOn)
                    UI.ledButton.Text = "LED ON";
                else
                    UI.ledButton.Text = "LED OFF";

                UI.ledStatus.Text = SocketClass.Recv();
            };



        }

    }
}

