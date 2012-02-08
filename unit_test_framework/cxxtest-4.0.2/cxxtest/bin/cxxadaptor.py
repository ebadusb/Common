# Use a single command line argument (-n INPUT) to generate the following CxxTestGen.py string
# and execute it as a system command:
#
#     cxxtestgen.py --xunit-printer -w "INPUT" --xunit-file "INPUT Results.xml" -o "INPUT.cpp" "INPUT.h"

import sys
import os
import subprocess
from optparse import OptionParser



parser = OptionParser("%prog [options] [<filename> ...]")
parser.add_option("-n", "--name",
                  action="store", type="string", dest="name",
                  help="Name of VC++ Project")
options, args = parser.parse_args(sys.argv)
cxxtestgen = "python "+os.environ["CXXTEST"] +"\\bin\\cxxtestgen.py --xunit-printer -w \"<INPUT>\" --xunit-file \"<INPUT> Results.xml\" -o \"<INPUT>.cpp\" \"<INPUT>.h\""
cxxtestgen = cxxtestgen.replace("<INPUT>", options.name);
output = subprocess.Popen(cxxtestgen, stdout=subprocess.PIPE, stderr=subprocess.STDOUT).stdout
for line in output: print(line)
   