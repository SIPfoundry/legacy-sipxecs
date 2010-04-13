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
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace AddinForOutlook
{
    public partial class PassCodeWindow : Form
    {
        public static String SCSPassword = intializePIN();
        public PassCodeWindow()
        {
            InitializeComponent();
        }

        private void SCSPasswd_TextChanged(object sender, EventArgs e)
        {
        }

        private void buttonOK_Click(object sender, EventArgs e)
        {
            if (this.SCSPasswd.Text == String.Empty)
            {
                MessageBox.Show("Empty password is not acceptable!");
                this.DialogResult = DialogResult.None;
                return;
            }
            SCSPassword = this.SCSPasswd.Text; //always

            try
            {
                Microsoft.Win32.RegistryKey key = Microsoft.Win32.Registry.CurrentUser.CreateSubKey(Constants.REG_KEY_NAME_ADDIN_SETTING);
                key.SetValue(Constants.REG_ATTR_NAME_ISPINREMEMBERED, (this.isPasswordRemembered.Checked ? 1 : 0), Microsoft.Win32.RegistryValueKind.DWord);

                if (!this.isPasswordRemembered.Checked)
                {
                    key.DeleteValue(Constants.REG_ATTR_NAME_USER_PIN, false);
                }
                else
                {
                    key.SetValue(Constants.REG_ATTR_NAME_USER_PIN, Crypto.Encrypt(this.SCSPasswd.Text), Microsoft.Win32.RegistryValueKind.String);
                }

                key.Close();
            }

            catch (System.Exception ex)
            {
                Logger.WriteEntry(LogLevel.Warning, ex.GetType() + ":" + ex.Message);
            }

            Close();
        }


        /// <summary>
        /// 
        /// </summary>
        /// <returns></returns>
        static String intializePIN()
        {
            try
            {
                Microsoft.Win32.RegistryKey key = Microsoft.Win32.Registry.CurrentUser.CreateSubKey(Constants.REG_KEY_NAME_ADDIN_SETTING);
                int val = (int)key.GetValue(Constants.REG_ATTR_NAME_ISPINREMEMBERED, Constants.DEFAULT_ISPINREMEMBERED);

                string str = string.Empty;                    
                if (val != 0)
                {
                     str = (string)key.GetValue(Constants.REG_ATTR_NAME_USER_PIN, String.Empty);
                     if (str != String.Empty)
                     {
                         str = Crypto.Decrypt(str);
                     }
                }

                key.Close();
                return str;
            }

            catch (System.Exception ex)
            {
                Logger.WriteEntry(LogLevel.Error, ex.GetType() + ":" + ex.Message);
                return String.Empty;
            }

        }

        private void PassCodeWindow_Load(object sender, EventArgs e)
        {
            try
            {
                Microsoft.Win32.RegistryKey key = Microsoft.Win32.Registry.CurrentUser.CreateSubKey(Constants.REG_KEY_NAME_ADDIN_SETTING);
                int val = (int)key.GetValue(Constants.REG_ATTR_NAME_ISPINREMEMBERED, Constants.DEFAULT_ISPINREMEMBERED);
                this.isPasswordRemembered.Checked = (val == 0 ? false : true);
                key.Close();

                this.SCSPasswd.Text = SCSPassword ; // From the cache.
                
            }

            catch (System.Exception ex)
            {
                Logger.WriteEntry(LogLevel.Error, ex.GetType() + ":" + ex.Message);
            }

        }

        private void Cancel_Click(object sender, EventArgs e)
        {
            this.Close();
        }
    }
}