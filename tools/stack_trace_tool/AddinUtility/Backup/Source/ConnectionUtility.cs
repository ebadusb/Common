/*****************************************************************************\
**
** ConnectionUtility
**
** Implementation of connection utility.
**
\*****************************************************************************/

namespace AddinUtility {

/* Usings ********************************************************************/
	
using System;
using System.Linq;
using System.Text;

/* Classes *******************************************************************/

public class ConnectionUtility {
	public static String GetConnectionString(String filename)
	{
		String extension = System.IO.Path.GetExtension(filename);
		String command = "";

		if (extension == ".accdb") {
			command += "Provider=Microsoft.ACE.OLEDB.12.0;";
			command += "Data Source=" + filename + ";";
		}
		else if (extension == ".mdb") {
			command += "Provider=Microsoft.Jet.OLEDB.4.0;";
			command += "Data Source=" + filename + ";";
		}
		else if (extension == ".xlsx") {
			command += "Provider=Microsoft.ACE.OLEDB.12.0;";
			command += "Data Source=" + filename + ";";
			command += "Extended Properties=\"Excel 12.0 Xml;HDR=YES\";";
		}
		else if (extension == ".xlsb") {
			command += "Provider=Microsoft.ACE.OLEDB.12.0;";
			command += "Data Source=" + filename + ";";
			command += "Extended Properties=\"Excel 12.0;HDR=YES\";";
		}
		else if (extension == ".xlsm") {
			command += "Provider=Microsoft.ACE.OLEDB.12.0;";
			command += "Data Source=" + filename + ";";
			command += "Extended Properties=\"Excel 12.0 Macro;HDR=YES\";";
		}
		else {
			command += "Provider=Microsoft.ACE.OLEDB.12.0;";
			command += "Data Source=" + filename + ";";
		}

		return command;
	}
}
}
