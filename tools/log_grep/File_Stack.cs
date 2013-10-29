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

namespace DLGrep
{
	#region File Manager Class
	/// <summary>
	/// FileManager
	/// This class will take care of getting filenames of files that need to be searched.
	/// The main reason for this class is to keep it seperate from the main code.
	/// </summary>
	///
	class FileManager
	{
		public bool done = false;
        public bool recurse = false;
        public string dlogPattern = "*.dlog";
		Queue myQueue = new Queue();
            
        // Adds a directory or file to the stack for consideration
        public bool Add(string FileName)
        {
            bool okay = true;

            if (Directory.Exists(FileName))
            {
                DirectoryInfo DirectoryArgument = new DirectoryInfo(FileName);
                myQueue.Enqueue(DirectoryArgument);
            }
            else if (File.Exists(FileName))
            {
                FileInfo FileArgument = new FileInfo(FileName);
                myQueue.Enqueue(FileArgument);
            }
            else
            {
                okay = false;
            }
            done = (myQueue.Count == 0);
            return okay;
        }

		// this function returns the next file that is found
		public FileInfo Next()
		{
            done = (myQueue.Count == 0);
            if (done)
            {
                return new FileInfo("EOQ");
            }

            object Current = myQueue.Dequeue();
			if (Current is FileInfo)
			{
                FileInfo CurrFile = (FileInfo)Current;
				Debug.WriteLine("NextFile: " + CurrFile.FullName);
                done = (myQueue.Count == 0);
                return CurrFile;
			}
			else if (Current is DirectoryInfo)
			{
                DirectoryInfo CurrentDir = (DirectoryInfo)Current;
				Debug.WriteLine("NextDir+: " + CurrentDir.Name);
				try
				{
                    // If recursing, add the sub-directories first
                    if (recurse)
                    {
                        DirectoryInfo[] SubDirs = CurrentDir.GetDirectories();
                        foreach (DirectoryInfo dir in SubDirs)
                        {
                            myQueue.Enqueue(dir);
                        }
                    }
                    // Get files in the current directory that match the pattern
                    FileSystemInfo[] Items = CurrentDir.GetFileSystemInfos(dlogPattern);
					foreach (FileSystemInfo Item in Items)
					{
						myQueue.Enqueue(Item);					
						#region debug
						if (Item is FileInfo)
							Debug.WriteLine(((FileInfo)Item).FullName);
						else if (Item is DirectoryInfo)
							Debug.WriteLine(((DirectoryInfo)Item).FullName);
						else
							Debug.WriteLine("WTF");
                        #endregion
					}
                    // Return the next item from the queue
					return Next();
				}
				catch
				{
					return Next();
				}
			}
			else
			{
                #region debug
                Debug.WriteLine("Huh? This object is whacked!");
                #endregion 
                // skip whatever this is
                return Next();
			}
		}
    }
		
	#endregion
}
