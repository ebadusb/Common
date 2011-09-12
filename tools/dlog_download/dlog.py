"""
This is a simple script to download a dlog file from the quad when you are looking at an issue tracker. From the command line, specify the dlog file you want to download as an argument to this script. The dlog file will be downloaded from the quad and decoded using DLDGui -
                                                    Sriram Sundararajan
"""
import os, glob, sys, re
import shutil, logging
from logging.handlers import RotatingFileHandler
from ConfigParser import SafeConfigParser
""" 
    Input an IT no. and a dlog name 
        Sanitize the dlog name.    
        Download it from the quad 
"""
Config = {                              \
            'logger'    : None,         \
            'repo'      : '',           \
            'CfgObj'    : None          
         }

def initConfig():
    configfile = SafeConfigParser()
    curdir = os.getcwd() #already set in main
    configPath = os.path.join(curdir, 'config.ini') 
    configfile.read(configPath)
    Config['logger'] = logSetup() 
    Config['CfgObj'] = configfile

def logSetup():
    loggingFormat = '%(asctime)-15s %(message)s' 
    logging.basicConfig(format=loggingFormat, level=logging.DEBUG)
    logger = logging.getLogger('')
    #mode='a'ppend maxBytes=1MB, backupCount=1
    fileHandler = RotatingFileHandler('dlog.log', 'a', 1048576, 1)
    fileHandler.setLevel(logging.DEBUG)
    formatter = logging.Formatter('%(asctime)-15s %(message)s')
    fileHandler.setFormatter(formatter)
    logger.addHandler(fileHandler)
    return logger

class InvalidDlogNameException(Exception):
    def __init__(self, msg):
        Config['logger'].exception(msg)

class DatalogNotFoundException(Exception):
    def __init__(self, msg):
        Config['logger'].exception(msg)

class InstallPathNotFoundException(Exception):
    def __init__(self):
        Config['logger'].exception("The installation path for Datalog Downloader was not found in the %PATH% environment variable")

class DatalogOptions:
    def __init__(self, cfgObj):
		#default download option
        self.download = True
        if Config['CfgObj'].get('Options', 'Download').lower() == 'true':
            self.download = True
        elif Config['CfgObj'].get('Options', 'Download').lower() == 'false':
            self.download = False
        self.downloadPath = cfgObj.get('Options', 'DownloadPath')

        self.launch = None
        if cfgObj.get('Options', 'Launch').lower() == 'true':
            self.launch = True
        if cfgObj.get('Options', 'Launch').lower() == 'false':
            self.launch = False

        self.launchProgram = cfgObj.get('Options','LaunchProgram')

	
class Datalog:
    #configure settings based on dlog name
    def __init__(self, dlog):
        """
        ()              -> denotes a group
        (?P<name>regex) -> names the group
        (?P<id>[TASP])  -> looks for a T,A,S,P,W,C and stores the result so 
                           it can be referenced with 'id' later.
        \d              -> looks for just whole numbers
        {1,5}           -> look for at least 1 decimal, upto a max of 5.
        _               -> look for an '_' after you match the S/N
        re.I            -> ignore case
        re.VERBOSE      -> regex can span multiple lines for readability
        """
        self.dlogOrig = dlog
        p = re.compile(r"""
            (?P<id>[TASPWC])      #Device Prefix
            (?P<sn>\d{1,5})_    #Device S/N
            (?P<date>\d{8,8})_  #Date
            (?P<run_no>\d{1,3}) #Run Number
         """,re.I|re.VERBOSE)
        m = p.search(dlog)
        if m:
            devSN       = int(m.group('sn'))
            devPrefix   = m.group('id').upper()
            if devPrefix == 'T':
                folderName = "%s%d" %(devPrefix, devSN)
            else:
                folderName   = "1%s%05d" %(devPrefix, devSN) 
	    devMap = {'T':'Trima', 'A':'Atreus', 'S':'Optia', 'P':'Optia', 'W':'Massive', 'C':'CES', }
            self.set(devMap[devPrefix], folderName, \
                    devSN, m.group('date'), m.group('run_no'), devPrefix); 
        else:
            msg = "The dlog name %s doesn't make any sense to me!" %(dlog)
            raise InvalidDlogNameException(msg)
            
    def set(self, devString, folderName, devSN, date, run, prefix):
        self.prefix = prefix
        self.deviceString = devString
        self.folderName = folderName
        self.deviceSN = devSN
        self.date = date
        self.run = run
        self.repository = Config['CfgObj'].get(self.deviceString, 'Repository')
        self.launchPath = self.quadPath = self.getQuadPath()

        self.options = DatalogOptions(Config['CfgObj'])

    def getQuadPath(self):
        dlogDir = os.path.join(self.repository, \
                self.folderName, 'dlogs')
        if  not os.path.exists(dlogDir) and (self.deviceString == 'Optia'):
            if self.prefix == 'P':
                self.folderName = "1S%05d" %(self.deviceSN)
            elif self.prefix == 'S':
                self.folderName = "1P%05d" %(self.deviceSN)
            dlogDir = os.path.join(self.repository, \
                     self.folderName, 'dlogs')

        #I'm feeling lucky
        dlogTemp = self.dlogOrig
        if not dlogTemp.endswith('.dlog'):
            dlogTemp = dlogTemp + '.dlog'
        if os.path.exists(dlogDir + "\\" + dlogTemp):
            return dlogDir + "\\" + dlogTemp

        #now create the dlog name
        dlogSearchStr = "1%s%05d_%s_%s*" % (self.prefix,self.deviceSN, self.date, self.run)
        Config['logger'].debug("dlogDir: %s dlogSearchStr: %s" %(dlogDir, dlogSearchStr))
        return locate(dlogSearchStr, dlogDir)
#End class Datalog

#Assume only unique filenames are present
def locate(pattern, root=os.curdir):
    if not root.endswith("\\"):
        files = glob.glob(root + "\\" + pattern)
    else:
        files = glob.glob(root + pattern)
    if len(files) > 0:
        return files[0]
    else:
        msg = "No file in the folder %s matches the pattern %s" %(root, pattern) 
        raise DatalogNotFoundException(msg)

def downloadDlog(remotePath, localPath):
    #localPath = os.path.abspath(localPath)
    head, tail = os.path.split(remotePath)
    localDlogPath = os.path.join(localPath, tail)
    Config['logger'].debug("remotePath: %s. localPath: %s localDlogPath: %s" %(remotePath, localPath, localDlogPath))
    #does file already exist?
    if os.path.exists(localDlogPath):
        return localDlogPath
    #copy 
    try:
        if not os.path.exists(localPath):
            os.makedirs(localPath)
        shutil.copyfile(remotePath, localDlogPath)
    except (IOError, os.error), why:
        Config['logger'].exception( "Can't copy %s to %s: %s" % (`remotePath`, `localDlogPath`, str(why)))
        raise 
    return localDlogPath
   
def launch(cmd, launchPath):
    if cmd: #cmd is neither None nor '' 
        cmd = '%s %s' % (cmd, launchPath)
        os.system(cmd)
    else:
        os.startfile(launchPath)

def main(argv=None):
    try:   
        initConfig()
        objDlog = Datalog(argv[1])
        if objDlog.options.download:
            objDlog.launchPath = downloadDlog(objDlog.quadPath, objDlog.options.downloadPath)
            
        if objDlog.options.launch:
            launch(objDlog.options.launchProgram, objDlog.launchPath)
        return 0
    except DatalogNotFoundException:
        return 1 
    except IOError:
        return 2
    except os.error:
        return 3 
    except InstallPathNotFoundException:
        return 4
    except InvalidDlogNameException:
        return 5
    except Exception, why:
        if why:
            Config['logger'].error(str(why))
        else:
            import traceback
            Config['logger'].error(traceback.format_exc())
        return  -1

def explainReturnCode(retVal):
    import win32ui, win32con
    dlog = sys.argv[1]
    if retVal == 1:
        msg = "Could not locate " + dlog 
    elif retVal == 5:
        msg = "The dlog name \"%s\" doesn't make any sense to me!" %(dlog)
    elif retVal == -1:
        msg = "Unknown Error... The worst kind. E-mail %s to the author" \
                   %s (os.path.dirname(sys.executable) + "\dlog.log")
    win32ui.MessageBox(msg, "Datalog Download", \
        win32con.MB_OK + win32con.MB_ICONEXCLAMATION \
        + win32con.MB_APPLMODAL + win32con.MB_TOPMOST)

def usage():
    print "Usage: dlog.exe filename" 
    sys.exit(0)

if __name__ == '__main__':
    showMessage = False
    if len(sys.argv) == 1:
        import win32clipboard as wc 
        wc.OpenClipboard()
        clipText = wc.GetClipboardData(wc.CF_TEXT)
        wc.CloseClipboard()
        showMessage = True
        if clipText:
            sys.argv.append(clipText)
        else:
            usage()        

    elif len(sys.argv) > 2:
        usage()

    cwd = os.getcwd()
    os.chdir(os.path.dirname(sys.executable))
    retVal = main(sys.argv)
    if showMessage and (retVal != 0):
        explainReturnCode(retVal)

    os.chdir(cwd)
    sys.exit(retVal)
