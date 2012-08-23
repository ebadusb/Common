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
            lblAbout.Text = "[-location buildpath] [-version (2.02 | 2.20)]\n\t[-osfile filename] [-ospath pathnames]\n[-pathalias old[:new]]\n[-address values]\nfilename";
        }
     
   
    }
}
