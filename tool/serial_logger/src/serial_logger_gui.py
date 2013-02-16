#!/usr/bin/env python
"""\
serial_logger_gui.py is a tool for reading data from a connected serial device
with a gui.

Author: David Goodman (dagoodma@ucsc.edu)

Notes:
-----
* 2013-02-15 -- dagoodma
    Created this gui.

"""
import sys
import os
sys.path.append('library')
import SerialLogger
import string
import pprint
import argparse
import ConfigParser
# import textwrap
# import tree # RB Tree
import logging
import signal
import serial
# for creating our own port list
from SerialLoggerException import *
from serial.tools.list_ports import *


# Python 2.x and 3.x compatability 
try:
    from tkinter import *
    import tkinter.filedialog
    import tkinter.messagebox
    #import tkinter.listbox
except ImportError as ex:
    # Must be using Python 2.x, import and rename
    from Tkinter import *
    import tkFileDialog
    import tkMessageBox

    tkinter.filedialog = tkFileDialog
    del tkFileDialog
    tkinter.messagebox = tkMessageBox
    del tkMessageBox

DEBUG = False
title = "Serial Logger"
START_STRING = '--------------------- Started ------------------------'
EXIT_STRING  = '---------------------- Exited ------------------------'
myTerminalLogger = None

# Catches Ctrl-C
def signal_handler(signal, frame):
    if DEBUG:
        sys.stdout.write('\n\nYou pressed Ctrl-C.')

    cleanup()
    sys.exit(0)

# Clean up for serial port and log when exiting.
def cleanup():
    try:
        myTerminalLogger.__del__()
        myTerminalLogger = None
        #if not myTerminalLogger is None:
        #    del myTerminalLogger

    except NameError as ex:
        # Don't handle
        pass
    except Exception as ex:
        logging.exception(ex)
        # Do nothing
    try:
        logging.info(EXIT_STRING)
    except NameError as ex:
        pass

"""
class OptionMenu2(OptionMenu):
    def __init__(self, *args, **kw):
        self._command = kw.get("command")
        OptionMenu.__init__(self, *args, **kw)
    def addOption(self, label):
        self["menu"].add_command(label=label, command=tkinter._setit(variable, label, self._command))

"""

#------------------------------ Gui -------------------------------
class Application(Frame):              
    def __init__(self, master=None):
        Frame.__init__(self, master)
        self.pack_propagate(0)
        self.grid( sticky=N+S+E+W)                    
        self.updateAvailablePorts()
        self.createWidgets()
        self.pp = pprint.PrettyPrinter(indent=4)


        # Start the SerialLogger

    """\
    Creates the gui and all of its content.
    """
    def createWidgets(self):


        #----------------------------------------
        # Create the Port entry

        self.port_value = StringVar()
        self.port_label = Label( self, text="Port: " )
        self.port_label.grid(row=0, column = 0)
        #self.port_menu = Listbox(self, height=1, width=40)
        self.port_menu = OptionMenu( self,  self.port_value, *self.port_choices) 
            #w idth = 40, textvariable=self.port_value )
        self.port_menu.config(width=40)
        self.port_value.set(self.port_choices[0])
        self.port_menu.grid(row=0, column = 1)
        # self.port_button = Button (self, text="<", command=self.choosePortValue)
        # self.port_button.grid(row=0, column = 5)


        #----------------------------------------
        # Create the Baud rate entry

        self.baudrate_value = IntVar()
        self.baudrate_choices = [ 9600, 14400, 19200, 28800, 38400, 57600, 102400, 115200, 128000, 230400, 256000, 460800, 512000, 921600, 1843200, 2048000 ]
        self.baudrate_label = Label( self, text="Baud rate: " )
        self.baudrate_label.grid(row=0, column = 2)
        self.baudrate_menu  = OptionMenu( self,  self.baudrate_value, *self.baudrate_choices)
            # width = 10, textvariable=self.baudrate_value )
        self.baudrate_menu.config(width=10)
        self.baudrate_value.set(self.baudrate_choices[0])
        self.baudrate_menu.grid(row=0, column = 3)
        #self.baudrate_button = Button (self, text="<", command=self.chooseBaudrateValue)
        #self.baurdrate_button.grid(row=0, column = 2)

        #----------------------------------------
        # Create the Log file entry

        self.log_value = StringVar()
        self.log_label = Label( self, text="Out" )
        self.log_label.grid(row=1,column = 0)
        self.log_entry = Entry( self, width = 40, textvariable=self.log_value )
        self.log_entry.grid(row=1, column = 1)
        self.log_button = Button (self, text="Browse", command=self.browseLogFile)
        self.log_button.grid(row=1, column = 2)

        #----------------------------------------
        # Create the Lang box
        """

        self.language_value = StringVar()
        self.language_choices = [ "C", "Python" ]
        self.language_label = Label( self, text="Lang" )
        self.language_label.grid(row=2, column=0)
        self.language_menu = OptionMenu(self,self.language_value,*self.language_choices)
        self.language_value.set(self.language_choices[0])
        self.language_menu.config(width=10)
        self.language_menu.grid(row=2, column=1,sticky=W)

        #----------------------------------------
        # Create the Protocol box
        self.protocol_value = StringVar()
        self.protocol_choices = [ "v0.9", "v1.0" ]
        self.protocol_label = Label( self, text="Protocol")
        self.protocol_label.grid(row=3, column=0)
        self.protocol_menu = OptionMenu(self,self.protocol_value,*self.protocol_choices)
        self.protocol_value.set(self.protocol_choices[1])
        self.protocol_menu.config(width=10)
        self.protocol_menu.grid(row=3, column=1,sticky=W)
        #----------------------------------------
        # Create the generate button

        self.generate_button = Button ( self, text="Generate", command=self.generateHeaders)
        self.generate_button.grid(row=4,column=1)
        """
        #----------------------------------------
        # Create the connect/disconnect button

        self.connect_button = Button ( self, text="Connect", command=self.connect,width=9)
        self.connect_button.grid(row=1,column=3)

        #----------------------------------------
        # Create the connect/disconnect button

        #self.terminal = Text( self, width = 40 )
        #self.terminal.grid(row=0, column = 0)

    """\
    Open a file selection window to choose the XML message definition.
    """
    def browseLogFile(self):
        log_file = tkinter.filedialog.askopenfilename(parent=self, title='Choose a log file')
        if DEBUG:
            print("Log: " + log_file)
        if log_file != None:
            self.log_value.set(log_file)

    """\
    Open a directory selection window to choose an output directory for
    headers.
    ""
    def browseOutDirectory(self):
        mavlinkFolder = os.path.dirname(os.path.realpath(__file__))
        out_dir = tkinter.filedialog.askdirectory(parent=self,initialdir=mavlinkFolder,title='Please select an output directory')
        if DEBUG:
            print("Output: " + out_dir)
        if out_dir != None:
            self.out_value.set(out_dir)
    """

    def connect(self):
        self.connect_button.config(command=self.disconnect);
        self.connect_button.config(text="Disconnect")
        pass

    def disconnect(self):
        self.connect_button.config(command=self.connect);
        self.connect_button.config(text="Connect")
        pass


    def updateAvailablePorts(self):
        """\
        Lists the available serial ports.

        """
        # Build a port list
        port_list_all = comports()
        #self.port_menu.delete(0,self.port_menu.size())
        #self.port_menu.option_clear()
        self.port_choices = list()
        for device in port_list_all:
            self.port_choices.append(device[0])
            #self.port_menu.addOption(device[0])

        #self.port_menu.insert(0,*self.port_choices)
        
        if len(self.port_choices) < 1:
            tkinter.messagebox.showerror('No Available Serial Ports','No serial ports are available.')
       


    """\
    Generates the header files and place them in the output directory.
    ""
    def generateHeaders(self):
        # Verify settings
        rex = re.compile(".*\\.xml$", re.IGNORECASE)
        if not self.xml_value.get():
            tkinter.messagebox.showerror('Error Generating Headers','An XML message defintion file must be specified.')
            return

        if not self.out_value.get():
            tkinter.messagebox.showerror('Error Generating Headers', 'An output directory must be specified.')
            return


        if os.path.isdir(self.out_value.get()):
            if not tkinter.messagebox.askokcancel('Overwrite Headers?','The output directory \'{0}\' already exists. Headers may be overwritten if they already exist.'.format(self.out_value.get())):
                return

        # Verify XML file with schema (or do this in mavgen)
        # TODO write XML schema (XDS)

        # Generate headers
        opts = MavgenOptions(self.language_value.get(), self.protocol_value.get()[1:], self.out_value.get());
        args = [self.xml_value.get()]
        if DEBUG:
            print("Generating headers")
            self.pp.pprint(opts)
            self.pp.pprint(args)
        try:
            mavgen(opts,args)
            tkinter.messagebox.showinfo('Successfully Generated Headers', 'Headers generated succesfully.')

        except Exception as ex:
            if DEBUG:
                print('An occurred while generating headers:\n\t{0!s}'.format(ex))
            tkinter.messagebox.showerror('Error Generating Headers','An error occurred in mavgen: {0!s}'.format(ex))
            return
"""

# End of Application class 
# ---------------------------------

"""-------------------------------------------------------------
                              Start
   -------------------------------------------------------------"""

"""
# ----------------------------------
# Initialize some variables
args_raw = string.join(sys.argv)
args_obj = None
pp = pprint.PrettyPrinter(indent=4)

# Fallback defaults
config_file = '.config.cfg'
log_file = 'serial_logger.log'
log_from_config = True
baud_rate = 9600
device_port = None
timeout = 5
log_level='INFO'
interactive = True



# ----------------------------------
# Parse arguments

parser = argparse.ArgumentParser(
    formatter_class=argparse.RawDescriptionHelpFormatter,
    description=""\
Connects to a serial device and sends and receives data.
"",
    usage='serial_logger_gui.py -h | [-l log_file] [--loglevel=LEVEL] [-c config_file] [-t timeout] [-b baud_rate] [device_path] ',
    add_help=False
    )
parser.add_argument('device', nargs='?',
    help='device path or id of the serial port')
parser.add_argument('-b', '--baud', dest='baud_rate', nargs=1, type=int,
    help='baud rate for the serial connection (default: {0!s})'.format(baud_rate))
parser.add_argument('-c','--config', dest='config_file', nargs=1,
    help='config file with default values (default: {0})'.format(config_file))
parser.add_argument('-t', '--timeout', dest='timeout', nargs=1, type=int,
    help='serial connection timeout in seconds (default: {0!s})'.format(timeout))
parser.add_argument('-l','--log', dest='log_file', nargs=1,
    help='log file to record session to (default: {0})'.format(log_file))
parser.add_argument('--loglevel', dest='log_level', action='store', default=log_level,
    help='sets the logging level (default: %(default)s)', choices=['DEBUG', 'INFO', 'WARNING', 'ERROR', 'CRITICAL'])
parser.add_argument('-h', '--help', action='store_true', dest='want_help',
    help='show this help message and exit')


# Actually parse the arguments given
try:
    args_obj = parser.parse_args()
    #pp.pprint(args_obj)
    if (args_obj.want_help):
        parser.print_help()
        sys.exit()
    if (args_obj.config_file):
        config_file = args_obj.config_file
    if (args_obj.log_file):
        log_file = args_obj.log_file
        log_from_config = False
except Exception, ex:
    raise
    sys.exit()


# -------------------------------
# Read config file

config_obj = ConfigParser.ConfigParser()
config_obj.readfp(open(config_file))

if log_from_config :
    log_file = config_obj.get('DEFAULT', 'logfile')

# Set the baud rate
if args_obj.baud_rate:
    baud_rate = args_obj.baud_rate
elif config_obj.has_option('DEFAULT','baud'):
    baud_rate = config_obj.get('DEFAULT', 'baud')

# Set the device port 
if args_obj.device:
    device_port = args_obj.device
elif config_obj.has_option('DEFAULT','device'):
    device_port = config_obj.get('DEFAULT', 'device')

# Set the connection timeout
if args_obj.timeout:
    timeout = args_obj.timeout
elif config_obj.has_option('DEFAULT', 'timeout'):
    timeout = config_obj.get('DEFAULT','timeout')

# --------------------------------
# Initialize logger
log_level = getattr(logging, str(args_obj.log_level).upper())
FORMAT = "%(levelname)s %(asctime)-15s %(message)s"
logging.basicConfig(filename=log_file,level=log_level, format=FORMAT)
logging.info(START_STRING)

if DEBUG:
    print('(DEBUG) Config Options:')
    pp.pprint(config_obj.sections())

# --------------------------------
# Create and initialize SerialLogger object
try:
    logging.debug('Initializing SerialLogger')

    # Override Ctrl-C with definition
    signal.signal(signal.SIGINT, signal_handler)

    # Initial new terminal object
    myTerminalLogger = SerialLogger.SerialLogger(
        baud_rate = baud_rate,
        device_port = device_port,
        timeout = timeout,
        interactive = interactive
    )
# Start the SerialLogger
    interactive_str = 'interactive'
    if not interactive:
        interactive_str = 'non-interactive'
    logging.debug('Starting SerialLogger in {0} mode'.format(interactive_str))
    if interactive:
        myTerminalLogger.start_terminal()
except Exception, ex:
    logging.exception('SerialLogger failed: {0!s}'.format(ex))
    cleanup()
    raise

"""


#----------------------------------------------------------


app = Application()                    
app.master.title(title) 
app.mainloop()  

cleanup()

