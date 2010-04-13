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
using System.Drawing;

namespace AddinForOutlook
{
    using System;
    using System.Drawing;
    using System.Collections;
    using System.ComponentModel;
    using System.Windows.Forms;

    public class BaseEditDialog : System.Windows.Forms.Form
    {
        protected Button button_OK;
        protected Button button_Cancel;
        // These Objects cannot be changed in the derived Forms
        private Label label1;
        private Label label2;
        protected TextBox userName;
        private Label BNLabel;
        protected TextBox ConferenceNumber;
        private Label label6;
        protected TextBox ConferenceAccessCode;
        protected TextBox scsIPAddress;
        protected CheckBox isDefaultLocation;
        private ToolTip toolTip1;
        private IContainer components;
        protected Button button_Reset;


        public BaseEditDialog()
        {
            InitializeComponent();
        }

        // Derived Forms MUST override this method
        protected virtual void ResetSettings()
        {
            // Code logic in derived Form
        }

        // Derived Forms MUST override this method
        protected virtual bool SaveSettings()
        {
            // Code logic in derived Form
            return true;
        }

        // Handle Closing of the Form.
        protected override void OnClosing(CancelEventArgs e)
        {
            // If user clicked the OK button, make sure to
            // save the content. This must be done in the
            // SaveSettings() method in the derived Form.
            if (!e.Cancel && (this.DialogResult == DialogResult.OK))
            {
                // If SaveSettings() is OK (TRUE), then e.Cancel
                // will be FALSE, therefore the application will be exit.
                e.Cancel = !SaveSettings();
            }

            // Make sure any Closing event handler for the
            // form are called before the application exits.
            base.OnClosing(e);
        }

        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            this.button_OK = new System.Windows.Forms.Button();
            this.button_Cancel = new System.Windows.Forms.Button();
            this.label1 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.userName = new System.Windows.Forms.TextBox();
            this.button_Reset = new System.Windows.Forms.Button();
            this.BNLabel = new System.Windows.Forms.Label();
            this.ConferenceNumber = new System.Windows.Forms.TextBox();
            this.label6 = new System.Windows.Forms.Label();
            this.ConferenceAccessCode = new System.Windows.Forms.TextBox();
            this.scsIPAddress = new System.Windows.Forms.TextBox();
            this.isDefaultLocation = new System.Windows.Forms.CheckBox();
            this.toolTip1 = new System.Windows.Forms.ToolTip(this.components);
            this.SuspendLayout();
            // 
            // button_OK
            // 
            this.button_OK.DialogResult = System.Windows.Forms.DialogResult.OK;
            this.button_OK.Location = new System.Drawing.Point(91, 293);
            this.button_OK.Name = "button_OK";
            this.button_OK.Size = new System.Drawing.Size(75, 23);
            this.button_OK.TabIndex = 0;
            this.button_OK.Text = "OK";
            this.button_OK.UseVisualStyleBackColor = true;
            this.button_OK.Click += new System.EventHandler(this.button_OK_Click);
            // 
            // button_Cancel
            // 
            this.button_Cancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.button_Cancel.Location = new System.Drawing.Point(246, 293);
            this.button_Cancel.Name = "button_Cancel";
            this.button_Cancel.Size = new System.Drawing.Size(75, 23);
            this.button_Cancel.TabIndex = 1;
            this.button_Cancel.Text = "Cancel";
            this.button_Cancel.UseVisualStyleBackColor = true;
            this.button_Cancel.Click += new System.EventHandler(this.button_Cancel_Click);
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(75, 45);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(91, 17);
            this.label1.TabIndex = 4;
            this.label1.Text = "Server Name";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(72, 98);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(55, 17);
            this.label2.TabIndex = 5;
            this.label2.Text = "User ID";
            // 
            // userName
            // 
            this.userName.Location = new System.Drawing.Point(265, 98);
            this.userName.MaxLength = 128;
            this.userName.Name = "userName";
            this.userName.Size = new System.Drawing.Size(206, 22);
            this.userName.TabIndex = 6;
            this.userName.Text = "anonymous";
            this.toolTip1.SetToolTip(this.userName, "SIP user account created on the CTI server");
            // 
            // button_Reset
            // 
            this.button_Reset.Location = new System.Drawing.Point(396, 293);
            this.button_Reset.Name = "button_Reset";
            this.button_Reset.Size = new System.Drawing.Size(75, 23);
            this.button_Reset.TabIndex = 9;
            this.button_Reset.Text = "Reset";
            this.button_Reset.UseVisualStyleBackColor = true;
            this.button_Reset.Click += new System.EventHandler(this.button_Reset_Click);
            // 
            // BNLabel
            // 
            this.BNLabel.AutoSize = true;
            this.BNLabel.Location = new System.Drawing.Point(72, 152);
            this.BNLabel.Name = "BNLabel";
            this.BNLabel.Size = new System.Drawing.Size(135, 17);
            this.BNLabel.TabIndex = 12;
            this.BNLabel.Text = "Conference Number";
            // 
            // ConferenceNumber
            // 
            this.ConferenceNumber.Location = new System.Drawing.Point(265, 152);
            this.ConferenceNumber.MaxLength = 16;
            this.ConferenceNumber.Name = "ConferenceNumber";
            this.ConferenceNumber.Size = new System.Drawing.Size(206, 22);
            this.ConferenceNumber.TabIndex = 13;
            this.ConferenceNumber.Text = "200";
            this.toolTip1.SetToolTip(this.ConferenceNumber, "Conference number assigned to the user account  by SCS administrator.");
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Location = new System.Drawing.Point(75, 210);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(184, 17);
            this.label6.TabIndex = 14;
            this.label6.Text = "Conference Access Number";
            // 
            // ConferenceAccessCode
            // 
            this.ConferenceAccessCode.Location = new System.Drawing.Point(265, 207);
            this.ConferenceAccessCode.MaxLength = 32;
            this.ConferenceAccessCode.Name = "ConferenceAccessCode";
            this.ConferenceAccessCode.Size = new System.Drawing.Size(206, 22);
            this.ConferenceAccessCode.TabIndex = 15;
            this.ConferenceAccessCode.Text = "12345678";
            this.toolTip1.SetToolTip(this.ConferenceAccessCode, "Conference Access Code to join the conference.");
            // 
            // scsIPAddress
            // 
            this.scsIPAddress.Location = new System.Drawing.Point(265, 45);
            this.scsIPAddress.MaxLength = 128;
            this.scsIPAddress.Name = "scsIPAddress";
            this.scsIPAddress.Size = new System.Drawing.Size(206, 22);
            this.scsIPAddress.TabIndex = 3;
            this.scsIPAddress.Text = "192.168.0.1";
            this.toolTip1.SetToolTip(this.scsIPAddress, "CTI Server IP address or host name");
            // 
            // isDefaultLocation
            // 
            this.isDefaultLocation.AutoSize = true;
            this.isDefaultLocation.Checked = true;
            this.isDefaultLocation.CheckState = System.Windows.Forms.CheckState.Checked;
            this.isDefaultLocation.Location = new System.Drawing.Point(289, 254);
            this.isDefaultLocation.Name = "isDefaultLocation";
            this.isDefaultLocation.Size = new System.Drawing.Size(182, 21);
            this.isDefaultLocation.TabIndex = 16;
            this.isDefaultLocation.Text = "Default meeting location";
            this.toolTip1.SetToolTip(this.isDefaultLocation, "Check if the bridge is the default meeting location.");
            this.isDefaultLocation.UseVisualStyleBackColor = true;
            // 
            // BaseEditDialog
            // 
            this.AcceptButton = this.button_OK;
            this.CancelButton = this.button_Cancel;
            this.ClientSize = new System.Drawing.Size(554, 364);
            this.Controls.Add(this.isDefaultLocation);
            this.Controls.Add(this.ConferenceAccessCode);
            this.Controls.Add(this.label6);
            this.Controls.Add(this.ConferenceNumber);
            this.Controls.Add(this.BNLabel);
            this.Controls.Add(this.button_Reset);
            this.Controls.Add(this.userName);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.scsIPAddress);
            this.Controls.Add(this.button_Cancel);
            this.Controls.Add(this.button_OK);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.Name = "BaseEditDialog";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "Settings";
            this.Load += new System.EventHandler(this.BaseEditDialog_Load);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        private void button_OK_Click(object sender, EventArgs e)
        {

            try
            {
                this.Close();
            }
            catch (System.Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }

        private void button_Cancel_Click(object sender, EventArgs e)
        {
            try
            {
                this.Close();
            }
            catch (System.Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }

        private void button_Reset_Click(object sender, EventArgs e)
        {
            try
            {
                ResetSettings();
            }
            catch (System.Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }

        private void BaseEditDialog_Load(object sender, EventArgs e)
        {
            try
            {
                ResetSettings();
            }
            catch (System.Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }
    }
}

