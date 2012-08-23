/*****************************************************************************\
**
** EventLog
**
** Implementation of an event log utility.
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

/* Classes *******************************************************************/

public class EventLog {
	public EventLog(String file)
	{
		mFile = file;
	}

	public void Open()
	{
		try {
			mStream = new StreamWriter(mFile, false);
			this.Info("Oppening log file: " + Path.GetFileName(mFile));
		}
		catch {
		}
	}

	public void Close()
	{
		try {
			this.Info("Closing log file: " + Path.GetFileName(mFile));
			mStream.Close();
		}
		catch {
		}
	}
	
	public void Info(String text)
	{
		StackFrame frame = new StackFrame(1, true);

		if (mStream != null) {
			mStream.WriteLine("Info \t" 
				+ frame.GetMethod() + "\t"
				+ text);
			mStream.Flush();
		}
	}

	public void Warning(String text)
	{
		StackFrame frame = new StackFrame(1, true);

		if (mStream != null) {
			mStream.WriteLine("Warning \t"
				+ frame.GetMethod() + "\t"
				+ text);
			mStream.Flush();
		}
	}

	public void Error(String text)
	{
		StackFrame frame = new StackFrame(1, true);

		if (mStream != null) {
			mStream.WriteLine("Error \t"
				+ frame.GetMethod() + "\t"
				+ text);
			mStream.Flush();
		}
	}

	private String mFile;
	private StreamWriter mStream;
}

public class EventLogFactory {
	public static EventLog CreateEventLog(String folder, String file, int limit)
	{
		String name = Path.GetFileNameWithoutExtension(file) + "*" + Path.GetExtension(file);
		DirectoryInfo dirInfo = new DirectoryInfo(folder);
		FileInfo[] fileInfo = dirInfo.GetFiles(name);
		IEnumerable<FileInfo> files = fileInfo.OrderByDescending(info => info.CreationTime);
		int count = 0;
			
		foreach (FileInfo info in files) {
			if (count >= limit - 1) {
				File.Delete(info.FullName);
			}
			++count;
		}

		return new EventLog(folder + "\\"
			+ Path.GetFileNameWithoutExtension(file) + "_"
			+ DateTime.Now.ToString("yyyy'_'MM'_'dd'_'hh'_'mm'_'ss")
			+ Path.GetExtension(file));
	}
}

}
