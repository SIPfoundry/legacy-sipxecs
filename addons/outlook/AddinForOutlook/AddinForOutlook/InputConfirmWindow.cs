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
    public partial class InputConfirmWindow : Form
    {
        public string DestinationNumber = String.Empty;

        public InputConfirmWindow(String label, String number)
        {
            InitializeComponent();
            this.promptText.Text = label;
            this.conferenceNumber.Text = this.DestinationNumber = number;
        }

        private void okButton_Click(object sender, EventArgs e)
        {

            if (Utility.checkInvalidLetters(conferenceNumber.Text))
            {
                MessageBox.Show("Invalid letter found! only [0-9][a-z][A-B] allowed!");
                this.DialogResult = DialogResult.None;
                return;
            }

            this.DestinationNumber = this.conferenceNumber.Text;
            this.Close();
        }

        private void cancelButton_Click(object sender, EventArgs e)
        {
            this.Close();
        }

        private void resetButton_Click(object sender, EventArgs e)
        {
            this.conferenceNumber.Text = this.DestinationNumber;
        }

        private void conferenceNumber_TextChanged(object sender, EventArgs e)
        {
            if (Utility.checkInvalidLetters(conferenceNumber.Text))
            {
                MessageBox.Show("Invalid letter found! only [0-9][a-z][A-B] allowed!");
            }
        }

    }
}