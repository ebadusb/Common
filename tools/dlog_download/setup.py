from distutils.core import setup
import py2exe
import os, sys
import glob

source = os.getcwd() + os.sep
my_data_files = [ ('', [ source + 'DlogDownload.htm']),
                    ('', [ source + 'download.ico']),
                    ('', [source + 'config.ini']),
                    ('', [source + 'dlog.py']),
                    ('', [source + 'setup.py']),
                    ('', [source + 'setup.nsi']),
                    ('', [source + 'vcredist_x86.exe'])
                ]

setup(console=['dlog.py'], \
      data_files = my_data_files )
