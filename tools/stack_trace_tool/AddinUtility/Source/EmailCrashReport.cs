namespace AddinUtility {

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Net.Mail;
using Outlook = Microsoft.Office.Interop.Outlook;	

public class EmailCrashReport {

	public void AuthenticatedMailSend(string mailTo, string subject, string body)
	{
		/*
 		MailMessage message = new MailMessage();
			
		message.To.Add(mailTo);		
		message.Subject = subject;
		message.Body = body;
		MailMessage email = new MailMessage("Jonathan.Sylvain@caridianbct.com", mailTo, subject, body);
		SmtpClient client = new SmtpClient("USBSMAIL2K3V.bct.gambro.net");
		
		try {
			client.Send(email);
		}
		catch (Exception e) {
		}
		*/

		try {
			Outlook.Application app = new Outlook.Application();
			Outlook.MailItem msg = (Outlook.MailItem)app.CreateItem(Outlook.OlItemType.olMailItem);

			msg.Recipients.Add(mailTo);
			msg.Subject = subject;
			msg.Body = body;

			/*
			String sSource = "C:\\setupxlg.txt";
			String sDisplayName = "MyFirstAttachment";
			int iPosition = (int)msg.Body.Length + 1;
			int iAttachType = (int)Outlook.OlAttachmentType.olByValue;
			Outlook.Attachment oAttach = msg.Attachments.Add(sSource, iAttachType, iPosition, sDisplayName);
			*/
			
			msg.Display(true);
		}
		catch (Exception e) {
			Console.WriteLine("{0} Exception caught: ", e);
		}
	}
}
}
