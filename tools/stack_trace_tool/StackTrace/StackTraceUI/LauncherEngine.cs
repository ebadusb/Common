﻿/*****************************************************************************\
**
** LauncherEngine
**
** Implementation of the launcher engine.
**
\*****************************************************************************/

namespace DatalogLauncher {

/* Usings ********************************************************************/

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;
using System.Diagnostics;
using System.Windows.Forms;
using System.Threading;
using System.Text.RegularExpressions;
using System.Xml;
using System.Xml.Serialization;
using AddinUtility;

//using DatalogLauncher;

/* Classes *******************************************************************/

public class LauncherEngine {

   public static readonly String kStartupPath = System.Windows.Forms.Application.StartupPath;
   public static readonly String kDatalogApps = "DatalogApps.config";

	#region Constructor
	public LauncherEngine()
	{
		mSettings = new LauncherSettings();
		mApplication = "";

      if(!File.Exists(DatalogPathname.defaultConfig)) 
         DatalogPathname.CreateDefaultConfig(DatalogPathname.defaultConfig);

		mDirectories = DatalogPathname.GetFolderSettings(DatalogPathname.defaultConfig);
	}
	#endregion

	#region Settings
	public void LoadSettings()
	{
		if (System.IO.File.Exists(Path.Combine(kStartupPath, kDatalogApps))) {
			XmlTextReader reader = null;
			XmlSerializer serializer = null;

			try {
				serializer = new XmlSerializer(mSettings.GetType());
				reader = new XmlTextReader(Path.Combine(kStartupPath, kDatalogApps));
				mSettings = (LauncherSettings)serializer.Deserialize(reader);
			}
			catch {
				System.Windows.Forms.MessageBox.Show(
						"Could not read " + Path.Combine(kStartupPath, kDatalogApps),
						"Error",
						System.Windows.Forms.MessageBoxButtons.OK,
						System.Windows.Forms.MessageBoxIcon.Error);
			}

			if (reader != null) {
				reader.Close();
			}
		}
	}

	public void StoreSettings()
	{
		XmlTextWriter writer = null;
		XmlSerializer serializer = null;

		try {
			serializer = new XmlSerializer(mSettings.GetType());
			writer = new XmlTextWriter(Path.Combine(kStartupPath, kDatalogApps), System.Text.Encoding.Default);
			serializer.Serialize(writer, mSettings);
		}
		catch {
			System.Windows.Forms.MessageBox.Show(
					"Could not create " + Path.Combine(kStartupPath, kDatalogApps),
					"Error",
					System.Windows.Forms.MessageBoxButtons.OK,
					System.Windows.Forms.MessageBoxIcon.Error);
		}

		if (writer != null) {
			writer.Close();
		}
	}
	#endregion

	#region Properties
	public String Application
	{
		get { return mApplication; }
		set { mApplication = value; }
	}

	public LauncherSettings Settings
	{
		get { return mSettings; }
		set { mSettings = value; }
	}
	#endregion

	#region Functions
	public void ProcessFromArgument(String text)
	{
		this.ProcessDatalog(text);
	}

	public void ProcessFromClipboard()
	{
		this.ProcessDatalog(LauncherUtility.GetTextFromClipboard());
	}

	private void ProcessDatalog(String text)
	{
		String logname = DatalogPathname.ExtractDatalogName(text);
		String filename = DatalogPathname.GetCompletePathname(logname, mDirectories);

		if (System.IO.File.Exists(filename)) {
			if (String.IsNullOrEmpty(mApplication)) {
				Process.Start(filename);
			}
			else {
				Process.Start(mApplication, filename);
			}
		}
		else {
			System.Windows.Forms.MessageBox.Show(
				"Could not locate log.\n" + logname,
				"Error",
				System.Windows.Forms.MessageBoxButtons.OK,
				System.Windows.Forms.MessageBoxIcon.Error);
		}
	}
	#endregion
	
	#region Members
	private LauncherSettings mSettings;
	private String mApplication;
	private List<String> mDirectories;
	#endregion
}
}
