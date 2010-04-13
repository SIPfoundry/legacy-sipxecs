namespace AddinForOutlook
{
    partial class PassCodeWindow
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            this.label1 = new System.Windows.Forms.Label();
            this.SCSPasswd = new System.Windows.Forms.TextBox();
            this.buttonOK = new System.Windows.Forms.Button();
            this.toolTip1 = new System.Windows.Forms.ToolTip(this.components);
            this.isPasswordRemembered = new System.Windows.Forms.CheckBox();
            this.cancelButton = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(38, 46);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(119, 17);
            this.label1.TabIndex = 0;
            this.label1.Text = "User Account PIN";
            // 
            // SCSPasswd
            // 
            this.SCSPasswd.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.SCSPasswd.Location = new System.Drawing.Point(175, 44);
            this.SCSPasswd.Name = "SCSPasswd";
            this.SCSPasswd.PasswordChar = '*';
            this.SCSPasswd.Size = new System.Drawing.Size(161, 22);
            this.SCSPasswd.TabIndex = 1;
            this.toolTip1.SetToolTip(this.SCSPasswd, "The SIP user account\'s PIN number");
            this.SCSPasswd.TextChanged += new System.EventHandler(this.SCSPasswd_TextChanged);
            // 
            // buttonOK
            // 
            this.buttonOK.DialogResult = System.Windows.Forms.DialogResult.OK;
            this.buttonOK.Location = new System.Drawing.Point(64, 125);
            this.buttonOK.Name = "buttonOK";
            this.buttonOK.Size = new System.Drawing.Size(75, 23);
            this.buttonOK.TabIndex = 2;
            this.buttonOK.Text = "OK";
            this.buttonOK.UseVisualStyleBackColor = true;
            this.buttonOK.Click += new System.EventHandler(this.buttonOK_Click);
            // 
            // isPasswordRemembered
            // 
            this.isPasswordRemembered.AutoSize = true;
            this.isPasswordRemembered.Checked = true;
            this.isPasswordRemembered.CheckState = System.Windows.Forms.CheckState.Checked;
            this.isPasswordRemembered.Location = new System.Drawing.Point(175, 86);
            this.isPasswordRemembered.Name = "isPasswordRemembered";
            this.isPasswordRemembered.Size = new System.Drawing.Size(149, 21);
            this.isPasswordRemembered.TabIndex = 3;
            this.isPasswordRemembered.Text = "Remember the PIN";
            this.isPasswordRemembered.UseVisualStyleBackColor = true;
            // 
            // cancelButton
            // 
            this.cancelButton.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.cancelButton.Location = new System.Drawing.Point(218, 125);
            this.cancelButton.Name = "cancelButton";
            this.cancelButton.Size = new System.Drawing.Size(75, 23);
            this.cancelButton.TabIndex = 4;
            this.cancelButton.Text = "Cancel";
            this.cancelButton.UseVisualStyleBackColor = true;
            this.cancelButton.Click += new System.EventHandler(this.Cancel_Click);
            // 
            // PassCodeWindow
            // 
            this.AcceptButton = this.buttonOK;
            this.AutoScaleDimensions = new System.Drawing.SizeF(8F, 16F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.CancelButton = this.cancelButton;
            this.ClientSize = new System.Drawing.Size(389, 160);
            this.Controls.Add(this.cancelButton);
            this.Controls.Add(this.isPasswordRemembered);
            this.Controls.Add(this.buttonOK);
            this.Controls.Add(this.SCSPasswd);
            this.Controls.Add(this.label1);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.Name = "PassCodeWindow";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "User Account PIN";
            this.Load += new System.EventHandler(this.PassCodeWindow_Load);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.TextBox SCSPasswd;
        private System.Windows.Forms.Button buttonOK;
        private System.Windows.Forms.ToolTip toolTip1;
        private System.Windows.Forms.CheckBox isPasswordRemembered;
        private System.Windows.Forms.Button cancelButton;
    }
}