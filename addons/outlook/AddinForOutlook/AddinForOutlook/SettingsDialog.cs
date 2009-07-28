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
using System.Collections;
using System.ComponentModel;
using System.Drawing;
using System.Windows.Forms;

namespace AddinForOutlook
{
    public class SettingsDialog : BaseEditDialog
    {
        // The ResetSettings() method in the Anchestor
        // is overide. Therefore you can code the Logic
        // here in the derived Form.
        protected override void ResetSettings()
        {
            try
            {
                Microsoft.Win32.RegistryKey key = Microsoft.Win32.Registry.CurrentUser.CreateSubKey(Constants.REG_KEY_NAME_ADDIN_SETTING);

                scsIPAddress.Text = (String)key.GetValue(Constants.REG_ATTR_NAME_CTI_SERVER_NAME, Constants.DEFAULT_CTI_SERVER_NAME);
                userName.Text = (String)key.GetValue(Constants.REG_ATTR_NAME_USERNAME, Constants.DEFAULT_USER_NAME);
                ConferenceNumber.Text = (String)key.GetValue(Constants.REG_ATTR_NAME_CONFERENCE_NUMBER, Constants.DEFAULT_CONFERENCE_NUMBER);
                ConferenceAccessCode.Text = (String)key.GetValue(Constants.REG_ATTR_NAME_CONFERENCE_ACCESS_CODE, Constants.DEFAULT_PASSCODE);
                
                int val = (int)key.GetValue(Constants.REG_ATTR_NAME_ISDEFAULTLOCATION, Constants.DEFAULT_IS_DEFAULT_LOCATION );
                isDefaultLocation.Checked = (val == 0 ? false : true);

                key.Close();
            }

            catch (System.Exception ex)
            {
                Logger.WriteEntry(LogLevel.Error, ex.GetType() + ":" + ex.Message);
            }
        }

        // The SaveSettings() method in the Anchestor
        // is overide. Therefore you can code the Logic
        // here in the derived Form.
        protected override bool SaveSettings()
        {
            try
            {
                Microsoft.Win32.RegistryKey key;

                key = Microsoft.Win32.Registry.CurrentUser.CreateSubKey(Constants.REG_KEY_NAME_ADDIN_SETTING);

                key.SetValue(Constants.REG_ATTR_NAME_CTI_SERVER_NAME, scsIPAddress.Text, Microsoft.Win32.RegistryValueKind.String);
                key.SetValue(Constants.REG_ATTR_NAME_USERNAME, userName.Text, Microsoft.Win32.RegistryValueKind.String);
                key.SetValue(Constants.REG_ATTR_NAME_CONFERENCE_NUMBER, ConferenceNumber.Text, Microsoft.Win32.RegistryValueKind.String);
                key.SetValue(Constants.REG_ATTR_NAME_CONFERENCE_ACCESS_CODE, ConferenceAccessCode.Text, Microsoft.Win32.RegistryValueKind.String);
                key.SetValue(Constants.REG_ATTR_NAME_ISDEFAULTLOCATION, isDefaultLocation.Checked ?  1 : 0, Microsoft.Win32.RegistryValueKind.DWord);
                key.Close();
            }

            catch (System.Exception ex)
            {
                Logger.WriteEntry(LogLevel.Error, ex.GetType() + ":" + ex.Message);
                return false;
            }
            
            return true;
        }

        private void InitializeComponent()
        {
            this.SuspendLayout();
            // 
            // button_OK
            // 
            this.button_OK.Location = new System.Drawing.Point(75, 293);
            // 
            // button_Cancel
            // 
            this.button_Cancel.Location = new System.Drawing.Point(223, 293);
            // 
            // isDefaultLocation
            // 
            this.isDefaultLocation.Location = new System.Drawing.Point(265, 256);
            // 
            // SettingsDialog
            // 
            this.ClientSize = new System.Drawing.Size(489, 344);
            this.Name = "SettingsDialog";
            this.Text = "Settings For SCS";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

    }
}



