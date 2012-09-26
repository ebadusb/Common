using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows.Forms;

namespace StackTraceUI
{
    static class Program
    {
        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main(string[] args)
        {
            if (args.Length > 0)
               dataLogFile = args[0];

            exeDir = Application.StartupPath;

            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            Application.Run(new StackTraceUI());
        }

        public static string dataLogFile;
        public static string exeDir;
    }
}
