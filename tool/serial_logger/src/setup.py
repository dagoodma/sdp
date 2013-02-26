from distutils.core import setup
import py2exe, sys, os
import glob

iconFilename = "compas.ico"

include = [ 
        'serial', 'Tkinter', 'os', 'sys', 'tkFileDialog', 'tkMessageBox',
        'string', 'pprint', 'argparse', 'ConfigParser', 'logging', 
        'signal', 'threading', 'time', 'Queue', 'SerialLogger', 
        'SerialLoggerException', 'serial.tools.list_ports',
]

dll_exclude = [
    'msvcr71.dll'
]

opts = {
    "py2exe": {
        #"ascii": True,
        "includes": include,
        "dist_dir": "bin",
        "dll_excludes": dll_exclude,
        #bundle_files": 1,
        #"compressed": True,
    }
}

data_files=[('.', glob.glob(iconFilename))]
#data_files=[('.', glob.globiconFilename)]

setup(
    version='1.0',
    description='Serial port terminal with logging',
    author='David Goodman',
    windows= [
        {
            "script": 'serial_logger_gui.py',
            "icon_resources": [(1, iconFilename)],
        }
    ],
    options = opts,
    data_files=data_files,
    #zipfile = None,
)


