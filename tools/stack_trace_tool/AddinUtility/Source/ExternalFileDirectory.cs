/*****************************************************************************\
**
** ExternalFileDirectory
**
** Utility class for extracting paths of external files.
**
\*****************************************************************************/

namespace AddinUtility {

/* Usings ********************************************************************/

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Win32;

/* Classes *******************************************************************/

public class ExternalFileDirectory {
	public static String GetConfigFolder()
	{
		String value = System.Windows.Forms.Application.LocalUserAppDataPath;

		try {
			RegistryKey myKey = Registry.CurrentUser.OpenSubKey("SOFTWARE\\CaridianBCT\\AnalysisToolSet", false);

			if (myKey != null) {
				value = myKey.GetValue("Config Path").ToString();
			}
		}
		catch {
		}

		return value;
	}

	public static String GetEventFolder()
	{
		String value = System.Windows.Forms.Application.LocalUserAppDataPath;

		try {
			RegistryKey myKey = Registry.CurrentUser.OpenSubKey("SOFTWARE\\CaridianBCT\\AnalysisToolSet", false);

			if (myKey != null) {
				value = myKey.GetValue("Event Path").ToString();
			}
		}
		catch {
		}

		return value;
	}
}
}
