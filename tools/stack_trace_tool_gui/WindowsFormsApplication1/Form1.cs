

namespace StackTraceUI
{
   public partial class Form1 : System.Windows.Forms.Form
   {
      public Form1()
      {
         InitializeComponent();
         ConfigCallBacks();
         GetFileNames();
         LoadConfigFile();
         if (Program.dataLogFile != "")
            textBox1.Text = Program.dataLogFile;
      }
   }
}
