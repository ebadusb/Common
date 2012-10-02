#region Using
using System;
using System.Text;
using System.Collections;
using System.IO;
using System.Diagnostics;
using System.Runtime.Serialization.Formatters.Binary;
using System.Data;
using System.Text.RegularExpressions;

#region ICSharpCode
using ICSharpCode.SharpZipLib.BZip2;
using ICSharpCode.SharpZipLib.Zip;
using ICSharpCode.SharpZipLib;
using ICSharpCode.SharpZipLib.Zip.Compression;
using ICSharpCode.SharpZipLib.Zip.Compression.Streams;
using ICSharpCode.SharpZipLib.GZip;
#endregion

#endregion


namespace Log_Grep
{
	#region Log Time Class
	/// <summary>
	/// LogTime
	/// This is a simple container class to hold the time variables
	/// </summary>
	class LogTime
	{
		// Variables
		public int Day;
		public int Month;
		public int Year;
		public int Hour;
		public int Minute;
		public int Second;
	}
	#endregion
	/// <summary>
	/// LogDefinition.
	/// This is a utility class that takes care of the IO to the log file. 
	/// There is a routine that opens the file, and there is a routine that
	/// will read the file untill it gets a string message. If you reach the end 
	/// of te file before you reach a stream message it will return an empty
	/// string
	/// </summary>
	class LogDefinition
	{
		#region Variables
		// Filestream to the log file

		//GZipInputStream LogFile;
		//Inflater InflaterNoHeader; 	
		//InflaterInputStream LogFile; 
		//ZlibDecompressionStream LogFile;
		//DecypherStream DecyStream;
		GZipInputStream GzipLogFile;
		byte MagicByte=0xa5;		
		MemoryStream LogFile;
		// Console variables

		
		

		// these variables are defined in the Start Of Log File Record
		int SizeOfChar, SizeOfInt,SizeOfLong,SizeOfFloat,SizeOfDouble,SizeOfDataLogTaskID;		
		int SizeOfDataLogNodeID;

		// these variables store the safest allowed read into a record. This is taking into 
		//	account the fact that the first 2 bytes have already been read.
		int SizeOfCreateTaskRecord;
		int SizeOfTaskDeletedRecord;
		int SizeOfStartOfNetworkConnectionRecord;
		int SizeOfTimeStampRecord;
		int SizeOfEndOfLogFileRecord;
		int SizeOfEndOfNetworkOutputRecord;
		int SizeOfLogLevelRecord;
		int SizeOfPrintOutputRecord;
		int SizeOfPeriodicLogOutputRecord;
		int SizeOfStreamOutputRecord;
		int SizeOfPeriodicSetRecord;
		int SizeOfPeriodicLogItemRecord;
		int SizeOfBinaryRecord;
		LogTime ValueOfTime;
		string NodeIDName="";
		byte MajorVersion;
		byte MinorVersion;

		// the return string for the read function
		StringBuilder StreamString = new StringBuilder(1048576);

		// trying to save speed by having only 1 buffer.
		byte[] data=new byte[1048576];
		byte[] dataSave=new byte[1048576];
		#endregion
		// member functions

		// this function does simple computation to speedup the ReadMessage function
		private void Compute()
		{
			SizeOfCreateTaskRecord=8+SizeOfDataLogTaskID+2+SizeOfDataLogNodeID+2;

			SizeOfTaskDeletedRecord=10+SizeOfDataLogTaskID+SizeOfDataLogNodeID;

			SizeOfStartOfNetworkConnectionRecord=SizeOfDataLogNodeID+1+1+2+1+1+1+2;

			SizeOfTimeStampRecord=8;

			SizeOfEndOfLogFileRecord=8;

			SizeOfEndOfNetworkOutputRecord=8+SizeOfDataLogNodeID;

			SizeOfLogLevelRecord=8+2+SizeOfDataLogNodeID+2;

			SizeOfPrintOutputRecord=8+2+SizeOfDataLogTaskID+SizeOfDataLogNodeID+2+2+2;

			SizeOfStreamOutputRecord=8+2+SizeOfDataLogTaskID+SizeOfDataLogNodeID+2+2;

			SizeOfPeriodicLogOutputRecord=8+2+SizeOfDataLogNodeID+2;

			SizeOfPeriodicSetRecord=8+2+SizeOfDataLogNodeID+2;

			SizeOfPeriodicLogItemRecord=8+2+SizeOfDataLogNodeID+2+2+2;

			SizeOfBinaryRecord=4;

			// testing
		}
	
		// this function Creates the Gzip file stream from the base stream and sets up the
		// internal members of the class. At the end of this function the program should have read the 
		// first record in the file.
		public void InitializeLog(FileStream Log)
		{
			#region variables
			bool done= false;
			MemoryStream DecodedFileStream;
			
			long i=0;
			char[] tempCharArray= new char[1];
			byte[] tempByteArray= new byte[1];
			int tempInt;
			bool ReRead=false;
			long GzipPosition=0;
			
			int tempInt1; // this is a small vatiable to split up calculations;
			#endregion
			
			#region Convert the file to a memory stream
			#region DEBUG
			Debug.WriteLine("transfering and decoding the file into memory");
			#endregion
			i=Convert.ToInt16(Log.Position);
			DecodedFileStream= new MemoryStream();
			while(i<Log.Length)
			{
				if ((Log.Length-i)>data.Length)
				{
					tempInt1=Log.Read(data,0,data.Length);
					DecodedFileStream.Write(data,0,tempInt1);
					i=i+tempInt1;
				}
				else
				{
					tempInt1=Convert.ToInt32(Log.Length-i);
					tempInt1=Log.Read(data,0,tempInt1);
					DecodedFileStream.Write(data,0,tempInt1);
					i=i+tempInt1;
				}
			}
			
			Debug.WriteLine("read " + DecodedFileStream.Position + " bytes from" + Log.Name);
			Debug.WriteLine("File Length Was : " + Log.Position);
			if(Log.Position==0)
			{
				throw new IOException("EOF");
			}
			DecodedFileStream.Seek(0, SeekOrigin.Begin);
			#region DEBUG
			Debug.WriteLine("finished loading into memory, file closed");
			#endregion
			#endregion

			#region Plain Text Read
			// first read the plain text w/ normal file I/O
			done=false;
			while(!done)
			{
				DecodedFileStream.Read(data,0,1);
				Debug.Write(DecodedFileStream.Position + ":"+Convert.ToChar(data[0]));
				if (DecodedFileStream.Position>=Log.Position)
				{
					throw new IOException("Not a Dlog file");
				}
				if(data[0]==0x1a)
				{
					DecodedFileStream.Read(data,0,1);
					if(data[0]==0x04)
					{
						DecodedFileStream.Read(data,0,1);
						if(data[0]==0x00)
						{
							done=true;
						}
					}
				}
			}
			#endregion

			
			#region Read the log settings
			tempInt1=DecodedFileStream.Read(data,0,18);
			#region DEBUG
			 
			{
				Debug.WriteLine("first read successfull :");
				Debug.WriteLine("  return Value of the read : "+tempInt1);
				Debug.Write("  Data from the first read : ");
				for (i=0;i<17;i++)
				{
					Debug.Write(string.Format("{0,1:x} ",data[i]));
				}
				Debug.WriteLine(string.Format("{0,1:x}",data[17]));
			}
			#endregion

			
			// load the log settings
			SizeOfChar=BitConverter.ToInt16(data,6);
			SizeOfInt=BitConverter.ToInt16(data,8);
			SizeOfLong=BitConverter.ToInt16(data,10);
			SizeOfFloat=BitConverter.ToInt16(data,12);
			SizeOfDouble=BitConverter.ToInt16(data,14);
			SizeOfDataLogTaskID=BitConverter.ToInt16(data,16);
			tempInt1=SizeOfDataLogTaskID;
			DecodedFileStream.Read(data,0,tempInt1);
			DecodedFileStream.Read(data,0,6);
			MajorVersion=data[3];
			MinorVersion=data[2];
			if((MajorVersion != 3)||(MinorVersion!=1))
			{
				throw new IOException("EOF");
			}
			SizeOfDataLogNodeID=BitConverter.ToInt16(data,0);
			tempInt1=BitConverter.ToInt16(data,4);
			// read platform name
			DecodedFileStream.Read(data,0,tempInt1);
			// read Node ID
			DecodedFileStream.Read(data,0,SizeOfDataLogNodeID);
			NodeIDName="";
			for (i=0;i<SizeOfDataLogNodeID;i++)
			{
				NodeIDName=NodeIDName+data[i].ToString("X")+" ";
			}

			// read the time stamp
			DecodedFileStream.Read(data,0,7);
			ValueOfTime = new LogTime();
			ValueOfTime.Day=Convert.ToInt16(data[0]);
			ValueOfTime.Month=Convert.ToInt16(data[1]);
			ValueOfTime.Year=BitConverter.ToInt16(data,2);
			ValueOfTime.Hour=Convert.ToInt16(data[4]);
			ValueOfTime.Minute=Convert.ToInt16(data[5]);
			ValueOfTime.Second=Convert.ToInt16(data[6]);
			// find the length of the node name
			DecodedFileStream.Read(data,0,2);
			tempInt1=BitConverter.ToInt16(data,0);
			// read the node name
			DecodedFileStream.Read(data,0,tempInt1);
			
			#region DEBUG
			{
				Debug.IndentLevel = 1;
				Debug.WriteLine("Log File Header Information :");
				Debug.IndentLevel = 1;
				Debug.WriteLine("Size of Char = " + SizeOfChar);
				Debug.WriteLine("Size of Int = " + SizeOfInt);
				Debug.WriteLine("Size of Long = " + SizeOfLong);
				Debug.WriteLine("Size of Float = " + SizeOfFloat);
				Debug.WriteLine("Size of Double = " + SizeOfDouble);
				Debug.WriteLine("Size of Data Log Task ID = " + SizeOfDataLogTaskID);
				Debug.WriteLine("Size of Data Log Node ID = " + SizeOfDataLogNodeID);
				Debug.WriteLine("Size of Last Read = " + tempInt1);
				Debug.WriteLine("Time of Record:");
				Debug.IndentLevel = 2;
				Debug.WriteLine("Day = "+ValueOfTime.Day);
				Debug.WriteLine("Month = "+ValueOfTime.Month);
				Debug.WriteLine("Year = "+ValueOfTime.Year);
				Debug.WriteLine("Hour = "+ValueOfTime.Hour);
				Debug.WriteLine("Minute = "+ValueOfTime.Minute);
				Debug.WriteLine("Second = "+ValueOfTime.Second);
				Debug.IndentLevel = 0;
				Debug.Write("  Node name = ");
				for (i=0;i<tempInt1;i++)
				{
					Debug.Write(Convert.ToChar(data[i]));
				}
				Debug.WriteLine("");
				Debug.WriteLine("  Network Node ID = "+ NodeIDName);
				Debug.WriteLine("End of Log File Header Information");
			}
			
			#endregion
			Compute();
			#endregion

			

			#region Open the Gzip file and loading it into memory
			try
			{
				
				#region DEBUG
				
			{
				Debug.WriteLine("Opening the Zlib stream");
			}
				#endregion
				GzipPosition=DecodedFileStream.Position;
				GzipLogFile = new GZipInputStream(DecodedFileStream);

				#region DEBUG		
				Debug.WriteLine("LogFile is now a Memory stream :");
				Debug.WriteLine("  LogFile.CanRead = "+GzipLogFile.CanRead);
				Debug.WriteLine("  LogFile.CanWrite = "+GzipLogFile.CanWrite);
				Debug.WriteLine("  LogFile.CanSeek = "+GzipLogFile.CanSeek);
				Debug.WriteLine("  LogFile.Position = "+GzipLogFile.Position);
				#endregion
				LogFile = new MemoryStream();
				tempInt=1;
				while ( tempInt!=0)
				{
					tempInt = GzipLogFile.Read(data,0,128);
					for(i=0; i<tempInt ; i++)
					{
						data[i] = Convert.ToByte(data[i]^MagicByte);
					}
					LogFile.Write(data,0,tempInt);
					
				}
				
			}
				#region catches
			catch (ApplicationException ApEx)
			{
				if (ApEx.Message == "Deflated stream ends early.")
				{
					ReRead=true;
				}
				else
				{
					Debug.WriteLine("LogDefinition: InitializeLog: no catch for ApplicationException: " + ApEx.Message);
					throw new IOException(ApEx.Message) ;
				}
			}
			catch (	FormatException FoEx)
			{
				if ((FoEx.Message == "broken uncompressed block")||(FoEx.Message == "Illegal rep dist code"))
				{
					ReRead=true;
				}
				else
				{
					Debug.WriteLine("LogDefinition: InitializeLog: no catch for ApplicationException: " + FoEx.Message);
					throw new IOException(FoEx.Message) ;
				}

			}
			catch (OutOfMemoryException OOMEx)
			{
				Debug.WriteLine("OutOfMemoryException OOMEx.Message:"+OOMEx.Message);
				ReRead=true;
			}
			catch(Exception Ex)
			{
				Debug.WriteLine("LogDefinition: InitializeLog: no catch for Exception: " + Ex.GetType());
				throw new IOException(Ex.Message) ;
			}
			#endregion
			LogFile.Seek(0, SeekOrigin.Begin);
			Log.Close();
			#endregion

		}

		

		
		// this function will read the file untill it finds a stream record. this record is 
		// passed back as the string. 
		public string ReadMessage()
		{
			
			
			#region variables
			#region tempVars
			int tempInt1, tempInt2, i, tempInt3, index;// these are temporary variables to clean 
			char tempChar;						// internal computation.
			
			bool tempBool;
			
			#endregion
			int ByteCount, TotalByteCount,type;		// these variables are used to navigate stream records.

			int LogType = 0;					// 2 byte value that specifies record type being read.
			long RecordSecond, RecordNanosecond, RecordLevelID, RecordTaskID, RecordNodeID, RecordLine;
			string RecordFile;
			bool done=false;					// this is a controll variable.
			StreamString.Remove(0,StreamString.Length);
			#endregion
			
			// rebuild and used as a return value.]
			#region DEBUG
			 Debug.WriteLine("Entering the main loop in ReadMessages");
			#endregion
			while ( !done )
			{
				if ( LogFile.Read(data,0,2) <= 0 ) break;
				LogType=BitConverter.ToUInt16(data,0);
				#region DEBUG
				
				Debug.WriteLine("");
				Debug.Write("Log Type HEX = ");
				Debug.Write(string.Format("{0,1:x} ",data[0]));
				Debug.WriteLine(string.Format("{0,1:x}",data[1]));
				Debug.WriteLine("Log Type = " + LogType);
		
				#endregion
				switch ( LogType )
				{
					case 0x5501: // Log level record
						#region Log Level Record
						 Debug.WriteLine("Log Level Record");
						LogFile.Read(data,0,SizeOfLogLevelRecord);
						
						Debug.WriteLine("Hex Dump :");
						
						Debug.WriteLine("");
						
						tempInt1=BitConverter.ToUInt16(data,(SizeOfLogLevelRecord-2));
						Debug.WriteLine(tempInt1);
						LogFile.Read(data,0,tempInt1);
						break;
						#endregion
					case 0x5502: // Print record
						//old
						#region Print record
						/*
						 Debug.WriteLine("Print Record");
						LogFile.Read(data,0,SizeOfPrintOutputRecord);
						#region DEBUG
						
						{
							Debug.WriteLine("Size of Print Output Record = "+SizeOfPrintOutputRecord);
							Debug.WriteLine("Hex Dump :");
							for (i=0;i<SizeOfPrintOutputRecord;i++)
							{
								Debug.Write(string.Format("{0,1:x} ",data[i]));
							}
							Debug.WriteLine("");
						}
						#endregion
						tempInt1=BitConverter.ToUInt16(data,(SizeOfPrintOutputRecord-6));
						tempInt2=BitConverter.ToUInt16(data,(SizeOfPrintOutputRecord-4));
						LogFile.Read(dataSave,0,tempInt1);
						#region DEBUG
						
						{
							Debug.WriteLine("Size of format = "+tempInt1);
							Debug.WriteLine("Hex Dump :");
							for (i=0;i<tempInt1;i++)
							{
								Debug.Write(string.Format("{0,1:x} ",dataSave[i]));
							}
							Debug.WriteLine("");
							for (i=0;i<tempInt1;i++)
							{
								Debug.Write(Convert.ToChar(dataSave[i]));
							}
							Debug.WriteLine("");
						}
						#endregion
						LogFile.Read(data,0,tempInt2);
						#region DEBUG
						
						{
							Debug.WriteLine("Size of file = "+tempInt2);
							Debug.WriteLine("Hex Dump :");
							for (i=0;i<tempInt2;i++)
							{
								Debug.Write(string.Format("{0,1:x} ",data[i]));
							}
							Debug.WriteLine("");
							Debug.WriteLine("Text Version :");
							for (i=0;i<tempInt2;i++)
							{
								Debug.Write(Convert.ToChar(data[i]));
							}
							Debug.WriteLine("");
						}
						#endregion
						for (i=0;i<tempInt2;i++)
						{
							tempChar = Convert.ToChar(data[i]);
							StreamString.Append(tempChar);
						}
						StreamString.Append("(");
						StreamString.Append(BitConverter.ToUInt16(data,(SizeOfPrintOutputRecord-2)));
						StreamString.Append("):");
						index=0;
						while(index<tempInt1)
						{
							#region prinf printing
							if(Convert.ToChar(dataSave[index])=='%')
							{
								index++;
								 Debug.Write("  %");
								switch(Convert.ToChar(dataSave[index++]))
								{
									case '.':
									{
										// TODO Clean up this hack
										 Debug.Write(".");
										if ((Convert.ToChar(dataSave[index])=='0')||(Convert.ToChar(dataSave[index])=='1')||(Convert.ToChar(dataSave[index])=='2')
										  ||(Convert.ToChar(dataSave[index])=='3')||(Convert.ToChar(dataSave[index])=='4')||(Convert.ToChar(dataSave[index])=='5')
										  ||(Convert.ToChar(dataSave[index])=='6')||(Convert.ToChar(dataSave[index])=='7')||(Convert.ToChar(dataSave[index])=='8')
										  ||(Convert.ToChar(dataSave[index])=='9'))
										{
											Debug.WriteLine(Convert.ToChar(dataSave[index]));
											dataSave[index]=0x25;
										}
										break;
									}
									case 'c':
									{
										Debug.WriteLine("c");
										LogFile.Read(data,0,SizeOfChar);
										break;
									}
									case 's':
									{
										Debug.WriteLine("s");
										LogFile.Read(data,0,2);
										tempInt3=BitConverter.ToInt16(data,0);
										Debug.WriteLine("    Size of string = "+tempInt3);
										LogFile.Read(data,0,tempInt3);
										
										{
											Debug.Write("    Message of string = ");
											for(i=0;i<tempInt3;i++)
											{
												Debug.Write(Convert.ToChar(data[i]));
											}
											Debug.WriteLine("");
										}
										for(i=0;i<tempInt3;i++)
										{
											tempChar = Convert.ToChar(data[i]);
											StreamString.Append(tempChar);
										}
										break;
									}
									case 'd':
									{
										Debug.WriteLine("d");
										LogFile.Read(data,0,SizeOfInt);
										StreamString.Append(BitConverter.ToInt16(data,0));
										break;
									}
									case 'X':
									{
										Debug.WriteLine("X");
										LogFile.Read(data,0,SizeOfInt);
										StreamString.Append(data[0].ToString("X"));
										break;
									}
									case 'x':
									{
										Debug.WriteLine("x");
										LogFile.Read(data,0,SizeOfInt);
										StreamString.Append(data[0].ToString("x"));
										break;
									}
									case 'u':
									{
										Debug.WriteLine("u");
										LogFile.Read(data,0,SizeOfInt);
										StreamString.Append(BitConverter.ToUInt16(data,0));
										break;
									}
									case 'f':
									{
										Debug.WriteLine("f");
										LogFile.Read(data,0,SizeOfFloat);
										StreamString.Append(BitConverter.ToSingle(data,0));
										break;
									}
									case 'g':
									{
										Debug.WriteLine("g");
										LogFile.Read(data,0,SizeOfFloat);
										StreamString.Append(BitConverter.ToSingle(data,0));
										break;
									}
									case 'l':
									{
										Debug.Write("l");
										switch(Convert.ToChar(dataSave[index++]))
										{
											case 'd':
											{
												Debug.WriteLine("d");
												LogFile.Read(data,0,SizeOfLong);
												StreamString.Append(BitConverter.ToInt32(data,0));
												break;
											}
											case 'X':
											{
												Debug.WriteLine("x");
												LogFile.Read(data,0,SizeOfLong);
												StreamString.Append(data[0].ToString("X"));
												StreamString.Append(data[1].ToString("X"));
												StreamString.Append(data[2].ToString("X"));
												StreamString.Append(data[3].ToString("X"));
												break;
											}
											case 'x':
											{
												Debug.WriteLine("x");
												LogFile.Read(data,0,SizeOfLong);
												StreamString.Append(data[0].ToString("x"));
												StreamString.Append(data[1].ToString("x"));
												StreamString.Append(data[2].ToString("x"));
												StreamString.Append(data[3].ToString("x"));
												break;
											}
											case 'u':
											{
												Debug.WriteLine("u");
												LogFile.Read(data,0,SizeOfLong);
												StreamString.Append(BitConverter.ToUInt32(data,0));
												break;
											}
											case 'f':
											{
												Debug.WriteLine("f");
												LogFile.Read(data,0,SizeOfDouble);
												StreamString.Append(BitConverter.ToDouble(data,0));
												break;
											}
											case 'g':
											{
												Debug.WriteLine("g");
												LogFile.Read(data,0,SizeOfDouble);
												StreamString.Append(BitConverter.ToDouble(data,0));
												break;
											}
										}
										break;
									}
									default:
									{
										break;
									}
								}
							}
								#endregion
							#region sting printing 
							else
							{
								tempChar = Convert.ToChar(dataSave[index++]);
								StreamString.Append(tempChar);
							}
							#endregion
						}
						*/
						#endregion
						
						
						//new
						#region parse
						//Debug.WriteLine("Print Record");
						LogFile.Read(data,0,SizeOfPrintOutputRecord);
						#region DEBUG
						//Debug.WriteLine("Size of Print Output Record = "+SizeOfPrintOutputRecord);
						//Debug.WriteLine("Hex Dump :");
						//Debug.WriteLine(BitConverter.ToString(data,0,SizeOfPrintOutputRecord);
						#endregion
						RecordSecond=BitConverter.ToInt32(data,0);
						RecordNanosecond=BitConverter.ToInt32(data,4);
						RecordLevelID=BitConverter.ToInt16(data,8);
						StreamString.Remove(0,StreamString.Length);
					switch(SizeOfDataLogTaskID)
					{
						case 0:
							RecordTaskID=0;
							break;
						case 2:
							RecordTaskID=BitConverter.ToInt16(data,10);
							break;
						case 4:
							RecordTaskID=BitConverter.ToInt32(data,10);
							break;
						case 8:
							RecordTaskID=BitConverter.ToInt64(data,10);
							break;
						default:
							throw new Exception("Bad Node ID");
					}
					switch(SizeOfDataLogNodeID)
					{
						case 0:
							RecordNodeID=0;
							break;
						case 2:
							RecordNodeID=BitConverter.ToInt16(data,(10+SizeOfDataLogTaskID));
							break;
						case 4:
							RecordNodeID=BitConverter.ToInt32(data,(10+SizeOfDataLogTaskID));
							break;
						case 8:
							RecordNodeID=BitConverter.ToInt64(data,(10+SizeOfDataLogTaskID));
							break;
						default:
							throw new Exception("Bad Node ID");
					}
						tempInt1=BitConverter.ToUInt16(data,(SizeOfPrintOutputRecord-6));
						tempInt2=BitConverter.ToUInt16(data,(SizeOfPrintOutputRecord-4));
						LogFile.Read(dataSave,0,tempInt1);
						#region DEBUG
						//Debug.WriteLine("Size of format = "+tempInt1);
						//Debug.WriteLine("Hex Dump :");
						//Debug.WriteLine(BitConverter.ToString(dataSave,0,tempInt1));
						//Debug.WriteLine("Text Version :");
						//Debug.WriteLine(Encoding.ASCII.GetString(dataSave,0,tempInt1));
						#endregion
						LogFile.Read(data,0,tempInt2);
						#region DEBUG
						//Debug.WriteLine("Size of file = "+tempInt2);
						//Debug.WriteLine("Hex Dump :");
						//Debug.WriteLine(BitConverter.ToString(data,0,tempInt2));
						//Debug.WriteLine("Text Version :");
						//Debug.WriteLine(Encoding.ASCII.GetString(data,0,tempInt2));
						#endregion
						StreamString.Append(Encoding.ASCII.GetString(data,0,tempInt2));
						RecordFile=StreamString.ToString();
						RecordLine=BitConverter.ToUInt16(data,(SizeOfPrintOutputRecord-2));
						#region DEBUG
						//Debug.WriteLine("Line number : "+RecordLine);
						#endregion
						StreamString.Remove(0,StreamString.Length);
						index=0;
						while(index<tempInt1)
						{
							#region prinf printing
							if(Convert.ToChar(dataSave[index])=='%')
							{
								index++;
								//Debug.Write("  %");
								switch(Convert.ToChar(dataSave[index++]))
								{
									case '.':
									{
										// TODO Clean up this hack
										//Debug.Write(".");
										if ((Convert.ToChar(dataSave[index])=='0')||(Convert.ToChar(dataSave[index])=='1')||(Convert.ToChar(dataSave[index])=='2')
										 || (Convert.ToChar(dataSave[index])=='3')||(Convert.ToChar(dataSave[index])=='4')||(Convert.ToChar(dataSave[index])=='5')
										 || (Convert.ToChar(dataSave[index])=='6')||(Convert.ToChar(dataSave[index])=='7')||(Convert.ToChar(dataSave[index])=='8')
										 || (Convert.ToChar(dataSave[index])=='9'))
										{
											//Debug.WriteLine(Convert.ToChar(dataSave[index]));
											dataSave[index]=0x25;
										}
										break;
									}
									case 'c':
									{
										//Debug.WriteLine("c");
										LogFile.Read(data,0,SizeOfChar);
										break;
									}
									case 's':
									{
										//Debug.WriteLine("s");
										LogFile.Read(data,0,2);
										tempInt3=BitConverter.ToInt16(data,0);
										//Debug.WriteLine("    Size of string = "+tempInt3);
										LogFile.Read(data,0,tempInt3);
										//Debug.WriteLine("    Message of string = ");
										//Debug.WriteLine(Encoding.ASCII.GetString(data,0,tepInt3));
										StreamString.Append(Encoding.ASCII.GetString(data,0,tempInt3));
										break;
									}
									case 'd':
									{
										//Debug.WriteLine("d");
										LogFile.Read(data,0,SizeOfInt);
										StreamString.Append(BitConverter.ToInt16(data,0));
										break;
									}
									case 'X':
									{
										//Debug.WriteLine("X");
										LogFile.Read(data,0,SizeOfInt);
										StreamString.Append(data[0].ToString("X"));
										break;
									}
									case 'x':
									{
										//Debug.WriteLine("x");
										LogFile.Read(data,0,SizeOfInt);
										StreamString.Append(data[0].ToString("x"));
										break;
									}
									case 'u':
									{
										//Debug.WriteLine("u");
										LogFile.Read(data,0,SizeOfInt);
										StreamString.Append(BitConverter.ToUInt16(data,0));
										break;
									}
									case 'f':
									{
										//Debug.WriteLine("f");
										LogFile.Read(data,0,SizeOfFloat);
										StreamString.Append(BitConverter.ToSingle(data,0));
										break;
									}
									case 'g':
									{
										//Debug.WriteLine("g");
										LogFile.Read(data,0,SizeOfFloat);
										StreamString.Append(BitConverter.ToSingle(data,0));
										break;
									}
									case 'l':
									{
										//Debug.Write("l");
										switch(Convert.ToChar(dataSave[index++]))
										{
											case 'd':
											{
												//Debug.WriteLine("d");
												LogFile.Read(data,0,SizeOfLong);
												StreamString.Append(BitConverter.ToInt32(data,0));
												break;
											}
											case 'X':
											{
												//Debug.WriteLine("x");
												LogFile.Read(data,0,SizeOfLong);
												StreamString.Append(data[0].ToString("X"));
												StreamString.Append(data[1].ToString("X"));
												StreamString.Append(data[2].ToString("X"));
												StreamString.Append(data[3].ToString("X"));
												break;
											}
											case 'x':
											{
												//Debug.WriteLine("x");
												LogFile.Read(data,0,SizeOfLong);
												StreamString.Append(data[0].ToString("x"));
												StreamString.Append(data[1].ToString("x"));
												StreamString.Append(data[2].ToString("x"));
												StreamString.Append(data[3].ToString("x"));
												break;
											}
											case 'u':
											{
												//Debug.WriteLine("u");
												LogFile.Read(data,0,SizeOfLong);
												StreamString.Append(BitConverter.ToUInt32(data,0));
												break;
											}
											case 'f':
											{
												//Debug.WriteLine("f");
												LogFile.Read(data,0,SizeOfDouble);
												StreamString.Append(BitConverter.ToDouble(data,0));
												break;
											}
											case 'g':
											{
												//Debug.WriteLine("g");
												LogFile.Read(data,0,SizeOfDouble);
												StreamString.Append(BitConverter.ToDouble(data,0));
												break;
											}
										}
										break;
									}
									default:
									{
										break;
									}
								}
							}
								#endregion
								#region sting printing 
							else
							{
								tempChar = Convert.ToChar(dataSave[index++]);
								StreamString.Append(tempChar);
							}
							#endregion
						}
						#endregion
						return StreamString.ToString();		
					case 0x5503: // Stream record
						
						//old:
						# region Stream Records
						/*
						Debug.WriteLine("Stream Record");
						LogFile.Read(data,0,SizeOfStreamOutputRecord);
							
						// retrieve the filename "filename(line):"
						tempInt1=BitConverter.ToUInt16(data,(SizeOfStreamOutputRecord-4));
						tempInt2=BitConverter.ToUInt16(data,(SizeOfStreamOutputRecord-2));
						LogFile.Read(data,0,tempInt1);
						for (i=0;i<tempInt1;i++)
						{
							StreamString.Append(Convert.ToChar(data[i]));
						}
						StreamString.Append("(");
						StreamString.Append(tempInt2);
						StreamString.Append("):");
						
						LogFile.Read(data,0,5);
						TotalByteCount=BitConverter.ToUInt16(data,3);
						tempInt1=1;
						tempInt2=0;
						while (tempInt1 !=0)
						{	
							tempInt1=0;
							if (TotalByteCount!=tempInt2)
							{
								tempInt1=LogFile.Read(data,tempInt2,(TotalByteCount-tempInt2));
							}
							tempInt2=tempInt2+tempInt1;
						}
						ByteCount = 0;
						while ( ByteCount < TotalByteCount )
						{
							type=data[ByteCount];
							ByteCount++;

							switch (type)
							{

								case 1: // signed char
								case 2: //unsigned char
									StreamString.Append(Convert.ToChar(data[ByteCount]));
									ByteCount=SizeOfChar+ByteCount;
									break;
								case 3: // sighned int
								case 5: // sighned long
									StreamString.Append(BitConverter.ToInt32(data,ByteCount));
									ByteCount=SizeOfInt+ByteCount;
									break;
								case 4: // unsighed int
								case 6: // unsighed long
									StreamString.Append(BitConverter.ToUInt32(data,ByteCount));
									ByteCount=SizeOfInt+ByteCount;
									break;
								case 7: // string (char *)
									tempInt1=BitConverter.ToInt16(data,ByteCount);
									ByteCount=ByteCount+2;
									for (i=0;i<tempInt1;i++)
									{
										StreamString.Append(Convert.ToChar(data[ByteCount+i]));
									}
									ByteCount =tempInt1+ByteCount;
									break;
								case 8: // float
									StreamString.Append(BitConverter.ToSingle(data,ByteCount));
									ByteCount = SizeOfFloat+ByteCount;
									break;
								case 9: // double
									StreamString.Append(BitConverter.ToDouble(data,ByteCount));
									ByteCount = SizeOfDouble+ByteCount;
									break;
								case 10: // bool
									tempBool=BitConverter.ToBoolean(data,ByteCount);
									if (tempBool)
									{
										StreamString.Append("false");
									}
									else 
									{
										StreamString.Append("true");
									}
									ByteCount = 1+ByteCount;
									break;
								case 100: // change flag settings
									ByteCount = 2+ByteCount;
									break;
								case 101: // change precision setting
									ByteCount = 1+ByteCount;							
									break;
								default:
									
									{
										Debug.WriteLine("");
										Debug.WriteLine("Bad argument type : "+type);
									}

									break;
							}
						}
						*/
						#endregion

						//new
						# region Parse
						//Debug.WriteLine("Stream Record");
						LogFile.Read(data,0,SizeOfStreamOutputRecord);
						RecordSecond=BitConverter.ToInt32(data,0);
						RecordNanosecond=BitConverter.ToInt32(data,4);
						RecordLevelID=BitConverter.ToInt16(data,8);
						StreamString.Remove(0,StreamString.Length);
					switch(SizeOfDataLogTaskID)
					{
						case 0:
							RecordTaskID=0;
							break;
						case 2:
							RecordTaskID=BitConverter.ToInt16(data,10);
							break;
						case 4:
							RecordTaskID=BitConverter.ToInt32(data,10);
							break;
						case 8:
							RecordTaskID=BitConverter.ToInt64(data,10);
							break;
						default:
							throw new Exception("Bad Node ID");
					}
					switch(SizeOfDataLogNodeID)
					{
						case 0:
							RecordNodeID=0;
							break;
						case 2:
							RecordNodeID=BitConverter.ToInt16(data,(10+SizeOfDataLogTaskID));
							break;
						case 4:
							RecordNodeID=BitConverter.ToInt32(data,(10+SizeOfDataLogTaskID));
							break;
						case 8:
							RecordNodeID=BitConverter.ToInt64(data,(10+SizeOfDataLogTaskID));
							break;
						default:
							throw new Exception("Bad Node ID");
					}
							
						// retrieve the filename "filename(line):"
						tempInt1=BitConverter.ToUInt16(data,(SizeOfStreamOutputRecord-4));
						tempInt2=BitConverter.ToUInt16(data,(SizeOfStreamOutputRecord-2));
						LogFile.Read(data,0,tempInt1);
						StreamString.Append(Encoding.ASCII.GetString(data,0,tempInt1));
						RecordFile=StreamString.ToString();
						RecordLine=tempInt2;
						StreamString.Remove(0,StreamString.Length);
						#region Debug
						//Debug.WriteLine("File Name  :"+RecordFile);
						//Debug.WriteLine("File Line  :"+RecordLine);
						#endregion
						LogFile.Read(data,0,5);
						TotalByteCount=BitConverter.ToUInt16(data,3);
						tempInt1=1;
						tempInt2=0;
						while (tempInt1 !=0)
						{	
							tempInt1=0;
							if (TotalByteCount!=tempInt2)
							{
								tempInt1=LogFile.Read(data,tempInt2,(TotalByteCount-tempInt2));
							}
							tempInt2=tempInt2+tempInt1;
						}
						ByteCount = 0;
						while ( ByteCount < TotalByteCount )
						{
							type=data[ByteCount];
							ByteCount++;

							switch (type)
							{

								case 1: // signed char
								case 2: //unsigned char
									StreamString.Append(Convert.ToChar(data[ByteCount]));
									ByteCount=SizeOfChar+ByteCount;
									break;
								case 3: // sighned int
								case 5: // sighned long
									StreamString.Append(BitConverter.ToInt32(data,ByteCount));
									ByteCount=SizeOfInt+ByteCount;
									break;
								case 4: // unsighed int
								case 6: // unsighed long
									StreamString.Append(BitConverter.ToUInt32(data,ByteCount));
									ByteCount=SizeOfInt+ByteCount;
									break;
								case 7: // string (char *)
									tempInt1=BitConverter.ToInt16(data,ByteCount);
									ByteCount=ByteCount+2;
									StreamString.Append(Encoding.ASCII.GetString(data,ByteCount,tempInt1));
									ByteCount =tempInt1+ByteCount;
									break;
								case 8: // float
									StreamString.Append(BitConverter.ToSingle(data,ByteCount));
									ByteCount = SizeOfFloat+ByteCount;
									break;
								case 9: // double
									StreamString.Append(BitConverter.ToDouble(data,ByteCount));
									ByteCount = SizeOfDouble+ByteCount;
									break;
								case 10: // bool
									tempBool=BitConverter.ToBoolean(data,ByteCount);
									if (tempBool)
									{
										StreamString.Append("false");
									}
									else 
									{
										StreamString.Append("true");
									}
									ByteCount = 1+ByteCount;
									break;
								case 100: // change flag settings
									ByteCount = 2+ByteCount;
									break;
								case 101: // change precision setting
									ByteCount = 1+ByteCount;							
									break;
								default:
									
								{
									//Debug.WriteLine("");
									//Debug.WriteLine("Bad argument type : "+type);
								}

									break;
							}
						}
						#endregion
						return StreamString.ToString();
					case 0x5504: // Periodic log output record
						#region Periodic log output record
						 Debug.WriteLine("Periodic Log Output Record");
						LogFile.Read(data,0,SizeOfPeriodicLogOutputRecord);
						tempInt1=BitConverter.ToUInt16(data,(SizeOfPeriodicLogOutputRecord-2));
						for(i=0;i<tempInt1;i++)
						{
							LogFile.Read(data,0,4);
							tempInt2=BitConverter.ToInt16(data,0);
							LogFile.Read(data,0,tempInt2);
						}
						break;
						#endregion
					case 0x5505: //Periodic set record
						#region Periodic set record
						 Debug.WriteLine("Periodic set Record");
						LogFile.Read(data,0,SizeOfPeriodicSetRecord);
						tempInt1=BitConverter.ToUInt16(data,(SizeOfPeriodicSetRecord-2));
						LogFile.Read(data,0,tempInt1);
						break;
						#endregion
					case 0x5506: // Periodic log item record
						#region Periodic Log Item Record
						 Debug.WriteLine("Periodic Log Item Record");
						LogFile.Read(data,0,SizeOfPeriodicLogItemRecord);
						tempInt3=BitConverter.ToInt16(data,SizeOfPeriodicLogItemRecord-6);
						tempInt2=BitConverter.ToInt16(data,SizeOfPeriodicLogItemRecord-4);
						tempInt1=BitConverter.ToInt16(data,SizeOfPeriodicLogItemRecord-2);
						LogFile.Read(data,0,tempInt1);
						LogFile.Read(data,0,tempInt2);
						LogFile.Read(data,0,tempInt3);
						break;
						#endregion
					case 0x5507: // Task create record
						#region Task create record
						 Debug.WriteLine("Task Create Record");
						LogFile.Read(data,0,SizeOfCreateTaskRecord);
						
						tempInt1=BitConverter.ToUInt16(data,(SizeOfCreateTaskRecord-2));
						#region DEBUG
						Debug.WriteLine("Size of the string read = "+tempInt1);
						LogFile.Read(data,0,tempInt1);
						
						#endregion
						break;
						#endregion
						
					case 0x5508: // Task delete record
						#region Task Delete record
						 Debug.WriteLine("Task Delete Record");
						LogFile.Read(data,0,SizeOfTaskDeletedRecord);
						break;
						#endregion
					case 0x5509: // Network log header
						#region Network Log Header
						 Debug.WriteLine("Network Log Record");
						LogFile.Read(data,0,SizeOfStartOfNetworkConnectionRecord);
						tempInt1=BitConverter.ToUInt16(data,(SizeOfStartOfNetworkConnectionRecord-2));
						LogFile.Read(data,0,tempInt1);
						break;
						#endregion
					case 0x55f0: // Binary record
						#region Binary Record
						 Debug.WriteLine("Binary Record Record");
						LogFile.Read(data,0,SizeOfBinaryRecord); // from Spec
						tempInt1=BitConverter.ToInt32(data,SizeOfBinaryRecord-4);
						LogFile.Read(data,0,tempInt1);
						break;
						#endregion
					case 0x55fd: // End of log file record
						#region End of Log File Record
						 Debug.WriteLine("End Of Log File Record");
						LogFile.Read(data,0,SizeOfEndOfLogFileRecord);
						break;
						#endregion
					case 0x55fe: // Time stamp record
						#region Time Stamp Record
						 Debug.WriteLine("Time Stamp Record");
						LogFile.Read(data,0,SizeOfTimeStampRecord);
						
						break;
						#endregion
					case 0x55ff: // Missing record definition
						#region DEBUG
						 Debug.WriteLine("Missing Record Definition");
						#endregion
						break;

					default: 
						#region DEBUG
						 Debug.WriteLine("Missing Record Definition");
						#endregion
						throw new IOException("Missing Record");
						
				}
			}
		
			return "";
		}


		// this function closes the log file.
		public void CloseLog()
		{
			try
			{
				LogFile.Close();
			}
				#region Catches
			catch (NullReferenceException NREx)
			{
				Debug.WriteLine("NULL REFERENCE EXCEPTION: CloseLog()");
				Debug.WriteLine(NREx.Message);
				Debug.WriteLine("PROGRAM TERMINATED");

			}
			catch
			{
				Debug.WriteLine("EXCEPTION: CloseLog()");
				Debug.WriteLine("PROGRAM TERMINATED");
			}
			#endregion
		}
	}
}
