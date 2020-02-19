using System;
using System.Net;
using System.Net.Sockets;
using System.Text;
using Android.Widget;
using static Jora_client.UI;

public static class SocketClass
{
    public static Socket socket;
    private static IPAddress ipAddress;
    public static void Open()
    {
        
        // Connect to a remote device.  
        try
        {
            // Establish the remote endpoint for the socket.  
            // This example uses port 11000 on the local computer.  
            //IPHostEntry ipHostInfo = Dns.GetHostEntry(Dns.GetHostName());
            
            IPAddress.TryParse(gatewayIp.Text, out ipAddress); //ipHostInfo.AddressList[0];
            IPEndPoint remoteEP = new IPEndPoint(ipAddress, 7878);

            //connectionStatus.Text += $"Connected to: {ipAddress}\n";
            //connectionStatus.Text += ipHostInfo.AddressList[0] + " --> " + ipAddress + "\n";
            //connectionStatus.Text += ipHostInfo.AddressList[0] + "\n";

            // Create a TCP/IP  socket.  
            socket = new Socket(ipAddress.AddressFamily,
                SocketType.Stream, ProtocolType.Tcp);

            // Connect the socket to the remote endpoint. Catch any errors.  
            try
            {
                IAsyncResult r = socket.BeginConnect(remoteEP, null, null);
                //s.connect(remoteEP);
                bool success = r.AsyncWaitHandle.WaitOne(5000, true);

                if (socket.Connected)
                {
                    socket.EndConnect(r);
                    connectButton.Text = "Connected";
                }
                else
                {
                    socket.Close();
                }

                connectionStatus.Text += $"Connected to {socket.RemoteEndPoint.ToString()}" + "\n";                   
            

            }
            catch (ArgumentNullException ane)
            {
                //connectionStatus.Text += $"ArgumentNullException : {ane.ToString()}" + "\n";
                connectionStatus.Text += $"Failed to connect to {ipAddress}\n";
            }
            catch (SocketException se)
            {
                //connectionStatus.Text += $"SocketException : {se.ToString()}" + "\n";
                connectionStatus.Text += $"Failed to connect to {ipAddress}\n";
            }
            catch (Exception e)
            {
                //connectionStatus.Text += $"Unexpected exception : {e.ToString()}" + "\n";
                connectionStatus.Text += $"Failed to connect to {ipAddress}\n";
            }

        }
        catch (Exception e)
        {
            connectionStatus.Text += e.ToString() + "\n";
            //connectionStatus.Text += $"Failed to connect to {ipAddress}";
        }
    }    

    public static void Send(string msg)
    {
        // Encode the data string into a byte array.  
        byte[] byteMsg = Encoding.ASCII.GetBytes(msg);

        try
        {
            socket.Send(byteMsg);
        }
        catch (Exception e)
        {
            connectionStatus.Text += $"Unexpected exception : {e.ToString()}" + "\n";
        }

    }

    public static string Recv()
    {
        // Data buffer for incoming data.  
        byte[] bytes = new byte[1024];

        // Receive the response from the remote device.  
        int bytesRec = socket.Receive(bytes);

        if (bytesRec > 0)
            return Encoding.ASCII.GetString(bytes);
        else
            return "-";
    }

    public static void Close()
    {
        // Release the socket.  
        connectionStatus.Text += $"Disconnected from {ipAddress}\n";
        socket.Shutdown(SocketShutdown.Both);
        socket.Close();
        connectButton.Text = "Connect";
    }
}