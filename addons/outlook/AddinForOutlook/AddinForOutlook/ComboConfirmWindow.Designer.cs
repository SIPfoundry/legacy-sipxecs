namespace AddinForOutlook
{
    partial class ComboConfirmWindow
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
            this.promptText = new System.Windows.Forms.Label();
            this.yesButton = new System.Windows.Forms.Button();
            this.nolButton = new System.Windows.Forms.Button();
            this.phoneList = new System.Windows.Forms.ComboBox();
            this.SuspendLayout();
            // 
            // promptText
            // 
            this.promptText.AutoSize = true;
            this.promptText.Location = new System.Drawing.Point(38, 38);
            this.promptText.Name = "promptText";
            this.promptText.Size = new System.Drawing.Size(367, 17);
            this.promptText.TabIndex = 1;
            this.promptText.Text = "Call the contact at the number below? (or type a number)";
            // 
            // yesButton
            // 
            this.yesButton.DialogResult = System.Windows.Forms.DialogResult.Yes;
            this.yesButton.Location = new System.Drawing.Point(106, 122);
            this.yesButton.Name = "yesButton";
            this.yesButton.Size = new System.Drawing.Size(75, 23);
            this.yesButton.TabIndex = 3;
            this.yesButton.Text = "OK";
            this.yesButton.UseVisualStyleBackColor = true;
            this.yesButton.Click += new System.EventHandler(this.okButton_Click);
            // 
            // nolButton
            // 
            this.nolButton.DialogResult = System.Windows.Forms.DialogResult.No;
            this.nolButton.Location = new System.Drawing.Point(227, 122);
            this.nolButton.Name = "nolButton";
            this.nolButton.Size = new System.Drawing.Size(75, 23);
            this.nolButton.TabIndex = 4;
            this.nolButton.Text = "Cancel";
            this.nolButton.UseVisualStyleBackColor = true;
            this.nolButton.Click += new System.EventHandler(this.cancelButton_Click);
            // 
            // phoneList
            // 
            this.phoneList.FormattingEnabled = true;
            this.phoneList.Location = new System.Drawing.Point(63, 67);
            this.phoneList.MaxLength = 32;
            this.phoneList.Name = "phoneList";
            this.phoneList.Size = new System.Drawing.Size(342, 24);
            this.phoneList.TabIndex = 5;
            this.phoneList.TextChanged += new System.EventHandler(this.phoneList_TextChanged);
            // 
            // ComboConfirmWindow
            // 
            this.AcceptButton = this.yesButton;
            this.AutoScaleDimensions = new System.Drawing.SizeF(8F, 16F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.CancelButton = this.nolButton;
            this.ClientSize = new System.Drawing.Size(491, 167);
            this.Controls.Add(this.phoneList);
            this.Controls.Add(this.nolButton);
            this.Controls.Add(this.yesButton);
            this.Controls.Add(this.promptText);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.Name = "ComboConfirmWindow";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "Confirm To Call";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label promptText;
        private System.Windows.Forms.Button yesButton;
        private System.Windows.Forms.Button nolButton;
        private System.Windows.Forms.ComboBox phoneList;
    }
}