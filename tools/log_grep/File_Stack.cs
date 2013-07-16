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
		Stack FileStack = new Stack();

		// this function returns the next file that is found
		public FileInfo Next()
		{
            object Current = FileStack.Pop();
			if (Current is FileInfo)
			{
				Debug.WriteLine("test: file FullName");
				Debug.WriteLine(((FileInfo)Current).FullName);
                return (FileInfo)Current;
			}
			else if (Current is DirectoryInfo)
			{
                DirectoryInfo CurrentDir = (DirectoryInfo)Current;
				Debug.WriteLine("test: directory name");
				Debug.WriteLine(CurrentDir.Name);
				try
				{
                    FileSystemInfo[] Items = CurrentDir.GetFileSystemInfos();
					foreach ( FileSystemInfo Item in Items)
					{
						FileStack.Push(Item);
						
						#region debug
						if (Item is FileInfo)
							Debug.WriteLine(((FileInfo)Item).FullName);
						else if (Item is DirectoryInfo)
							Debug.WriteLine(((DirectoryInfo)Item).FullName);
						else
							Debug.WriteLine("WTF");
                        #endregion
					}
					return Next();
				}
				catch
				{
					return Next();
				}
			}
			else if (Current is String)
			{
				done = true;
				FileStack.Push(Current);
                return new FileInfo("EOS");
			}
			else
			{
                return new FileInfo("EOS");
			}
		}
		
		// this function initiates the file stack, and puts the first few bits on it.
		public bool Initialize(string FileName)
		{
            bool okay = true;
			FileInfo FileArgument;
			DirectoryInfo DirectoryArgument;

            FileStack.Push( "EOS" );               // bottom of the stack

            if (Directory.Exists(FileName))
			{
				DirectoryArgument = new DirectoryInfo(FileName);
				FileStack.Push(DirectoryArgument);
			}
            else if (File.Exists(FileName))
            {
                FileArgument = new FileInfo(FileName);
                FileStack.Push(FileArgument);
            }
            else
            {
                okay = false;
            }
            return okay;
		}
	}
	#endregion
}
