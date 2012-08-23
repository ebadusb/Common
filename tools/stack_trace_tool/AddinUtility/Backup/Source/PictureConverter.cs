/*****************************************************************************\
**
** PictureConverter
**
** Implementation of picture converter.
**
\*****************************************************************************/

namespace AddinUtility {

/* Usings ********************************************************************/

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

/* Classes *******************************************************************/

public class PictureConverter : System.Windows.Forms.AxHost {
	private PictureConverter() : base(String.Empty)
	{
	}

	static public stdole.IPictureDisp ImageToPictureDisp(System.Drawing.Image image)
	{
		return (stdole.IPictureDisp)GetIPictureDispFromPicture(image);
	}

	static public stdole.IPictureDisp IconToPictureDisp(System.Drawing.Icon icon)
	{
		return ImageToPictureDisp(icon.ToBitmap());
	}

	static public System.Drawing.Image PictureDispToImage(stdole.IPictureDisp picture)
	{
		return GetPictureFromIPicture(picture);
	}
}
}
