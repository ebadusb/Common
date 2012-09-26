using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace StackTraceUI
{
    public partial class About : Form
    {
        public About()
        {
            InitializeComponent();

            System.IO.StreamReader inputStream = null;
            try
            {
               inputStream = new System.IO.StreamReader(System.IO.Path.Combine(Program.exeDir, "README.txt")); 
               lblAbout.Text = inputStream.ReadToEnd();
            }
            finally
            {
               inputStream.Dispose();
               inputStream.Close();
            }
        } // end constructor
     
    } // end class
}
