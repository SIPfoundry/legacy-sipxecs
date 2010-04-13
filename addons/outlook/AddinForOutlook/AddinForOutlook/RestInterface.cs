//
//
// Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////
using System;
using System.Collections.Generic;
using System.Text;
using System.Net;
using System.IO;
using System.Windows.Forms;


namespace AddinForOutlook
{
    sealed public class RestInterface
    {
        public static void restCall(String calledNumber, String password)
        {
            try
            {

                Microsoft.Win32.RegistryKey key = Microsoft.Win32.Registry.CurrentUser.CreateSubKey(Constants.REG_KEY_NAME_ADDIN_SETTING);

                
                String serverIP = Utility.getCtiServerIPAddress((String)key.GetValue(Constants.REG_ATTR_NAME_CTI_SERVER_NAME));
                if (serverIP == null)
                {
                    Logger.WriteEntry(LogLevel.Error, "Failed to resolve server IP address!");
                    MessageBox.Show("Failed to resolve server IP address! Make sure DNS service is configured correctly!");
                    key.Close(); 
                    
                    return;
                }


                String port = (String)key.GetValue(Constants.REG_ATTR_NAME_PORT, Constants.DEFAULT_CTI_SERVICE_PORT);
                string url = "https://" + serverIP + ":" + port + Constants.REST_VIRTUAL_PATH + calledNumber;
                string username = (String)key.GetValue(Constants.REG_ATTR_NAME_USERNAME);

                if (username == null || username == String.Empty)
                {
                    Logger.WriteEntry(LogLevel.Error, "username == null || username == empty");
                    MessageBox.Show("Invalid user account!");

                    key.Close();
                    return;
                }

                key.Close();

                Logger.WriteEntry(LogLevel.Information, url);


                HttpWebRequest request = (HttpWebRequest)WebRequest.Create(url);
                request.Method = Constants.REST_METHOD;
                request.Credentials = new NetworkCredential(username, password);

                request.Proxy = null; // No proxy detection.

                HttpWebResponse response = (HttpWebResponse)request.GetResponse();
                if (response == null)
                {
                    Logger.WriteEntry(LogLevel.Error, "response is null");
                    MessageBox.Show("Failed to get a response from server! please verify configuration and server availability");
                    return;
                }

                if (response.StatusCode != HttpStatusCode.OK)
                {
                    if (response.StatusCode == HttpStatusCode.Unauthorized)
                    {
                        Logger.WriteEntry(LogLevel.Warning, "unauthorized (401)");
                        MessageBox.Show("Authentication failed! please make sure username/password are correct!");
                        return;
                    }
                    else
                    {
                        using (StreamReader reader = new StreamReader(response.GetResponseStream()))
                        {
                            while (reader.Peek() != -1)
                            {
                                Logger.WriteEntry(LogLevel.Error, reader.ReadLine());
                            }
                        }
                        MessageBox.Show("Failed to call! please check the logs for details");
                        return;
                    }
                }

            }

            catch (System.Net.WebException wex)
            {               
                HttpWebResponse resp = (HttpWebResponse)wex.Response;
                if (resp != null && resp.StatusCode == HttpStatusCode.Unauthorized)
                {
                    Logger.WriteEntry(LogLevel.Warning, "unauthorized(401)");
                    MessageBox.Show("Authentication failed! please make sure the username/password are correct!");
                    return;
                }

                Logger.WriteEntry(LogLevel.Error, "WebException: " + wex.GetType() + " " + wex.Message + " " + wex.StackTrace);
                MessageBox.Show(wex.GetType() + " : " + wex.Message );
                return;
            }
            catch (System.Exception ex)
            {
                Logger.WriteEntry(LogLevel.Error, "Exception: " + ex.GetType() + " " + ex.Message + " " + ex.StackTrace);
                MessageBox.Show("Exception: " + ex.Message);
                return;
            }
        }
    }
}
