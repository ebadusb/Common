namespace StackTraceUI
{
    partial class StackTraceUI
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(StackTraceUI));
            this.txtDlogName = new System.Windows.Forms.TextBox();
            this.menuStrip1 = new System.Windows.Forms.MenuStrip();
            this.fileToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.openToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.exitToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.helpToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.aboutStackTraceToolToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.btnStackTrace = new System.Windows.Forms.Button();
            this.btnOpenDlog = new System.Windows.Forms.Button();
            this.menuStrip1.SuspendLayout();
            this.SuspendLayout();
            // 
            // txtDlogName
            // 
            this.txtDlogName.Location = new System.Drawing.Point(12, 26);
            this.txtDlogName.Name = "txtDlogName";
            this.txtDlogName.Size = new System.Drawing.Size(344, 20);
            this.txtDlogName.TabIndex = 1;
            // 
            // menuStrip1
            // 
            this.menuStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.fileToolStripMenuItem,
            this.helpToolStripMenuItem});
            this.menuStrip1.Location = new System.Drawing.Point(0, 0);
            this.menuStrip1.Name = "menuStrip1";
            this.menuStrip1.Size = new System.Drawing.Size(367, 24);
            this.menuStrip1.TabIndex = 4;
            this.menuStrip1.Text = "menuStrip1";
            // 
            // fileToolStripMenuItem
            // 
            this.fileToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.openToolStripMenuItem,
            this.exitToolStripMenuItem});
            this.fileToolStripMenuItem.Name = "fileToolStripMenuItem";
            this.fileToolStripMenuItem.Size = new System.Drawing.Size(35, 20);
            this.fileToolStripMenuItem.Text = "&File";
            // 
            // openToolStripMenuItem
            // 
            this.openToolStripMenuItem.Name = "openToolStripMenuItem";
            this.openToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
            this.openToolStripMenuItem.Text = "&Open...";
            this.openToolStripMenuItem.Click += new System.EventHandler(this.openToolStripMenuItem_Click);
            // 
            // exitToolStripMenuItem
            // 
            this.exitToolStripMenuItem.Name = "exitToolStripMenuItem";
            this.exitToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
            this.exitToolStripMenuItem.Text = "&Exit";
            this.exitToolStripMenuItem.Click += new System.EventHandler(this.exitToolStripMenuItem_Click);
            // 
            // helpToolStripMenuItem
            // 
            this.helpToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.aboutStackTraceToolToolStripMenuItem});
            this.helpToolStripMenuItem.Name = "helpToolStripMenuItem";
            this.helpToolStripMenuItem.Size = new System.Drawing.Size(41, 20);
            this.helpToolStripMenuItem.Text = "&Help";
            // 
            // aboutStackTraceToolToolStripMenuItem
            // 
            this.aboutStackTraceToolToolStripMenuItem.Name = "aboutStackTraceToolToolStripMenuItem";
            this.aboutStackTraceToolToolStripMenuItem.Size = new System.Drawing.Size(188, 22);
            this.aboutStackTraceToolToolStripMenuItem.Text = "&About Stack Trace Tool";
            this.aboutStackTraceToolToolStripMenuItem.Click += new System.EventHandler(this.aboutStackTraceToolToolStripMenuItem_Click);
            // 
            // btnStackTrace
            // 
            this.btnStackTrace.Location = new System.Drawing.Point(12, 55);
            this.btnStackTrace.Name = "btnStackTrace";
            this.btnStackTrace.Size = new System.Drawing.Size(75, 23);
            this.btnStackTrace.TabIndex = 5;
            this.btnStackTrace.Text = "&Stack Trace";
            this.btnStackTrace.UseVisualStyleBackColor = true;
            this.btnStackTrace.Click += new System.EventHandler(this.btnStackTrace_Click);
            // 
            // btnOpenDlog
            // 
            this.btnOpenDlog.Location = new System.Drawing.Point(125, 55);
            this.btnOpenDlog.Name = "btnOpenDlog";
            this.btnOpenDlog.Size = new System.Drawing.Size(75, 23);
            this.btnOpenDlog.TabIndex = 6;
            this.btnOpenDlog.Text = "&Open Dlog";
            this.btnOpenDlog.UseVisualStyleBackColor = true;
            this.btnOpenDlog.Click += new System.EventHandler(this.btnOpenDlog_Click);
            // 
            // StackTraceUI
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(367, 88);
            this.Controls.Add(this.btnOpenDlog);
            this.Controls.Add(this.btnStackTrace);
            this.Controls.Add(this.txtDlogName);
            this.Controls.Add(this.menuStrip1);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.MainMenuStrip = this.menuStrip1;
            this.Name = "StackTraceUI";
            this.Text = "Stack Trace Tool";
            this.menuStrip1.ResumeLayout(false);
            this.menuStrip1.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.TextBox txtDlogName;
        private System.Windows.Forms.MenuStrip menuStrip1;
        private System.Windows.Forms.ToolStripMenuItem fileToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem openToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem exitToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem helpToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem aboutStackTraceToolToolStripMenuItem;
        private System.Windows.Forms.Button btnStackTrace;
        private System.Windows.Forms.Button btnOpenDlog;
    }
}

