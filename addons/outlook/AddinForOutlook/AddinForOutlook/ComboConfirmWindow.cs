//
//
// Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

using System;
using System.Collections;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace AddinForOutlook
{
    public partial class ComboConfirmWindow : Form
    {
        public string DestinationNumber = String.Empty;
        public ComboConfirmWindow(String label, Hashtable thelist)
        {
            InitializeComponent();
            this.promptText.Text = label;

            foreach (String keyname in thelist.Keys)
            {
                String str = addDesc((String)(thelist[keyname]), keyname);
                this.phoneList.Items.Add(str);
            }

            if (thelist.ContainsKey(Constants.KEY_BUSINESS_PHONE))
            {
                this.phoneList.Text = thelist[Constants.KEY_BUSINESS_PHONE] + Constants.PREFIX_DESC + Constants.KEY_BUSINESS_PHONE + Constants.SUBFIX_DESC;
            }
            else if (thelist.Count != 0)
            {
                this.phoneList.Text = "Choose one";
            }
            else
            {
                this.phoneList.Text = "None";
            }
        }


        /// <summary>
        /// 
        /// </summary>
        /// <param name="phoneNumber"></param>
        /// <returns></returns>
        String removeDesc(String phoneNumber)
        {
            int index = 0;
            if ((index = phoneNumber.LastIndexOf(Constants.PREFIX_DESC)) != -1)
            {
                return phoneNumber.Substring(0, index);
            }

            return phoneNumber;
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="phoneNumber"></param>
        /// <param name="desc"></param>
        /// <returns></returns>
        String addDesc(String phoneNumber, String desc)
        {
            return phoneNumber + Constants.PREFIX_DESC + desc + Constants.SUBFIX_DESC;
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void okButton_Click(object sender, EventArgs e)
        {

            String str = removeDesc( this.phoneList.Text );
            if (Utility.checkInvalidLetters(str))
            {
                MessageBox.Show("Invalid letter found! only [0-9][a-z][A-B] allowed!");
                this.DialogResult = DialogResult.None;
                return;
            }

            DestinationNumber = str;
            this.Close();
        }

        private void cancelButton_Click(object sender, EventArgs e)
        {
            this.Close();
        }

        private void phoneList_TextChanged(object sender, EventArgs e)
        {
            String str = removeDesc(this.phoneList.Text);
            if (Utility.checkInvalidLetters(str))
            {
                MessageBox.Show("Invalid letter found! only [0-9][a-z][A-B] allowed!");
            }

        }


    }
}