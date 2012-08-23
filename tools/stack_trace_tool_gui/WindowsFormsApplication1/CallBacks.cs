

namespace StackTraceUI
{

   partial class Form1 
   {
      private string kConfigFileName = "StackTrace.cfg";
      private string kStackTraceExe = "StackTrace.exe";
      private string kOutputFile;
      private string kReadMeFile = "README.txt";

      private void GetFileNames() 
      {
         kConfigFileName = System.IO.Path.Combine(Program.exeDirectory,kConfigFileName);
         kStackTraceExe = System.IO.Path.Combine(Program.exeDirectory,kStackTraceExe);
         kReadMeFile = System.IO.Path.Combine(Program.exeDirectory,kReadMeFile);
      }

      private void ConfigCallBacks() 
      {
         this.openToolStripMenuItem.Click += new System.EventHandler(this.openClicked);
         this.exitToolStripMenuItem.Click += new System.EventHandler(this.cancelClicked);
         this.aboutToolStripMenuItem.Click += new System.EventHandler(this.aboutClicked);
         this.button1.Click += new System.EventHandler(this.okClicked);
         this.button2.Click += new System.EventHandler(this.cancelClicked);
      }

      private void openClicked(object sender, System.EventArgs ea)
      {
         this.openFileDialog1.FileName = "";
         this.openFileDialog1.ShowDialog();
         textBox1.Text = openFileDialog1.FileName;
      }

      private void cancelClicked(object sender, System.EventArgs ea)
      {
         System.Windows.Forms.Application.Exit();
      }

      private void aboutClicked(object sender, System.EventArgs ea)
      {
         // 
         // Start notepad process to see results

         System.Diagnostics.Process p = new System.Diagnostics.Process();
         p.StartInfo.UseShellExecute = false;
         p.StartInfo.FileName = "notepad.exe";
         p.StartInfo.Arguments = kReadMeFile;
         p.Start();
      }


      private string[] cfgNames = new string[7] { "dlog", "location", "ospath", "osfile", "version", "alias", "address"};
      private string[] cfgValues = new string[7];
      private System.IO.StreamWriter outputStream;


      private void LoadConfigFile()
      {
         try
         {
            string text = System.IO.File.ReadAllText(kConfigFileName);
            if (!ParseText(text, cfgNames, cfgValues))
               System.Console.WriteLine(@"Config file must be in format 'A=B' (with 'A' as option and 'B' as value) delimited by newlines.");
            
            textBox1.Text = cfgValues[0];
            textBox2.Text = cfgValues[1];
            textBox3.Text = cfgValues[2];
            if (cfgValues[4] == radioButton1.Text)
               radioButton1.Checked = true;
            else
               radioButton2.Checked = true;
         }
         catch (System.IO.IOException)
         {
            System.Console.WriteLine("Couldn't find config file " + kConfigFileName);
         }
      }

      private bool ParseText(string text, string[] varNames, string[] varValues)
      {
         //
         // This function splits up a contiguous string by new lines, then splits
         // each line by '=', and assigns the right-hand-side to the element of 
         // varValues array whose varNames' element is the same as the left-hand-side.
         //  
         string[] lines = text.Split(new char[]{'\n'});

         for (int lineNo=0; lineNo<lines.Length; ++lineNo)
         {
            if (lines[lineNo] == "" || lines[lineNo] == "\r")
               continue;

            string[] var = lines[lineNo].Split(new char[]{'='});
            if (var.Length != 2)
               return false;
            
            for (int i=0; i<varNames.Length; ++i) 
               if (var[0] == varNames[i])
                  varValues[i] = var[1].Split(new char[]{'\r'})[0];                     
         }

         return true;
      }

      private void okClicked(object sender, System.EventArgs ea)
      {

         Visible = false;  // make GUI invisible now

         // Create file name for output file (include path according to location of dlog)
         //
         try
         {
            string dlogDirectory = System.IO.Path.GetDirectoryName(textBox1.Text);
            string dlogName = System.IO.Path.GetFileNameWithoutExtension(textBox1.Text);
            kOutputFile = System.IO.Path.Combine(dlogDirectory, dlogName + "_STACKTRACE.txt");
         }
         catch (System.ArgumentException)
         {
            kOutputFile = System.IO.Path.Combine(Program.exeDirectory, "STACKTRACE_OUT.txt");
         }

         // Get selections to be used for StackTrace Tool
         //
         string dataLogFile = textBox1.Text; // get what's in textbox no matter what
         string buildPath = textBox2.Text;   // get what's in textbox no matter what
         string OSPaths = textBox3.Text;     // get what's in textbox no matter what
         string OSFile = cfgValues[3];       // get what's in cfg file
         string version = radioButton1.Checked ? radioButton1.Text : radioButton2.Text; // get according to checkbox
         string alias = cfgValues[5];        // get what's in cfg file
         string address = cfgValues[6];      // get what's in cfg file

         try
         {
            // 
            // Set up child process StackTrace (a command line tool) redirecting stdout to stream
            // with event that writes to file.

            outputStream = new System.IO.StreamWriter(kOutputFile);
            System.Diagnostics.Process p = new System.Diagnostics.Process();
            p.StartInfo.UseShellExecute = false;
            p.StartInfo.RedirectStandardOutput = true;
            p.StartInfo.FileName = kStackTraceExe;

            string cmdLineArgs;
            cmdLineArgs = (buildPath == "" || buildPath == null) ? "" : " -location " + buildPath;
            cmdLineArgs += (OSPaths == "" || OSPaths == null) ? "" : " -ospath " + OSPaths;
            cmdLineArgs += (OSFile == "" || OSFile == null) ? "" : " -osfile " + OSFile;
            cmdLineArgs += (version == "" || version == null) ? "" : " -version " + version;
            cmdLineArgs += (alias == "" || alias == null) ? "" : " -alias " + alias;
            cmdLineArgs += (address == "" || address == null) ? "" : " -address " + address;     
            p.StartInfo.Arguments = cmdLineArgs + " " + dataLogFile;
            System.Console.WriteLine(p.StartInfo.FileName + p.StartInfo.Arguments);

            p.OutputDataReceived += new System.Diagnostics.DataReceivedEventHandler(OnDataReceived);
            p.Start();
            p.BeginOutputReadLine();
            p.WaitForExit();  
         }
         finally
         {
            outputStream.Flush();
            outputStream.Close();
         }

         System.Console.WriteLine("Press any key to continue ... ");
         System.Console.ReadKey(true);

         // 
         // Start notepad process to see results

         System.Diagnostics.Process q = new System.Diagnostics.Process();
         q.StartInfo.UseShellExecute = false;
         q.StartInfo.FileName = "notepad.exe";
         q.StartInfo.Arguments = kOutputFile;
         q.Start();

         System.Windows.Forms.Application.Exit();
      }

      private void OnDataReceived(object sender, System.Diagnostics.DataReceivedEventArgs ea)
      {
         if (ea.Data != null && outputStream != null)
            outputStream.WriteLine(ea.Data);
      }


   } // end class Form1
} // end namespace StackTraceUI
