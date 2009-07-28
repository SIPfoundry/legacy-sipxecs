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
using System.ComponentModel;
using System.Configuration.Install;
using System.IO;
using AddinForOutlook;


namespace AddinForOutlookSetupExt
{
    [RunInstaller(true)]
    public partial class CustomInstaller : Installer
    {
        public CustomInstaller()
        {
            InitializeComponent();
        }

        private void CustomInstaller_AfterInstall(object sender, InstallEventArgs e)
        {
            string serverName = this.Context.Parameters["SERVERNAME"];
            string userName = this.Context.Parameters["USERNAME"];
            string conferenceNumber = this.Context.Parameters["CONFERENCENUMBER"];
            string pinNumber = this.Context.Parameters["PINNUMBER"];
            string isDefaultLoc = this.Context.Parameters["ISDEFAULTLOCATION"];

            Microsoft.Win32.RegistryKey key;

            key = Microsoft.Win32.Registry.CurrentUser.CreateSubKey(Constants.REG_KEY_NAME_ADDIN_SETTING);

            key.SetValue(Constants.REG_ATTR_NAME_CTI_SERVER_NAME, serverName, Microsoft.Win32.RegistryValueKind.String);
            key.SetValue(Constants.REG_ATTR_NAME_USERNAME, userName, Microsoft.Win32.RegistryValueKind.String);
            key.SetValue(Constants.REG_ATTR_NAME_CONFERENCE_NUMBER, conferenceNumber, Microsoft.Win32.RegistryValueKind.String);
            key.SetValue(Constants.REG_ATTR_NAME_CONFERENCE_ACCESS_CODE, pinNumber, Microsoft.Win32.RegistryValueKind.String);

            if (isDefaultLoc == "1")
            {
                key.SetValue(Constants.REG_ATTR_NAME_ISDEFAULTLOCATION, 1, Microsoft.Win32.RegistryValueKind.DWord);
            }
            else
            {
                key.SetValue(Constants.REG_ATTR_NAME_ISDEFAULTLOCATION, 0, Microsoft.Win32.RegistryValueKind.DWord);
            }

            key.Close();
        }

        /// <summary>
        /// 
        /// </summary>
        /// 
#if DEBUG
        void WriteToFile(String myPassedValue, String fName, String lName, String path)
        {
            StreamWriter writer = new StreamWriter("C:\\Install.log", true);
            writer.WriteLine("Install log file");
            writer.WriteLine(DateTime.Now.ToShortDateString() + " " + DateTime.Now.ToShortTimeString());
            writer.WriteLine("Install path is " + path);
            string s = "";
            switch (myPassedValue)
            {
                case "1":
                    s = "develop";
                    break;
                case "2":
                    s = "test";
                    break;
                case "3":
                    s = "product";
                    break;
            }
            writer.WriteLine("You selected " + s);
            writer.WriteLine("First name: " + fName);
            writer.WriteLine("Last name: " + lName);
            writer.Flush();
            writer.Close();
        }
#endif

    }
}