/*****************************************************************************\
**
** SerializeUtility
**
** Implementation of serialize utility.
**
\*****************************************************************************/

namespace AddinUtility {

/* Usings ********************************************************************/
	
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Xml;
using System.Xml.Serialization;

/* Classes *******************************************************************/

public class SerializeUtility {

	public static void ImportItem<ITEM>(String file, out ITEM item) where ITEM : new()
	{
		XmlSerializer serializer = new XmlSerializer(typeof(ITEM));
		XmlTextReader reader = null;

		item = new ITEM();

		try {
			reader = new XmlTextReader(file);
			item = (ITEM)serializer.Deserialize(reader);
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

	public static void ExportItem<ITEM>(String file, ITEM item)
	{
		XmlSerializer serializer = new XmlSerializer(item.GetType());
		XmlTextWriter writer = null;

		try {
			writer = new XmlTextWriter(file, System.Text.Encoding.Default);
			serializer.Serialize(writer, item);
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
