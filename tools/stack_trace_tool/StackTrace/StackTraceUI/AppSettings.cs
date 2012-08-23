using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace StackTraceUI
{
    public class AppSettings
    {
        private string buildPath;
        private string tornadoVersion;
        private string osFile;
        private string osPath;
        private string pathAlias;
        private string address;
        
        private string appVersion = "1.0";

        
        public string BuildPath
        {
            get { return buildPath; }
            set { buildPath = value; }
        }
        public string TornadoVersion
        {
            get { return tornadoVersion; }
            set { tornadoVersion = value; }
        }
        
        public string OsFile
        {
            get { return osFile; }
            set { osFile = value; }
        }

        public string OsPath
        {
            get { return osPath; }
            set { osPath = value; }
        }

        public string PathAlias
        {
            get { return pathAlias; }
            set { pathAlias = value; }
        }

        public string Address
        {
            get { return address; }
            set { address = value; }
        }

        
        public string AppVersion
        {
            get { return appVersion; }
            set { appVersion = value; }
        }
    }
}
