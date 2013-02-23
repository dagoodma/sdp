from distutils.core import setup
import py2exe, sys, os
opts = {
    "py2exe": {
        "includes": "serial",
        "dist_dir": "bin",
    }
}

setup(
    windows= [
        {
            "script": 'serial_logger_gui.py',
            "icon_resources": [(1, "compas.ico")]
        }
    ],
    options = opts
)


