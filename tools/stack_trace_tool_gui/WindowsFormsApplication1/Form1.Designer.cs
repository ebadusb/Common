namespace StackTraceUI
{
   partial class Form1
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
         this.textBox1 = new System.Windows.Forms.TextBox();
         this.label1 = new System.Windows.Forms.Label();
         this.textBox2 = new System.Windows.Forms.TextBox();
         this.label2 = new System.Windows.Forms.Label();
         this.label3 = new System.Windows.Forms.Label();
         this.textBox3 = new System.Windows.Forms.TextBox();
         this.menuStrip1 = new System.Windows.Forms.MenuStrip();
         this.fileToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
         this.openToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
         this.exitToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
         this.helpToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
         this.aboutToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
         this.button1 = new System.Windows.Forms.Button();
         this.button2 = new System.Windows.Forms.Button();
         this.openFileDialog1 = new System.Windows.Forms.OpenFileDialog();
         this.label4 = new System.Windows.Forms.Label();
         this.radioButton1 = new System.Windows.Forms.RadioButton();
         this.radioButton2 = new System.Windows.Forms.RadioButton();
         this.menuStrip1.SuspendLayout();
         this.SuspendLayout();
         // 
         // textBox1
         // 
         this.textBox1.Location = new System.Drawing.Point(159, 34);
         this.textBox1.Name = "textBox1";
         this.textBox1.Size = new System.Drawing.Size(308, 20);
         this.textBox1.TabIndex = 1;
         // 
         // label1
         // 
         this.label1.AutoSize = true;
         this.label1.Location = new System.Drawing.Point(5, 93);
         this.label1.Name = "label1";
         this.label1.Size = new System.Drawing.Size(101, 13);
         this.label1.TabIndex = 1;
         this.label1.Text = "OS Paths (optional):";
         // 
         // textBox2
         // 
         this.textBox2.Location = new System.Drawing.Point(159, 60);
         this.textBox2.Name = "textBox2";
         this.textBox2.Size = new System.Drawing.Size(308, 20);
         this.textBox2.TabIndex = 2;
         // 
         // label2
         // 
         this.label2.AutoSize = true;
         this.label2.Location = new System.Drawing.Point(5, 67);
         this.label2.Name = "label2";
         this.label2.Size = new System.Drawing.Size(148, 13);
         this.label2.TabIndex = 3;
         this.label2.Text = "Build Path Location (optional):";
         // 
         // label3
         // 
         this.label3.AutoSize = true;
         this.label3.Location = new System.Drawing.Point(5, 41);
         this.label3.Name = "label3";
         this.label3.Size = new System.Drawing.Size(73, 13);
         this.label3.TabIndex = 4;
         this.label3.Text = "Data Log File:";
         // 
         // textBox3
         // 
         this.textBox3.Location = new System.Drawing.Point(159, 86);
         this.textBox3.Name = "textBox3";
         this.textBox3.Size = new System.Drawing.Size(308, 20);
         this.textBox3.TabIndex = 3;
         // 
         // menuStrip1
         // 
         this.menuStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.fileToolStripMenuItem,
            this.helpToolStripMenuItem});
         this.menuStrip1.Location = new System.Drawing.Point(0, 0);
         this.menuStrip1.Name = "menuStrip1";
         this.menuStrip1.Size = new System.Drawing.Size(500, 24);
         this.menuStrip1.TabIndex = 6;
         this.menuStrip1.Text = "menuStrip1";
         // 
         // fileToolStripMenuItem
         // 
         this.fileToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.openToolStripMenuItem,
            this.exitToolStripMenuItem});
         this.fileToolStripMenuItem.Name = "fileToolStripMenuItem";
         this.fileToolStripMenuItem.Size = new System.Drawing.Size(35, 20);
         this.fileToolStripMenuItem.Text = "File";
         // 
         // openToolStripMenuItem
         // 
         this.openToolStripMenuItem.Name = "openToolStripMenuItem";
         this.openToolStripMenuItem.Size = new System.Drawing.Size(109, 22);
         this.openToolStripMenuItem.Text = "Open...";
         // 
         // exitToolStripMenuItem
         // 
         this.exitToolStripMenuItem.Name = "exitToolStripMenuItem";
         this.exitToolStripMenuItem.Size = new System.Drawing.Size(109, 22);
         this.exitToolStripMenuItem.Text = "Exit";
         // 
         // helpToolStripMenuItem
         // 
         this.helpToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.aboutToolStripMenuItem});
         this.helpToolStripMenuItem.Name = "helpToolStripMenuItem";
         this.helpToolStripMenuItem.Size = new System.Drawing.Size(41, 20);
         this.helpToolStripMenuItem.Text = "Help";
         // 
         // aboutToolStripMenuItem
         // 
         this.aboutToolStripMenuItem.Name = "aboutToolStripMenuItem";
         this.aboutToolStripMenuItem.Size = new System.Drawing.Size(108, 22);
         this.aboutToolStripMenuItem.Text = "About..";
         // 
         // button1
         // 
         this.button1.Location = new System.Drawing.Point(266, 146);
         this.button1.Name = "button1";
         this.button1.Size = new System.Drawing.Size(93, 25);
         this.button1.TabIndex = 0;
         this.button1.Text = "OK";
         this.button1.UseVisualStyleBackColor = true;
         // 
         // button2
         // 
         this.button2.Location = new System.Drawing.Point(374, 146);
         this.button2.Name = "button2";
         this.button2.Size = new System.Drawing.Size(93, 25);
         this.button2.TabIndex = 6;
         this.button2.Text = "Cancel";
         this.button2.UseVisualStyleBackColor = true;
         // 
         // openFileDialog1
         // 
         this.openFileDialog1.DefaultExt = "dlog";
         this.openFileDialog1.FileName = "openFileDialog1";
         this.openFileDialog1.Filter = "Data Log Files|*.dlog";
         this.openFileDialog1.Title = "Choose Data Log File";
         // 
         // label4
         // 
         this.label4.AutoSize = true;
         this.label4.Location = new System.Drawing.Point(5, 119);
         this.label4.Name = "label4";
         this.label4.Size = new System.Drawing.Size(45, 13);
         this.label4.TabIndex = 10;
         this.label4.Text = "Version:";
         // 
         // radioButton1
         // 
         this.radioButton1.AutoSize = true;
         this.radioButton1.CheckAlign = System.Drawing.ContentAlignment.TopLeft;
         this.radioButton1.Location = new System.Drawing.Point(159, 119);
         this.radioButton1.Name = "radioButton1";
         this.radioButton1.Size = new System.Drawing.Size(46, 17);
         this.radioButton1.TabIndex = 4;
         this.radioButton1.TabStop = true;
         this.radioButton1.Text = "2.02";
         this.radioButton1.UseVisualStyleBackColor = true;
         // 
         // radioButton2
         // 
         this.radioButton2.AutoSize = true;
         this.radioButton2.Location = new System.Drawing.Point(211, 119);
         this.radioButton2.Name = "radioButton2";
         this.radioButton2.Size = new System.Drawing.Size(46, 17);
         this.radioButton2.TabIndex = 5;
         this.radioButton2.TabStop = true;
         this.radioButton2.Text = "2.20";
         this.radioButton2.UseVisualStyleBackColor = true;
         // 
         // Form1
         // 
         this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
         this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
         this.ClientSize = new System.Drawing.Size(500, 183);
         this.Controls.Add(this.radioButton2);
         this.Controls.Add(this.radioButton1);
         this.Controls.Add(this.label4);
         this.Controls.Add(this.button2);
         this.Controls.Add(this.button1);
         this.Controls.Add(this.textBox3);
         this.Controls.Add(this.label3);
         this.Controls.Add(this.label2);
         this.Controls.Add(this.textBox2);
         this.Controls.Add(this.label1);
         this.Controls.Add(this.textBox1);
         this.Controls.Add(this.menuStrip1);
         this.MainMenuStrip = this.menuStrip1;
         this.Name = "Form1";
         this.Text = "StackTraceUI";
         this.menuStrip1.ResumeLayout(false);
         this.menuStrip1.PerformLayout();
         this.ResumeLayout(false);
         this.PerformLayout();

      } // end function Initizalize

      #endregion

      private System.Windows.Forms.TextBox textBox1;
      private System.Windows.Forms.Label label1;
      private System.Windows.Forms.TextBox textBox2;
      private System.Windows.Forms.Label label2;
      private System.Windows.Forms.Label label3;
      private System.Windows.Forms.TextBox textBox3;
      private System.Windows.Forms.MenuStrip menuStrip1;
      private System.Windows.Forms.ToolStripMenuItem fileToolStripMenuItem;
      private System.Windows.Forms.ToolStripMenuItem openToolStripMenuItem;
      private System.Windows.Forms.ToolStripMenuItem exitToolStripMenuItem;
      private System.Windows.Forms.ToolStripMenuItem helpToolStripMenuItem;
      private System.Windows.Forms.ToolStripMenuItem aboutToolStripMenuItem;
      private System.Windows.Forms.Button button1;
      private System.Windows.Forms.Button button2;
      private System.Windows.Forms.OpenFileDialog openFileDialog1;
      private System.Windows.Forms.Label label4;
      private System.Windows.Forms.RadioButton radioButton1;
      private System.Windows.Forms.RadioButton radioButton2;
   }

}

