using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Diagnostics;

using AddinUtility;

namespace StackTraceUI
{
    public partial class StackTraceUI : Form
    {
        public static AppSettings appSettings = new AppSettings();

        public StackTraceUI()
        {
            InitializeComponent();
            if (Clipboard.ContainsText())
            {
                txtDlogName.Text = Clipboard.GetData(DataFormats.Text).ToString();                    
            }
        }

        private void btnStackTrace_Click(object sender, EventArgs e)
        {
            string args = txtDlogName.Text.Trim();

            if (args == "")
            {
                MessageBox.Show("Enter Datalog name above");
            }

            string dlogName = null;
            

            if (args.Contains(' '))
            {
                char[] splitChar = { ' ', '\t' };
                string[] splitArgs = args.Split(splitChar);
                dlogName = splitArgs.Last();
            }
            else
            {
                dlogName = txtDlogName.Text;
            }

            string dlogPath = AddinUtility.DatalogPathname.GetCompletePathname(dlogName);
            if(dlogPath != dlogName)
                args = args.Replace(dlogName, dlogPath);


            string exeDirectory = System.Windows.Forms.Application.StartupPath;
            string cwd = System.IO.Directory.GetCurrentDirectory();
            Process.Start(exeDirectory + "\\StackTrace.exe", args);
            Application.Exit();
        }

        private void exitToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Application.Exit();    
        }

        private void btnOpenDlog_Click(object sender, EventArgs e)
        {
            string args = txtDlogName.Text.Trim();
            if (args == "")
            {
                MessageBox.Show("Enter Datalog name above");
            }

            DatalogLauncher.LauncherEngine le = new DatalogLauncher.LauncherEngine();
            le.ProcessFromArgument(args);
            Application.Exit();
        }

        private void viewToolStripMenuItem_Click(object sender, EventArgs e)
        {

        }

        private void openToolStripMenuItem_Click(object sender, EventArgs e)
        {
            OpenFileDialog ofd = new OpenFileDialog();
            ofd.Filter = "Datalog Files (*.dlog)|*.dlog|All files (*.*)|*.*";
            ofd.Title = "Choose Datalog File";
            ofd.ShowDialog();

            if (ofd.CheckFileExists)
            {
                txtDlogName.Text = ofd.FileName;
            }
            
            txtDlogName.Focus();
        }

        private void aboutStackTraceToolToolStripMenuItem_Click(object sender, EventArgs e)
        {
            About about = new About();
            about.Show();
        }



    }
}
