using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Diagnostics;
using AddinUtility;

namespace DlogPath
{
    class Program
    {
        static void Main(string[] args)
        {
            if (args.Length < 1)
            {
                System.Console.WriteLine("usage: " + Process.GetCurrentProcess().ProcessName + " dlogname");
                System.Environment.Exit(0);
            }

            String dlogPath = AddinUtility.DatalogPathname.GetCompletePathname(args[0]);
            System.Console.WriteLine(dlogPath);
        }
    }
}
