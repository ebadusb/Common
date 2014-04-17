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
           this.components = new System.ComponentModel.Container();
           System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(StackTraceUI));
           this.txtDlogName = new System.Windows.Forms.TextBox();
           this.menuStrip1 = new System.Windows.Forms.MenuStrip();
           this.fileToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
           this.openToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
           this.exitToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
           this.configToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
           this.loadConfigToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
           this.saveConfigToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
           this.helpToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
           this.aboutStackTraceToolToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
           this.btnStackTrace = new System.Windows.Forms.Button();
           this.btnOpenDlog = new System.Windows.Forms.Button();
           this.textBox1 = new System.Windows.Forms.TextBox();
           this.textBox2 = new System.Windows.Forms.TextBox();
           this.textBox3 = new System.Windows.Forms.TextBox();
           this.textBox4 = new System.Windows.Forms.TextBox();
           this.textBox5 = new System.Windows.Forms.TextBox();
           this.textBox6 = new System.Windows.Forms.TextBox();
           this.label1 = new System.Windows.Forms.Label();
           this.label2 = new System.Windows.Forms.Label();
           this.label3 = new System.Windows.Forms.Label();
           this.label4 = new System.Windows.Forms.Label();
           this.label5 = new System.Windows.Forms.Label();
           this.label6 = new System.Windows.Forms.Label();
           this.label7 = new System.Windows.Forms.Label();
           this.label8 = new System.Windows.Forms.Label();
           this.button1 = new System.Windows.Forms.Button();
           this.button2 = new System.Windows.Forms.Button();
           this.button3 = new System.Windows.Forms.Button();
           this.toolTip1 = new System.Windows.Forms.ToolTip(this.components);
           this.menuStrip1.SuspendLayout();
           this.SuspendLayout();
           // 
           // txtDlogName
           // 
           this.txtDlogName.Location = new System.Drawing.Point(93, 37);
           this.txtDlogName.Name = "txtDlogName";
           this.txtDlogName.Size = new System.Drawing.Size(344, 20);
           this.txtDlogName.TabIndex = 1;
           this.toolTip1.SetToolTip(this.txtDlogName, "Enter data log file name (full path optional)");
           this.txtDlogName.KeyPress += new System.Windows.Forms.KeyPressEventHandler(this.enterPressed_KeyPress);
           // 
           // menuStrip1
           // 
           this.menuStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.fileToolStripMenuItem,
            this.configToolStripMenuItem,
            this.helpToolStripMenuItem});
           this.menuStrip1.Location = new System.Drawing.Point(0, 0);
           this.menuStrip1.Name = "menuStrip1";
           this.menuStrip1.Size = new System.Drawing.Size(562, 24);
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
           this.openToolStripMenuItem.Size = new System.Drawing.Size(109, 22);
           this.openToolStripMenuItem.Text = "&Open...";
           this.openToolStripMenuItem.Click += new System.EventHandler(this.openToolStripMenuItem_Click);
           // 
           // exitToolStripMenuItem
           // 
           this.exitToolStripMenuItem.Name = "exitToolStripMenuItem";
           this.exitToolStripMenuItem.Size = new System.Drawing.Size(109, 22);
           this.exitToolStripMenuItem.Text = "&Exit";
           this.exitToolStripMenuItem.Click += new System.EventHandler(this.exitToolStripMenuItem_Click);
           // 
           // configToolStripMenuItem
           // 
           this.configToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.loadConfigToolStripMenuItem,
            this.saveConfigToolStripMenuItem});
           this.configToolStripMenuItem.Name = "configToolStripMenuItem";
           this.configToolStripMenuItem.Size = new System.Drawing.Size(49, 20);
           this.configToolStripMenuItem.Text = "&Config";
           // 
           // loadConfigToolStripMenuItem
           // 
           this.loadConfigToolStripMenuItem.Name = "loadConfigToolStripMenuItem";
           this.loadConfigToolStripMenuItem.Size = new System.Drawing.Size(140, 22);
           this.loadConfigToolStripMenuItem.Text = "&Load Config...";
           this.loadConfigToolStripMenuItem.Click += new System.EventHandler(this.loadConfigToolStripMenuItem_Click);
           // 
           // saveConfigToolStripMenuItem
           // 
           this.saveConfigToolStripMenuItem.Name = "saveConfigToolStripMenuItem";
           this.saveConfigToolStripMenuItem.Size = new System.Drawing.Size(140, 22);
           this.saveConfigToolStripMenuItem.Text = "&Save Config";
           this.saveConfigToolStripMenuItem.Click += new System.EventHandler(this.saveConfigToolStripMenuItem_Click);
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
           this.btnStackTrace.Font = new System.Drawing.Font("Comic Sans MS", 8F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
           this.btnStackTrace.ForeColor = System.Drawing.SystemColors.HotTrack;
           this.btnStackTrace.Location = new System.Drawing.Point(12, 291);
           this.btnStackTrace.Name = "btnStackTrace";
           this.btnStackTrace.Size = new System.Drawing.Size(83, 28);
           this.btnStackTrace.TabIndex = 5;
           this.btnStackTrace.Text = "&Stack Trace";
           this.btnStackTrace.UseVisualStyleBackColor = true;
           this.btnStackTrace.Click += new System.EventHandler(this.btnStackTrace_Click);
           // 
           // btnOpenDlog
           // 
           this.btnOpenDlog.Font = new System.Drawing.Font("Comic Sans MS", 8F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
           this.btnOpenDlog.ForeColor = System.Drawing.SystemColors.HotTrack;
           this.btnOpenDlog.Location = new System.Drawing.Point(101, 291);
           this.btnOpenDlog.Name = "btnOpenDlog";
           this.btnOpenDlog.Size = new System.Drawing.Size(75, 28);
           this.btnOpenDlog.TabIndex = 6;
           this.btnOpenDlog.Text = "&Open Dlog";
           this.btnOpenDlog.UseVisualStyleBackColor = true;
           this.btnOpenDlog.Click += new System.EventHandler(this.btnOpenDlog_Click);
           // 
           // textBox1
           // 
           this.textBox1.Location = new System.Drawing.Point(186, 97);
           this.textBox1.Name = "textBox1";
           this.textBox1.Size = new System.Drawing.Size(344, 20);
           this.textBox1.TabIndex = 7;
           this.toolTip1.SetToolTip(this.textBox1, "Full path to build location");
           this.textBox1.KeyPress += new System.Windows.Forms.KeyPressEventHandler(this.enterPressed_KeyPress);
           // 
           // textBox2
           // 
           this.textBox2.Location = new System.Drawing.Point(186, 123);
           this.textBox2.Name = "textBox2";
           this.textBox2.Size = new System.Drawing.Size(344, 20);
           this.textBox2.TabIndex = 8;
           this.toolTip1.SetToolTip(this.textBox2, "Comma-delimited list of relative paths to OS images (relative to current_build di" +
                   "rectory): control, apc (if applicable), and safety");
           this.textBox2.KeyPress += new System.Windows.Forms.KeyPressEventHandler(this.enterPressed_KeyPress);
           // 
           // textBox3
           // 
           this.textBox3.Location = new System.Drawing.Point(186, 149);
           this.textBox3.Name = "textBox3";
           this.textBox3.Size = new System.Drawing.Size(344, 20);
           this.textBox3.TabIndex = 9;
           this.toolTip1.SetToolTip(this.textBox3, "Toolchain version (2.02 for older builds, 2.2 for newer builds)");
           this.textBox3.KeyPress += new System.Windows.Forms.KeyPressEventHandler(this.enterPressed_KeyPress);
           // 
           // textBox4
           // 
           this.textBox4.Location = new System.Drawing.Point(186, 175);
           this.textBox4.Name = "textBox4";
           this.textBox4.Size = new System.Drawing.Size(344, 20);
           this.textBox4.TabIndex = 10;
           this.toolTip1.SetToolTip(this.textBox4, "OS image name (vxWorks, vxWorks_ampro, vxWorks_versalogic, etc.)");
           this.textBox4.KeyPress += new System.Windows.Forms.KeyPressEventHandler(this.enterPressed_KeyPress);
           // 
           // textBox5
           // 
           this.textBox5.Location = new System.Drawing.Point(186, 201);
           this.textBox5.Name = "textBox5";
           this.textBox5.Size = new System.Drawing.Size(344, 20);
           this.textBox5.TabIndex = 11;
           this.toolTip1.SetToolTip(this.textBox5, "Replace path or file strings with new strings (format [old:new])");
           this.textBox5.KeyPress += new System.Windows.Forms.KeyPressEventHandler(this.enterPressed_KeyPress);
           // 
           // textBox6
           // 
           this.textBox6.Location = new System.Drawing.Point(186, 227);
           this.textBox6.Name = "textBox6";
           this.textBox6.Size = new System.Drawing.Size(344, 20);
           this.textBox6.TabIndex = 12;
           this.toolTip1.SetToolTip(this.textBox6, "Comma-delimited list of machine-side addressess (in hex)");
           this.textBox6.KeyPress += new System.Windows.Forms.KeyPressEventHandler(this.enterPressed_KeyPress);
           // 
           // label1
           // 
           this.label1.AutoSize = true;
           this.label1.Location = new System.Drawing.Point(132, 100);
           this.label1.Name = "label1";
           this.label1.Size = new System.Drawing.Size(44, 13);
           this.label1.TabIndex = 13;
           this.label1.Text = "location";
           // 
           // label2
           // 
           this.label2.AutoSize = true;
           this.label2.Location = new System.Drawing.Point(137, 126);
           this.label2.Name = "label2";
           this.label2.Size = new System.Drawing.Size(39, 13);
           this.label2.TabIndex = 14;
           this.label2.Text = "ospath";
           // 
           // label3
           // 
           this.label3.AutoSize = true;
           this.label3.Location = new System.Drawing.Point(135, 152);
           this.label3.Name = "label3";
           this.label3.Size = new System.Drawing.Size(41, 13);
           this.label3.TabIndex = 15;
           this.label3.Text = "version";
           // 
           // label4
           // 
           this.label4.AutoSize = true;
           this.label4.Location = new System.Drawing.Point(145, 178);
           this.label4.Name = "label4";
           this.label4.Size = new System.Drawing.Size(31, 13);
           this.label4.TabIndex = 16;
           this.label4.Text = "osfile";
           // 
           // label5
           // 
           this.label5.AutoSize = true;
           this.label5.Location = new System.Drawing.Point(127, 204);
           this.label5.Name = "label5";
           this.label5.Size = new System.Drawing.Size(49, 13);
           this.label5.TabIndex = 17;
           this.label5.Text = "pathalias";
           // 
           // label6
           // 
           this.label6.AutoSize = true;
           this.label6.Location = new System.Drawing.Point(132, 230);
           this.label6.Name = "label6";
           this.label6.Size = new System.Drawing.Size(44, 13);
           this.label6.TabIndex = 18;
           this.label6.Text = "address";
           // 
           // label7
           // 
           this.label7.AutoSize = true;
           this.label7.Location = new System.Drawing.Point(9, 40);
           this.label7.Name = "label7";
           this.label7.Size = new System.Drawing.Size(70, 13);
           this.label7.TabIndex = 19;
           this.label7.Text = "Data Log File";
           // 
           // label8
           // 
           this.label8.AutoSize = true;
           this.label8.Font = new System.Drawing.Font("Microsoft Sans Serif", 9F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
           this.label8.Location = new System.Drawing.Point(183, 73);
           this.label8.Name = "label8";
           this.label8.Size = new System.Drawing.Size(182, 15);
           this.label8.TabIndex = 20;
           this.label8.Text = "StackTraceUI Configuration";
           // 
           // button1
           // 
           this.button1.Location = new System.Drawing.Point(421, 257);
           this.button1.Name = "button1";
           this.button1.Size = new System.Drawing.Size(109, 21);
           this.button1.TabIndex = 21;
           this.button1.Text = "C&lear Config";
           this.button1.UseVisualStyleBackColor = true;
           this.button1.Click += new System.EventHandler(this.btnClearConfig_Click);
           // 
           // button2
           // 
           this.button2.Location = new System.Drawing.Point(186, 257);
           this.button2.Name = "button2";
           this.button2.Size = new System.Drawing.Size(101, 21);
           this.button2.TabIndex = 22;
           this.button2.Text = "&Load Config";
           this.button2.UseVisualStyleBackColor = true;
           this.button2.Click += new System.EventHandler(this.loadConfigToolStripMenuItem_Click);
           // 
           // button3
           // 
           this.button3.Location = new System.Drawing.Point(293, 257);
           this.button3.Name = "button3";
           this.button3.Size = new System.Drawing.Size(122, 21);
           this.button3.TabIndex = 23;
           this.button3.Text = "Save Config";
           this.button3.UseVisualStyleBackColor = true;
           this.button3.Click += new System.EventHandler(this.saveConfigToolStripMenuItem_Click);
           // 
           // StackTraceUI
           // 
           this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
           this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
           this.ClientSize = new System.Drawing.Size(562, 327);
           this.Controls.Add(this.button3);
           this.Controls.Add(this.button2);
           this.Controls.Add(this.button1);
           this.Controls.Add(this.label8);
           this.Controls.Add(this.label7);
           this.Controls.Add(this.label6);
           this.Controls.Add(this.label5);
           this.Controls.Add(this.label4);
           this.Controls.Add(this.label3);
           this.Controls.Add(this.label2);
           this.Controls.Add(this.label1);
           this.Controls.Add(this.textBox6);
           this.Controls.Add(this.textBox5);
           this.Controls.Add(this.textBox4);
           this.Controls.Add(this.textBox3);
           this.Controls.Add(this.textBox2);
           this.Controls.Add(this.textBox1);
           this.Controls.Add(this.btnOpenDlog);
           this.Controls.Add(this.btnStackTrace);
           this.Controls.Add(this.txtDlogName);
           this.Controls.Add(this.menuStrip1);
           this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
           this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
           this.MainMenuStrip = this.menuStrip1;
           this.Name = "StackTraceUI";
           this.Text = "StackTraceUI";
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
        private System.Windows.Forms.ToolStripMenuItem configToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem loadConfigToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem saveConfigToolStripMenuItem;
        private System.Windows.Forms.TextBox textBox1;
        private System.Windows.Forms.TextBox textBox2;
        private System.Windows.Forms.TextBox textBox3;
        private System.Windows.Forms.TextBox textBox4;
        private System.Windows.Forms.TextBox textBox5;
        private System.Windows.Forms.TextBox textBox6;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.Label label7;
        private System.Windows.Forms.Label label8;
        private System.Windows.Forms.Button button1;
        private System.Windows.Forms.Button button2;
        private System.Windows.Forms.Button button3;
        private System.Windows.Forms.ToolTip toolTip1;
    }
}

