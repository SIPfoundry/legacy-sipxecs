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
    public partial class CallRulesWindow : Form
    {
        public CallRulesWindow()
        {
            InitializeComponent();

            Microsoft.Win32.RegistryKey key;

            key = Microsoft.Win32.Registry.CurrentUser.CreateSubKey(Constants.REG_KEY_NAME_ADDIN_SETTING);
            this.prefixToRemove.Text = (String)key.GetValue(Constants.REG_ATTR_NAME_PREFIX_TO_REMOVE);
            this.prefixToAdd.Text = (String)key.GetValue(Constants.REG_ATTR_NAME_PREFIX_TO_ADD);

            key.Close();

        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>

        private void okButton_Click(object sender, EventArgs e)
        {
            if (Utility.checkNonNumbers(prefixToRemove.Text) || Utility.checkNonNumbers(prefixToAdd.Text))
            {
                MessageBox.Show("Invalid letters found! only numbers [0 - 9] allowed for both prefixes");
                this.DialogResult = DialogResult.None;
                return;
            }

            try
            {
                Microsoft.Win32.RegistryKey key;

                key = Microsoft.Win32.Registry.CurrentUser.CreateSubKey(Constants.REG_KEY_NAME_ADDIN_SETTING);

                key.SetValue(Constants.REG_ATTR_NAME_PREFIX_TO_REMOVE, this.prefixToRemove.Text, Microsoft.Win32.RegistryValueKind.String);
                key.SetValue(Constants.REG_ATTR_NAME_PREFIX_TO_ADD, this.prefixToAdd.Text, Microsoft.Win32.RegistryValueKind.String);
                key.Close();
            }

            catch (System.Exception ex)
            {
                MessageBox.Show(ex.Message);
            }

            finally
            {
                this.Close();
            }
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void cancelButton_Click(object sender, EventArgs e)
        {
            this.Close();
        }

        private void prefixToRemove_TextChanged(object sender, EventArgs e)
        {
            if (Utility.checkNonNumbers(prefixToRemove.Text))
            {
                MessageBox.Show("Invalid letters found! only numbers [0 - 9] allowed !");
            }

        }

        private void prefixToAdd_TextChanged(object sender, EventArgs e)
        {
            if (Utility.checkNonNumbers(prefixToAdd.Text))
            {
                MessageBox.Show("Invalid letters found! only numbers [0 - 9] allowed !");
            }

        }
    }
}