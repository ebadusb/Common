/*****************************************************************************\
**
** LauncherSettings
**
** Implementation of the launcher settings.
**
\*****************************************************************************/

namespace DatalogLauncher {

/* Usings ********************************************************************/

using System;
using System.Collections.Generic;
using System.Windows.Forms;
using AddinUtility;

/* Classes *******************************************************************/

public class LauncherSettings {
	#region Constructor
	public LauncherSettings()
	{
		mApplications = new SerializableDictionary<string, string>();
	}
	#endregion

	#region Properties
	public SerializableDictionary<String, String> Applications
	{
		get { return mApplications; }
		set { mApplications = value; }
	}
	#endregion

	#region Members
	private SerializableDictionary<String, String> mApplications;
	#endregion
}
}
