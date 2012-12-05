using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Diagnostics;
using System.Xml;
using System.IO;
using System.Text.RegularExpressions;
using AddinUtility;


//string cwd = Directory.GetCurrentDirectory();


namespace StackTraceUI
{
    public partial class StackTraceUI : Form
    {
        //public static AppSettings appSettings = new AppSettings();
        public static readonly String kPrevConfig = "PrevConfig.config";
        private Dictionary<string, TextBox> tbDict = new Dictionary<string, TextBox>();

        
        // Constructor for StackTraceUI form
        //
        public StackTraceUI()
        {
            InitializeComponent();

            if (!String.IsNullOrEmpty(Program.dataLogFile))
               txtDlogName.Text = "\"" + Program.dataLogFile + "\"";
            else if (Clipboard.ContainsText())
                txtDlogName.Text = Clipboard.GetData(DataFormats.Text).ToString();                    

            tbDict.Add("location", textBox1);
            tbDict.Add("ospath", textBox2);
            tbDict.Add("version", textBox3);
            tbDict.Add("osfile", textBox4);
            tbDict.Add("alias", textBox5);
            tbDict.Add("address", textBox6);

            LoadOptionsConfig(Path.Combine(Program.exeDir, kPrevConfig));
        }


        private void btnStackTrace_Click(object sender, EventArgs e)
        {
            CreateOptionsConfig(Path.Combine(Program.exeDir, kPrevConfig));
   
            string dlogName = null;
            string args = txtDlogName.Text.Trim();
            if (args == "")
            {
                MessageBox.Show("Enter Datalog name above");
                return;
            }

            // Extract data log file name (may or may not be enclosed with quotes)
            //
            int loc = -1; // index position of dlog file name
            if (args.EndsWith("\""))
            {
               if (args.Length > 2)
                  loc = args.LastIndexOf('\"', args.Length-2);
               if (loc == -1)
               {
                  MessageBox.Show("Incorrect use of quotes");
                  return;
               }
               else
               {
                  dlogName = args.Substring(loc+1, args.Length-2-(loc+1)+1);
               }
            }
            else
            {
               dlogName = args.Split(new char[]{' '}).Last();
               loc = args.LastIndexOf(dlogName);
            }

            // If dlog file name doesn't include full path and not in current directory,
            //  replace dlog file name with full path dlog file name if it can be guessed.
            // May return empty string if it couldn't be located.
            //
            string dlogFullName = AddinUtility.DatalogPathname.GetCompletePathname(dlogName);
            if(dlogFullName != dlogName)
                args = args.Replace(dlogName, dlogFullName);


            // Get any options that have been filled or loaded, and append to args
            //
            Dictionary<string, string> cfgValues = new Dictionary<string, string>();

            foreach (var tbDictPair in tbDict)
               cfgValues[tbDictPair.Key] = tbDictPair.Value.Text.Trim();

            foreach (var cfgValuesPair in cfgValues)
               if (!String.IsNullOrEmpty(cfgValuesPair.Value))
                  args = args.Insert(loc, "-" + cfgValuesPair.Key + " " + cfgValuesPair.Value + " ");
                   

            // Pass everything to the command line tool, making sure working directory is set
            //  to installation directory
            //
            var startInfo = new ProcessStartInfo();
            startInfo.WorkingDirectory = Program.exeDir;
            startInfo.FileName = Path.Combine(Program.exeDir, "StackTrace.exe");
            startInfo.Arguments = args;
            //Process.Start(Path.Combine(Program.exeDir, "StackTrace.exe"), args);
            Process.Start(startInfo);
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


        private void viewToolStripMenuItem_Click(object sender, EventArgs e) { }


        private void openToolStripMenuItem_Click(object sender, EventArgs e)
        {
            OpenFileDialog ofd = new OpenFileDialog();
            ofd.Filter = "Datalog Files (*.dlog)|*.dlog|All files (*.*)|*.*";
            ofd.Title = "Choose Datalog File";
            ofd.ShowDialog();

            if (!String.IsNullOrEmpty(ofd.FileName))
                txtDlogName.Text = "\"" + ofd.FileName + "\"";
            
            txtDlogName.Focus();
        }


        private void loadConfigToolStripMenuItem_Click(object sender, EventArgs e) 
        {
            OpenFileDialog ofd = new OpenFileDialog();
            ofd.Filter = "Options Config Files (*.config)|*.config";
            ofd.Title = "Choose Options Config File";
            ofd.ShowDialog();
            string xmlFileName = ofd.FileName;

            if (!String.IsNullOrEmpty(xmlFileName))
            { 
               btnClearConfig_Click(sender, e);
               LoadOptionsConfig(xmlFileName);
            }
        }


        private void saveConfigToolStripMenuItem_Click(object sender, EventArgs e)
        {
            SaveFileDialog sfd = new SaveFileDialog();
            sfd.Title = "Save Options Config File As";
            sfd.Filter = "Options Config Files (*.config)|*.config";
            sfd.ShowDialog();
            string xmlFileName = sfd.FileName;

            if (!String.IsNullOrEmpty(xmlFileName))
               CreateOptionsConfig(xmlFileName);
        }


        private void btnClearConfig_Click(object sender, EventArgs e) 
        {
            foreach (var tbDictValue in tbDict.Values)
               tbDictValue.Text = "";
        }


        private void aboutStackTraceToolToolStripMenuItem_Click(object sender, EventArgs e)
        {
            About about = new About();
            about.Show();
        }


	     private void enterPressed_KeyPress(object sender, KeyPressEventArgs e)
	     {
		      if (e.KeyChar == (char)13)
            {
               e.Handled = true;
               btnStackTrace_Click(sender, null);
            }
	     }


        private bool LoadOptionsConfig(String fileName)
        {
            bool result = false;
            if (File.Exists(fileName))
            {  
               using (XmlReader reader = XmlReader.Create(fileName))
               {
                  while (reader.Read())
                  {
                     if (reader.IsStartElement())
                     {
                        string readerName = reader.Name;
                        if (readerName == "cfg_values")
                           continue;
                        else if (reader.Read() && tbDict.ContainsKey(readerName))
                           tbDict[readerName].Text = reader.Value;
                     }
                  }
               }
               result = true;
            }
            return result;
        }

        private void CreateOptionsConfig(String fileName)
        {
            Dictionary<string, string> cfgValues = new Dictionary<string, string>();

            foreach (var tbDictPair in tbDict)
               cfgValues[tbDictPair.Key] = tbDictPair.Value.Text.Trim();

            using (XmlWriter writer = XmlWriter.Create(fileName))
            {
               writer.WriteStartDocument();
               writer.WriteStartElement("cfg_values");

               foreach (var cfgValuesPair in cfgValues)
                 if (!String.IsNullOrEmpty(cfgValuesPair.Value))
                    writer.WriteElementString(cfgValuesPair.Key, cfgValuesPair.Value);

               writer.WriteEndElement();
               writer.WriteEndDocument();
            }
        }
            

    } // end class StackTraceUI 

} // end namespace
