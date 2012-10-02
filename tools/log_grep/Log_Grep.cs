#region Using
using System;
using System.Text;
using System.Collections;
using System.IO;
using System.Diagnostics;
using System.Runtime.Serialization.Formatters.Binary;
using System.Data;
using System.Text.RegularExpressions;
using System.Timers;
using System.Threading;

#region ICSharpCode
using ICSharpCode.SharpZipLib.BZip2;
using ICSharpCode.SharpZipLib.Zip;
using ICSharpCode.SharpZipLib;
using ICSharpCode.SharpZipLib.Zip.Compression;
using ICSharpCode.SharpZipLib.Zip.Compression.Streams;
using ICSharpCode.SharpZipLib.GZip;
#endregion

#endregion

using System.Collections.Specialized;
namespace Log_Grep
{
	#region Main Function & Class 1
	/// <summary>
	/// Class1
	/// This is a default class that holds the main executable.
	/// </summary>
	class Class1
	{
		static void Main(string[] args)
		{
			log_grep loggrep= new log_grep();
			loggrep.RealMain(args);
		}
	}
	class log_grep
	{

		string[] Pattern=new string[10];// these variables are used to keep track of the 
		int PatternCount=0;				// regular expressions
		int BufferSize;					// this is set to the backlog buffer for messages.
		int BackPrint=1;				// these variables hold data on how many messages to
		int ForwardPrint=0;				// and how many messages to print after the target message 
		object WriteControl = new object();
		bool FileNamePrintFlag;
		int ThreadCount=1;
		FileManager LogFileManager = new FileManager();
		Regex[] RegexPattern;			// this is a list of all the regular expresions
		Regex FilePattern = new Regex(@"\.dlog",RegexOptions.Compiled);// this is to ensure we onl read dlog files.
		bool OptionH=false;
		bool Optioni=false;
		bool Optionl=false;
		
	
		/// <summary>
		/// The main entry point for the application.
		/// </summary>
		[STAThread]
		
		public void RealMain(string[] args)
		{
			int i;
			// read the commandline
			#region Command Line Parsing
			if (args.Length<3)
			{
                help();
				return;
			}
			for(i=0;i<args.Length;i++)
			{
				switch (args[i])
				{
					case "-A":
						i++;
						this.ForwardPrint=Convert.ToInt16(args[i]);
						if (ForwardPrint<1)
						{
							Console.WriteLine("Usage: -A Num ; Num must be great than or equal to 1");
							return;
						}
						break;
					case "-B":
						i++;
						BackPrint=(Convert.ToInt16(args[i])+1);
						if (BackPrint<1)
						{
							Console.WriteLine("Usage: -B Num ; Num must be great than or equal to 1");
							return;
						}
						break;
					case "-C":
						i++;
						BufferSize=Convert.ToInt16(args[i]);
						BackPrint=BufferSize+1;
						ForwardPrint=BufferSize;
						if (BackPrint<1)
						{
							Console.WriteLine("Usage: -C Num ; Num must be great than or equal to 1");
							return;
						}
						break;
					
					case "-e":
						i++;
						Pattern[PatternCount]=args[i];
						PatternCount++;
						Debug.WriteLine("Option -e : pattern found = "+args[i]);
						break;
					
					case "-H":
						OptionH=true;
						break;

                    case "-t":
						i++;
						try
						{
							ThreadCount=Convert.ToInt16(args[i]);
						}
						catch
						{
							Console.WriteLine("bad threadcount");
							return;
						}
						if (ThreadCount <1)
						{
							Console.WriteLine("bad threadcount");
							return;
						}
						break;

                    case "-h":
                    case "--help":
                        help();
						return;
						
					case "-i":
						Optioni=true;
						break;
					case "-l":
						Optionl=true;
						break;
					case "-ef":
						i++;
						FilePattern= new Regex(args[i],RegexOptions.Compiled);
						break;
					default:
						LogFileManager.Initialize(args[i]);
						break;
				}
				#endregion
			}
			#region initialize the buffers.
			
			RegexPattern=new Regex[PatternCount];
			
			for(i=0;i<PatternCount;i++)
			{
				if (Optioni)
					RegexPattern[i]=new Regex(Pattern[i],RegexOptions.Compiled | RegexOptions.IgnoreCase);
				else
					RegexPattern[i]=new Regex(Pattern[i],RegexOptions.Compiled);
			}
			#endregion
			// find a file name...
			Thread[] Searchers;
			Searchers = new Thread[ThreadCount];
			for (i=0;i<ThreadCount;i++)
			{
				Searchers[i] = new Thread( new ThreadStart(searcher));
				Searchers[i].Start();
			}
            // Wait for the started threads to complete
            for (i = 0; i < ThreadCount; i++)
            {
                Searchers[i].Join();
            }
            while (! LogFileManager.done )
			{
                Console.WriteLine("Sleeping 1 sec for LogFileManager to complete ...");
				Thread.Sleep(1000);
			}
			return;
		}

        void help()
        {
            Console.WriteLine(" Log_Grep Version 1.4 -- Terumo BCT ");
            Console.WriteLine(" Options :");
            Console.WriteLine("  -t         : Set the number of search threads. Default is 1");
            Console.WriteLine("  -h         : Displays this help message");
            Console.WriteLine("  -e  [RegEx]: A regular expression to search (grep) for");
            Console.WriteLine("  -ef [RegEx]: A regular expression to match file names against");
            Console.WriteLine("  -i         : Ignores case for the regular expressions");
            Console.WriteLine("  -l         : Supresses normal output and just lists file names");
            Console.WriteLine("  -H         : Prefixes the output with the file name");
            Console.WriteLine("  -A  [NUM]  : Prints NUM lines after a match");
            Console.WriteLine("  -B  [NUM]  : Prints NUM lines before a match");
            Console.WriteLine("  -C  [NUM]  : Prints NUM lines surounding the match");
            Console.WriteLine("      [FILE] : Either a path or a file. For path, it will recurse down the path");
        }

		void searcher()
		{
			#region Variables
			
			string LogFileName="";			// this is the filename that will be read for the grep.
			string[] MessageBuffer;			// this is the buffer that the messages get stored in.
			bool[] PrintFlag;
			int MessageIndex=0;
			int StillPrinting=0;
			FileStream LogFileStream;		// file manipulation variables.
			MatchCollection Matches;
			MatchCollection FileMatches;
			bool done=false;				// temp variables, and controll variables
			string MessageString;
			int i;
			int j;
			int tempInt1;
			bool DoneRecursing=false;
			int FileMatchCount=0;
			MessageBuffer=new string[BackPrint];
			PrintFlag=new Boolean[BackPrint];
			
			#endregion
			LogDefinition LogFile=new LogDefinition();
			while(!DoneRecursing)
			{
				#region Get the file name
				lock (LogFileManager)
				{
					LogFileName=LogFileManager.Next();
				}
				FileNamePrintFlag=false;
				if (LogFileName == "EOS")
				{
					Debug.WriteLine("Finished the stack");
					DoneRecursing=true;
					break;
				}
				else // make sure the file is valid
				{
					FileMatches=FilePattern.Matches(LogFileName);
					if(FileMatches.Count >0) // we have a match
						done = false;
					else 
						done = true;
				}	
				
				
				#endregion
				
				if (!done)
				{
					Debug.WriteLine(string.Format("File Name : {0}", LogFileName));
					// reset for a new file
					MessageIndex=BackPrint;
					StillPrinting=0;
					// Open the file
					try
					{
						LogFileStream=File.OpenRead(LogFileName);
					
				
					// initialize the file
					
						LogFile.InitializeLog(LogFileStream);
					}
						#region Catches
					catch (ApplicationException ApEx)
					{
						if (!(ApEx.Message == "Deflated stream ends early."))
						{
							Debug.WriteLine("Log_Grep_Main :");
							Debug.WriteLine("Application Exception found in file : " + LogFileName);
							Debug.WriteLine("  Exception Message     : " + ApEx.Message);
						}
					}
					catch(Exception Ex)
					{
						if ((Ex is IOException)&&(Ex.Message=="EOF"))
						{
							done=true;
						}
						else
						{
							Debug.WriteLine("Main: InitializeLog: no catch for Exception: " + Ex.GetType());
							Debug.WriteLine("Main:File.OpenRead: no catch for Exception: " + Ex.GetType());
							Debug.WriteLine("  In file : "+LogFileName);
							Debug.WriteLine("  Exception Message : "+Ex.Message);
							done=true;
						}
					}
					#endregion	
					
					
					// read messages
					try
					{
						while(!done)
						{
							try
							{
								MessageString=LogFile.ReadMessage();
							}
								#region catches
							catch (IOException IOEx)
							{
								throw new IOException(IOEx.Message);
							}
							#endregion
							if (MessageString=="")
							{
								done=true;
							}
							else
							{
								
								// DEBUG message
								Debug.WriteLine(MessageString);

								// store the message.
						
								MessageIndex++;
								MessageBuffer[MessageIndex%BackPrint]=MessageString;
								PrintFlag[MessageIndex%BackPrint]=false;
						
						
								// try to match the message
								if (Optionl)
								{
									
									#region pattern matching
									for(i=0;i<PatternCount;i++)
									{
										Matches=RegexPattern[i].Matches(MessageString);
										if(Matches.Count >0) // we have a match
										{
											if(!FileNamePrintFlag)
											{
												FileNamePrintFlag=true;
												lock(WriteControl)
												{
													Console.WriteLine(LogFileName);
												}
												FileMatchCount++;
												done=true;
											}
										}
									}
									#endregion
								}
								else
								{
									#region pattern matching
									for(i=0;i<PatternCount;i++)
									{
										Matches=RegexPattern[i].Matches(MessageString);
										if(Matches.Count >0) // we have a match
										{
											StillPrinting=ForwardPrint; // set the forward print variable
											for(j=(BackPrint-1);j>=0;j--)	// pritn thise messages in the backlog that 
												// haven't been printed yet
											{
												tempInt1=(MessageIndex-j)%BackPrint;
												if(!(PrintFlag[tempInt1]))
												{
													lock (WriteControl)
													{
														if (OptionH)
														{
															Console.Write(LogFileName + ":");
														}
														Console.WriteLine(MessageBuffer[tempInt1]);
													}
													PrintFlag[tempInt1]=true;
												}	
											}
								
										}
									}
									#endregion
								}
							
								// print message if it is required.
								#region printing
								if((StillPrinting>0)&&(!(PrintFlag[MessageIndex%BackPrint])))
								{
									lock (WriteControl)
									{
										if (OptionH)
										{
											Console.Write(LogFileName + ":");
										}
									
										Console.WriteLine(MessageBuffer[MessageIndex%BackPrint]);
									}
									PrintFlag[MessageIndex%BackPrint]=true;
									StillPrinting--;
								}
								#endregion
								
						
							}
			
						}
						Debug.WriteLine("");
					}
						#region Catches
					catch (IOException IOEx)
					{
						if (IOEx.Message != "EOF")
						{
							Debug.WriteLine("Log_Grep_Main :");
							Debug.WriteLine("IO Exception found in file : " + LogFileName);
							Debug.WriteLine("  Exception Message     : " + IOEx.Message);
						}
					}
					catch (ApplicationException ApEx)
					{
						if (!(ApEx.Message == "Deflated stream ends early."))
						{
							Debug.WriteLine("Log_Grep_Main :");
							Debug.WriteLine("Application Exception found in file : " + LogFileName);
							Debug.WriteLine("  Exception Message     : " + ApEx.Message);
						}
					}
					catch (ZipException ZipEx)
					{
						Debug.WriteLine("Log_Grep_Main :");
						Debug.WriteLine("Zip Exception found in file : " + LogFileName);
						Debug.WriteLine("  Exception Message     : " + ZipEx.Message);
					}
					catch (Exception Ex)
					{
						Debug.WriteLine("Log_Grep_Main :");
						Debug.WriteLine("Exception found in file : " + LogFileName);
						Debug.WriteLine("  Exception type        : " + Ex.GetType());
						Debug.WriteLine("  Exception Message     : " +Ex.Message);
					}
					#endregion
					
					try 
					{
						LogFile.CloseLog();
					}
						#region Catches
					catch 
					{
						Debug.WriteLine("Log File Failed To Close: File name : " +LogFileName);
					}
					#endregion
				}
			}
			if (Optionl)
			{
				lock (WriteControl)
				{
					Console.WriteLine("There were "+FileMatchCount+" matching logs");
				}
			}
			
		}
	
	}
	#endregion
}
