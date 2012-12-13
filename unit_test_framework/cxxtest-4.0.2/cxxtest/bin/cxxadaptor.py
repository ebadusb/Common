
# Use command line argument RunnerName with this script to generate the following string and execute it as a system command:
#
#   python cxxtestgen.py --xunit-printer -w RunnerName --xunit-file="RunnerName Results.xml" -o RunnerName.cpp RunnerName.h
#   (This will generate a test runner from a single header file with the same name)
#
# Include optional argument [--headers] to generate the following string and execute it as a system command:
#
#   python cxxtestgen.py --xunit-printer -w RunnerName --xunit-file="RunnerName Results.xml" -o RunnerName.cpp --headers=RunnerName.lst
#   (This will generate a test runner from potentially multiple header files included in a .lst file with the same name)


if __name__ == "__main__":

    import sys
    import os
    import subprocess
    from optparse import OptionParser

    parser = OptionParser("%prog [options] RunnerName")
    parser.add_option("--headers", action="store_true", dest="headers", default=False, help="File name of listed header files")
    options, args = parser.parse_args()

    cxxtestgen = ""
    if args:
        name = args[0]

        if options.headers and os.path.isfile(name + ".lst"):
            cxxtestgen = "python " + os.path.join(os.path.dirname(sys.argv[0]), "cxxtestgen.py") + " --xunit-printer -w \"" + \
                           name + "\" --xunit-file=" + "\"" + name + " Results.xml\" -o \"" + name + ".cpp\" " + \
                            "--headers=\"" + name + ".lst\""

        elif os.path.isfile(name + ".h"):
            cxxtestgen = "python " + os.path.join(os.path.dirname(sys.argv[0]),"cxxtestgen.py") + " --xunit-printer -w \"" + \
                           name + "\" --xunit-file=" + "\"" + name + " Results.xml\" -o \"" + name + ".cpp\" " + \
                            "\"" + name + ".h\""

    if cxxtestgen:

        print cxxtestgen
        output = subprocess.Popen(cxxtestgen, stdout=subprocess.PIPE, stderr=subprocess.STDOUT).stdout
        for line in output: print(line)

    else:
        parser.print_help()
