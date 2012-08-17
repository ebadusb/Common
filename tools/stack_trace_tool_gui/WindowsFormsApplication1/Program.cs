//using System;
//using System.Collections.Generic;
//using System.Linq;
//using System.Windows.Forms;

namespace StackTraceUI
{
   static class Program
   {
      /// <summary>
      /// The main entry point for the application.
      /// </summary>
      [System.STAThread]
      static void Main(string[] args)
      {
         if (args.Length == 0)
            dataLogFile = "";
         else
            dataLogFile = args[0];

//         string appFileName = System.Environment.GetCommandLineArgs()[0];
         exeDirectory = System.Windows.Forms.Application.StartupPath;

         System.Windows.Forms.Application.EnableVisualStyles();
         System.Windows.Forms.Application.SetCompatibleTextRenderingDefault(false);
         System.Windows.Forms.Application.Run(new Form1());
      }

      public static string dataLogFile;
      public static string exeDirectory;
   }
}
