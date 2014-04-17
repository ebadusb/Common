namespace StackTraceUI
{
    partial class About
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
           System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(About));
           this.label1 = new System.Windows.Forms.Label();
           this.lblAbout = new System.Windows.Forms.Label();
           this.SuspendLayout();
           // 
           // label1
           // 
           this.label1.AutoSize = true;
           this.label1.Location = new System.Drawing.Point(13, 13);
           this.label1.Name = "label1";
           this.label1.Size = new System.Drawing.Size(0, 13);
           this.label1.TabIndex = 0;
           // 
           // lblAbout
           // 
           this.lblAbout.AutoSize = true;
           this.lblAbout.Font = new System.Drawing.Font("Courier New", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
           this.lblAbout.Location = new System.Drawing.Point(12, 9);
           this.lblAbout.Name = "lblAbout";
           this.lblAbout.Size = new System.Drawing.Size(56, 16);
           this.lblAbout.TabIndex = 1;
           this.lblAbout.Text = "label2";
           // 
           // About
           // 
           this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
           this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
           this.AutoScroll = true;
           this.AutoSize = true;
           this.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
           this.ClientSize = new System.Drawing.Size(645, 494);
           this.Controls.Add(this.lblAbout);
           this.Controls.Add(this.label1);
           this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
           this.Name = "About";
           this.Text = "About StackTraceUI";
           this.ResumeLayout(false);
           this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label lblAbout;
    }
}