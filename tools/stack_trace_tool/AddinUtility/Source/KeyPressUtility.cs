/*****************************************************************************\
**
** KeyPressUtility
**
** Implementation of key press utility.
**
\*****************************************************************************/

namespace AddinUtility {

/* Usings ********************************************************************/

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Forms;

/* Classes *******************************************************************/

public class KeyPressUtility {
	static public void LettersOnlyHandler(object sender, KeyPressEventArgs e)
	{
		if (!Char.IsLetter(e.KeyChar)
				&& (e.KeyChar != 127)		// DEL
				&& (e.KeyChar != 008)) {	// BS
			e.Handled = true;
		}
	}
}
}
