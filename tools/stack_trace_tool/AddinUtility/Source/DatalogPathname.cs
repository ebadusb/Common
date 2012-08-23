/*****************************************************************************\
**
** DatalogPathname
**
** Gets the full pathname of the given log name.
**
\*****************************************************************************/

namespace AddinUtility {

/* Usings ********************************************************************/

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;
using System.Diagnostics;
using System.Windows.Forms;
using System.Xml;
using System.Xml.Serialization;
using System.Text.RegularExpressions;

/* Classes *******************************************************************/

public class DatalogPathname {
    
    //Searches only "well known" locations in the quad. *Will break*
    //years from now.
    public static readonly String defaultConfig = "DatalogPaths.config";
    
    public static String GetCompletePathname(String text)
    {        
        if(!System.IO.File.Exists(defaultConfig)) {
            CreateDefaultConfig(defaultConfig);
        }
        //Create Default config and just use it. 
        return GetCompletePathname(text, GetFolderSettings(defaultConfig));
    }

	public static String GetCompletePathname(String text, String configs)
	{
		return GetCompletePathname(text, GetFolderSettings(configs));
	}

	public static String GetCompletePathname(String text, List<String> folders)
	{
        //User knows the actual path.
        if (File.Exists(text))
            return text;

        String logname = ExtractDatalogName(text);

		if (!String.IsNullOrEmpty(logname) 
				&& logname.Length >= 20 
				&& logname.Length <= 32 
				&& folders != null) {
			String path = "";
			String machine = logname.Substring(0, 7);
			String alias = machine[1] + machine.Substring(2, 5).TrimStart(("0").ToCharArray());
			
			foreach (String folder in folders) {
				path = folder + "\\" + machine + "\\dlogs";
				if (System.IO.Directory.Exists(path)) {
					DirectoryInfo dirInfo = new DirectoryInfo(path);
					FileInfo[] fileInfo = dirInfo.GetFiles(logname + "*");

					if (fileInfo.Length > 0) {
						return fileInfo[0].FullName;
					}
				}

				path = folder + "\\" + alias + "\\dlogs";
				if (System.IO.Directory.Exists(path)) {
					DirectoryInfo dirInfo = new DirectoryInfo(path);
					FileInfo[] fileInfo = dirInfo.GetFiles(logname + "*");

					if (fileInfo.Length > 0) {
						return fileInfo[0].FullName;
					}
				}

				if (System.IO.Directory.Exists(folder)) {
					DirectoryInfo dirInfo = new DirectoryInfo(folder);
					FileInfo[] fileInfo = dirInfo.GetFiles(logname + "*");

					if (fileInfo.Length > 0) {
						return fileInfo[0].FullName;
					}
				}
			}
		}

		return "";
	}

	public static String ExtractDatalogName(String text)
	{
		String logname = "";

		if (!String.IsNullOrEmpty(text)) {
			Match match = Regex.Match(text, "\\d?[A-Z]\\d{1,5}_\\d{8}_\\d{3}(_\\d{6})?");

			if (match.Success) {
				logname = match.Value;

				if (Char.IsLetter(logname[0])) {
					logname = '1' + logname;
				}

				while (logname.IndexOf('_') < 7) {
					logname = logname.Insert(2, "0");
				}
			}
		}

		return logname;
	}

	public static List<String> GetFolderSettings(String file)
	{
		List<String> folders = null;

		if (System.IO.File.Exists(file)) {
			XmlTextReader reader = null;
			XmlSerializer serializer = null;

			try {
				serializer = new XmlSerializer(typeof(List<String>));
				reader = new XmlTextReader(file);
				folders = (List<String>)serializer.Deserialize(reader);
			}
			catch {
				System.Windows.Forms.MessageBox.Show(
						"Could not read \"" + file + "\"",
						"Error",
						System.Windows.Forms.MessageBoxButtons.OK,
						System.Windows.Forms.MessageBoxIcon.Error);
			}

			if (reader != null) {
				reader.Close();
			}
		}

		return folders;
	}

	public static void CreateDefaultConfig(String file)
	{
		XmlTextWriter writer = null;
		XmlSerializer serializer = null;
		List<String> folders = new List<String>();

		folders.Add("\\\\bctquad3\\rundata\\");
		folders.Add("\\\\bctquad3\\Atreus\\rundata\\");
		folders.Add("\\\\uspsr200a\\Optia\\rundata\\");
		folders.Add("\\\\uspsr200b\\ces\\rundata\\");

		try {
			serializer = new XmlSerializer(typeof(List<String>));
			writer = new XmlTextWriter(defaultConfig, System.Text.Encoding.Default);
			serializer.Serialize(writer, folders);
		}
		catch {
			System.Windows.Forms.MessageBox.Show(
					"Could not create \"" + file + "\"",
					"Error",
					System.Windows.Forms.MessageBoxButtons.OK,
					System.Windows.Forms.MessageBoxIcon.Error);
		}

		if (writer != null) {
			writer.Close();
		}
	}
}
}
