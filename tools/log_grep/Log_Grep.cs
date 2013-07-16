#region Using
using System;
using System.Text;
using System.Collections;
using System.Collections.Specialized;
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

namespace DLGrep
{
	#region Main Function & Class 1
	/// <summary>
	/// Class1
	/// This is a default class that holds the main executable.
	/// </summary>
	class Class1
	{
		static int Main(string[] args)
		{
			Grepper dlgrep = new Grepper();
			return dlgrep.RealMain(args);
		}
	}

    /// <summary>
    ///  The worker class that carries out the grep operations
    /// </summary>
	class Grepper
	{
		string[] Pattern = new string[10]; // these variables are used to keep track of the 
		int PatternCount = 0;	    	   // regular expressions
		int BufferSize;					// this is set to the backlog buffer for messages.
		int BackPrint = 1;				// these variables hold data on how many messages to
		int ForwardPrint = 0;   		// and how many messages to print after the target message 
		object WriteControl = new object();
		int NumThreads = 1;
		Regex[] RegexPattern;			// this is a list of all the regular expresions
		Regex FilePattern = new Regex(@"\.dlog$",RegexOptions.Compiled);// this is to ensure we only read dlog files.
        string FileOrDir = ".";
        FileManager LogFileManager = new FileManager();
        bool OptionH = false;
		bool Optioni = false;
        bool Optionc = false;
        bool Optionl = false;
        int MinFileLen = 20*1024;      // mininum size for log files in bytes (100 KB)
        int MaxFileLen = 2*1024*1024;   // maximum size for log files (2 MB)
        DirectoryInfo TopDir = new DirectoryInfo( Directory.GetCurrentDirectory() );
        Uri TopDirUri = new Uri( Directory.GetCurrentDirectory() );
        int TotFileMatchCount = 0;

        [STAThread]

        /// <summary>
        /// Command-line help info.
        /// </summary>
        void help()
        {
            Console.WriteLine(" dlgrep [OPTION] -e PATTERN [-e PATTERN2 ...] [FILE_OR_DIR]");
            Console.WriteLine(" ");
            Console.WriteLine(" Search for PATTERN in DLOG file(s). Only message records are searched.");
            Console.WriteLine(" However, message records are prepended with timestamp, node name, task name, ");
            Console.WriteLine(" and the source filename and line number; thus, they are searchable too.");
            Console.WriteLine(" Returns with status: 0 if a pattern was matched, 1 if no matches, 2 on error.");
            Console.WriteLine(" ");
            Console.WriteLine(" Options:");
            Console.WriteLine("  -e  PATTERN : Regular expression to search (grep) for");
            Console.WriteLine("  -ef PATTERN : Regular expression to match file names against. Default: \\.dlog$");
            Console.WriteLine("  -i          : Ignore case for regular expression(s) set by -e");
            Console.WriteLine("  -c          : Print *only* the count of matching lines per file");
            Console.WriteLine("  -l          : Print *only* names of files containing matches");
            Console.WriteLine("  -H          : Print the filename for each match as a prefix");
            Console.WriteLine("  -A   NUM    : Print NUM lines after a match");
            Console.WriteLine("  -B   NUM    : Print NUM lines before a match");
            Console.WriteLine("  -C   NUM    : Print NUM lines surounding a match");
            Console.WriteLine("  -t   NUM    : Set the number of search threads. Default: 1");
            Console.WriteLine("  -min NUM    : Minimum log file size, in kbytes. Default: 20");
            Console.WriteLine("  -max NUM    : Maximum log file size, in kbytes. Default: 2048 (2MB)");
            Console.WriteLine("  -h          : Display this help message and exit");
            Console.WriteLine("  FILE_OR_DIR : Either a file or directory path; a directory will be");
            Console.WriteLine("                recursively searched. Default: . (current directory)");
            Console.WriteLine(" ");
            Console.WriteLine(" Examples:");
            Console.WriteLine(" # Search for proc.cpp *or* 'Displayed Alarm' in current directory");
            Console.WriteLine("   dlgrep -e proc.cpp -e 'Displayed Alarm'");
            Console.WriteLine("   dlgrep -e 'proc.cpp|Displayed Alarm'");
            Console.WriteLine(" # Count button pushes in all logs from 2012-2013");
            Console.WriteLine("   dlgrep -c -e 'GUI button pushed' -ef '1T.*_201[23].*.dlog");
            Console.WriteLine(" # Find logs in directory T5001 with BUILD versions 8.8 or 8.9");
            Console.WriteLine("   dlgrep -l -e BUILD=8.[89] T5001");
            Console.WriteLine(" ");
            Console.WriteLine(" dlgrep Version 1.5, Jul-2013 -- Copyright (C) 2013 Terumo BCT");
        }

        /// <summary>
		/// The main entry point for the application. Reads the command line options and
        /// spawns a searcher thread for each log file to carry out the search requests.
		/// </summary>
		public int RealMain(string[] args)
		{
            // read the commandline
			#region Command Line Parsing
            for (int i = 0; i < args.Length; i++)
			{
				switch (args[i])
				{
                    case "-h":
                    case "--help":
                        help();
                        return 0;

                    case "-e":
						i++;
						Pattern[PatternCount]=args[i];
						PatternCount++;
						Debug.WriteLine("Option -e : pattern found = "+args[i]);
						break;

                    case "-ef":
                        i++;
                        FilePattern = new Regex(args[i], RegexOptions.Compiled);
                        break;

                    case "-A":
                        i++;
                        this.ForwardPrint = Convert.ToInt16(args[i]);
                        if (ForwardPrint < 1)
                        {
                            Console.WriteLine("Usage: -A NUM ; requires NUM >= 1");
                            return 2;
                        }
                        break;

                    case "-B":
                        i++;
                        BackPrint = (Convert.ToInt16(args[i]) + 1);
                        if (BackPrint < 1)
                        {
                            Console.WriteLine("Usage: -B NUM ; requires NUM >= 1");
                            return 2;
                        }
                        break;

                    case "-C":
                        i++;
                        BufferSize = Convert.ToInt16(args[i]);
                        BackPrint = BufferSize + 1;
                        ForwardPrint = BufferSize;
                        if (BackPrint < 1)
                        {
                            Console.WriteLine("Usage: -C NUM ; requires NUM >= 1");
                            return 2;
                        }
                        break;

                    case "-H":
						OptionH=true;
						break;

                    case "-t":
						i++;
						try
						{
							NumThreads=Convert.ToInt16(args[i]);
						}
						catch
						{
                            NumThreads = 0;
						}
						if (NumThreads < 1)
						{
							Console.WriteLine("Usage: -t NUM ; bad threadcount");
							return 2;
						}
						break;

					case "-i":
						Optioni = true;
						break;

					case "-c":
						Optionc = true;
						break;

                    case "-l":
                        Optionl = true;
                        break;

                    case "-min":
                        i++;
                        MinFileLen = (Convert.ToInt16(args[i]) + 1) * 1024;
                        break;

                    case "-max":
                        i++;
                        MaxFileLen = (Convert.ToInt16(args[i]) + 1) * 1024;
                        break;

                    default:
                        FileOrDir = args[i];
						break;
				}
			}

            // Check for required argument(s)
            if (args.Length < 2)
            {
                help();
                return 2;
            }
            if (PatternCount == 0)
            {
                Console.WriteLine("dlgrep: No search PATTERN(s) were found!");
                return 2;
            }
            if (!LogFileManager.Initialize(FileOrDir))
            {
                Console.WriteLine("dlgrep: Invalid FILE_OR_DIR argument: " + FileOrDir);
                return 1;
            }
            if (Optionl && Optionc)
            {
                Console.WriteLine("dlgrep: Option -l supercedes option -c");
                Optionc = false;
            }
            #endregion

			#region initialize the buffers.
			
			RegexPattern=new Regex[PatternCount];
			
			for(int i=0;i<PatternCount;i++)
			{
				if (Optioni)
					RegexPattern[i]=new Regex(Pattern[i],RegexOptions.Compiled | RegexOptions.IgnoreCase);
				else
					RegexPattern[i]=new Regex(Pattern[i],RegexOptions.Compiled);
			}
			#endregion
			// find a file name...
			Thread[] Searchers;
			Searchers = new Thread[NumThreads];

            // Create & Start the thread(s)
            for (int i = 0; i < NumThreads; i++)
            {
                Searchers[i] = new Thread(new ThreadStart(searcher));
                Searchers[i].Start();
            }
            // Wait for the started threads to complete
            for (int i = 0; i < NumThreads; i++)
            {
                Searchers[i].Join();
            }
            while (! LogFileManager.done )
			{
                Console.WriteLine("Sleeping 1 sec for LogFileManager to complete ...");
				Thread.Sleep(1000);
			}
            if (Optionl)
            {
                Console.WriteLine("There were " + TotFileMatchCount + " matching logs");
            }

            // Like grep, exit with status 0 if any line matched, and 1 otherwise
			return (TotFileMatchCount > 0 ? 0 : 1);
		}

        /// <summary>
        /// Does the search operation on the records in a log file.
        /// </summary>
		void searcher()
		{
			#region Variables
            FileInfo fileInfo;
            string LogFileAbs  = "";	    // this is the full pathname of the log file that will be read
            string LogFileRel  = "";        // this is the relative filename
			string LogFileName = "";		// this is the short filename (no path)
            Uri LogFileUri;
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
			bool DoneRecursing = false;
			int FileMatchCount = 0;
            int PattMatchCount = 0;
			MessageBuffer = new string[BackPrint];
			PrintFlag = new Boolean[BackPrint];		
			#endregion

			LogDefinition LogFile = new LogDefinition();
			while(!DoneRecursing)
			{
				#region Get the file name
				lock (LogFileManager)
				{
					fileInfo = LogFileManager.Next();
                    LogFileAbs = fileInfo.FullName;
                    LogFileName = fileInfo.Name;
                }

                // Check for the termination file
                if (LogFileName == "EOS")
                {
                    Debug.WriteLine("Finished the stack");
                    DoneRecursing = true;
                    break;
                }

                // Check if the file name pattern matches
                FileMatches = FilePattern.Matches(LogFileName);
                if (FileMatches.Count == 0)
                {
                    done = true;
                }
                // Check if file size meets min/max criteria
                else if (fileInfo.Length < MinFileLen || fileInfo.Length > MaxFileLen)
                {
                    // Console.WriteLine("dlgrep: file too small/big: " + LogFileRel + " sizeKB=" + fileInfo.Length / 1024);
                    done = true;
                }
                else // this file is okay for searching
                {
                    done = false;
                }
				#endregion
				
				if (!done)
				{
                    LogFileRel = LogFileAbs.Substring(TopDir.FullName.Length + 1);
                    LogFileUri = new Uri(LogFileAbs);
                    string rel = TopDirUri.MakeRelativeUri(LogFileUri).ToString();
                    LogFileRel = rel.Substring(TopDir.Name.Length + 1);
					Debug.WriteLine(string.Format("File Name : {0}", LogFileAbs));

                    // reset for a new file
					MessageIndex = BackPrint;
					StillPrinting = 0;
                    PattMatchCount = 0;

                    // Open the file
					try
					{
						LogFileStream=File.OpenRead(LogFileAbs);
			    		// initialize the file
						LogFile.InitializeLog(LogFileStream);
					}
					#region Catches
					catch (ApplicationException ApEx)
					{
						if (!(ApEx.Message == "Deflated stream ends early."))
						{
							Debug.WriteLine("Log_Grep_Main :");
							Debug.WriteLine("Application Exception found in file : " + LogFileAbs);
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
							Debug.WriteLine("  In file : "+LogFileAbs);
							Debug.WriteLine("  Exception Message : "+Ex.Message);
							done=true;
						}
					}
					#endregion	
					
					// Read message records and look for matches
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
						
								{
									#region pattern matching
                                    // For each pattern defined ...
									for(i = 0; i < PatternCount; i++)
									{
										Matches=RegexPattern[i].Matches(MessageString);
										if(Matches.Count > 0) // we have a match
										{
                                            PattMatchCount++;
                                            if (Optionl)
                                            {
                                                // We're done with this file on the first match
                                                done = true;
                                                lock (WriteControl)
                                                {
                                                    Console.WriteLine(LogFileRel);
                                                }
                                                break;
                                            }
                                            if (Optionc)
                                            {
                                                // Counting the lines that have a match; skip out on first match
                                                // and proceed to the next line
                                                break;
                                            }
                                            StillPrinting = ForwardPrint; // set the forward print variable
											for(j=(BackPrint-1);j>=0;j--)
											{
                                                // print these messages in the backlog that 
                                                // haven't been printed yet
												tempInt1=(MessageIndex-j)%BackPrint;
												if(!(PrintFlag[tempInt1]))
												{
													lock (WriteControl)
													{
														if (OptionH)
														{
															Console.Write(LogFileRel + ":");
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
											Console.Write(LogFileRel + ":");
										}
										Console.WriteLine(MessageBuffer[MessageIndex%BackPrint]);
									}
									PrintFlag[MessageIndex%BackPrint]=true;
									StillPrinting--;
								}
								#endregion
							}
						} // while ! done reading records on the current file

                        if (PattMatchCount > 0)
                        {
                            // Count this file as having had a match
                            FileMatchCount++;
                        }
                        if (Optionc)
                        {
                            lock (WriteControl)
                            {
                                Console.WriteLine(LogFileRel + ":" + PattMatchCount);
                            }
                        }

                        Debug.WriteLine("---(EOF)---");
					} // try
					#region Catches
					catch (IOException IOEx)
					{
						if (IOEx.Message != "EOF")
						{
							Debug.WriteLine("Log_Grep_Main :");
							Debug.WriteLine("IO Exception found in file : " + LogFileAbs);
							Debug.WriteLine("  Exception Message     : " + IOEx.Message);
						}
					}
					catch (ApplicationException ApEx)
					{
						if (!(ApEx.Message == "Deflated stream ends early."))
						{
							Debug.WriteLine("Log_Grep_Main :");
							Debug.WriteLine("Application Exception found in file : " + LogFileAbs);
							Debug.WriteLine("  Exception Message     : " + ApEx.Message);
						}
					}
					catch (ZipException ZipEx)
					{
						Debug.WriteLine("Log_Grep_Main :");
						Debug.WriteLine("Zip Exception found in file : " + LogFileAbs);
						Debug.WriteLine("  Exception Message     : " + ZipEx.Message);
					}
					catch (Exception Ex)
					{
						Debug.WriteLine("Log_Grep_Main :");
						Debug.WriteLine("Exception found in file : " + LogFileAbs);
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
						Debug.WriteLine("Log File Failed To Close: File name : " +LogFileAbs);
					}
					#endregion
				}
			}
            // Update the total file match count
            Interlocked.Add(ref TotFileMatchCount, FileMatchCount);
		}
	
	}
	#endregion
}
