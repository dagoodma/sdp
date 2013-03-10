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
#sys.path.append('library')
import os
import string
import pprint
import argparse
import ConfigParser
import logging
import signal
import threading
#from threading import Thread
from time import sleep
import Queue
import serial
from serial.tools.list_ports import *
import SerialLogger
from SerialLoggerException import *


# Python 2.x and 3.x tkinter compatability 
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

# ----------------------------------
# Initialize some global variables

DEBUG = False

TITLE = "Serial Logger"
START_STRING = '--------------------- Started ------------------------'
EXIT_STRING  = '---------------------- Exited ------------------------'

# Fallback defaults
DEFAULT_CONFIG_FILE = '.config.cfg'
DEFAULT_LOG_FILE = 'serial_logger.log'
DEFAULT_BAUDRATE = 9600
DEFAULT_TIMEOUT = 6
DEFAULT_LOG_LEVEL = 'INFO'
DEFAULT_LOG_FORMAT = "%(levelname)s %(asctime)-15s %(message)s"
READ_DELAY = 0.0002 # (sec)  delay between serial read updates
UPDATE_DELAY = 5  # (msec) delay between terminal updates

terminal_lock = threading.Lock()

#------------------------------ Gui -------------------------------
class Application():#Frame):              
    def __init__(self, master, queue, endCommand):
        """\
        Contructor for the application.

        """
        self.is_connected = False
        self.pp = pprint.PrettyPrinter(indent=4)
        self.queue = queue
        self.endCommand = endCommand
        #------------------------------------------
        # Parse arguments, read config file, and start logger
        self.parseArguments()
        self.readConfigFile()
        self.startLogger()

        #------------------------------------------
        # Initialize the GUI
        #tkMaster = Frame.__init__(self, tkMaster)
        self.frame = Frame(master)
        self.master = master
        self.master.title(TITLE)
        self.master.iconbitmap(default='compas.ico')
        #self.master.wm_iconbitmap('compas.ico')
        self.frame.pack_propagate(0)
        self.frame.grid( sticky=N+S+E+W)                    
        self.updateAvailablePorts()
        self.createWidgets()
        self.master.protocol('WM_DELETE_WINDOW', self.quit)


    def parseArguments(self):
        """\
        Reads settings from the config file.

        """
        # ----------------------------------
        # Load default settings
        self.configfile = DEFAULT_CONFIG_FILE
        self.baudrate = DEFAULT_BAUDRATE
        self.logfile = DEFAULT_LOG_FILE
        self.timeout = DEFAULT_TIMEOUT
        self.loglevel = DEFAULT_LOG_LEVEL
        self.device = False

        # ----------------------------------
        # Parse arguments
        args_raw = ''
        if (len(sys.argv) > 1):
            args_raw = string.join(sys.argv)
        self.args_obj = None

        parser = argparse.ArgumentParser(
            formatter_class=argparse.RawDescriptionHelpFormatter,
            description="""\
Connects to a serial device and sends and receives data.
""",
            usage='serial_logger_gui.py -h | [-l log_file] [--loglevel=LEVEL] [-c config_file] [-t timeout] [-b baud_rate] [device_path] ',
            add_help=False
            )
        parser.add_argument('device', nargs='?',
            help='device path or id of the serial port')
        parser.add_argument('-b', '--baud', dest='baud_rate', nargs=1, type=int,
            help='baud rate for the serial connection (default: {0!s})'.format(self.baudrate))
        parser.add_argument('-c','--config', dest='config_file', nargs=1,
            help='config file with default values (default: {0})'.format(self.configfile))
        parser.add_argument('-t', '--timeout', dest='timeout', nargs=1, type=int,
            help='serial connection timeout in seconds (default: {0!s})'.format(self.timeout))
        parser.add_argument('-l','--log', dest='log_file', nargs=1,
            help='log file to record session to (default: {0})'.format(self.logfile))
        parser.add_argument('--loglevel', dest='log_level', action='store', default=self.loglevel,
            help='sets the logging level (default: %(default)s)', choices=['DEBUG', 'INFO', 'WARNING', 'ERROR', 'CRITICAL'])
        parser.add_argument('-h', '--help', action='store_true', dest='want_help',
            help='show this help message and exit')

        # Actually parse the arguments given
        try:
            self.args_obj = parser.parse_args()
            #pp.pprint(args_obj)
            if (self.args_obj.want_help):
                parser.print_help()
                self.quit()
            if (self.args_obj.config_file):
                self.configfile = self.args_obj.config_file
            if (self.args_obj.log_file):
                self.logfile = self.args_obj.log_file
            if (self.args_obj.baud_rate):
                self.baudrate = self.args_obj.baud_rate
            if (self.args_obj.device):
                self.device = self.args_obj.device
            if (self.args_obj.timeout):
                self.timeout = self.args_obj.timeout

        except Exception, ex:
            # Handle exception here
            raise
            self.quit()
            

    def readConfigFile(self):
        """\
        Reads settings from the config file.

        """
        self.config_obj = ConfigParser.ConfigParser()
        self.config_obj.readfp(open(self.configfile))

        # Set the log file
        if (not self.args_obj.log_file and self.config_obj.has_option('DEFAULT','logfile')):
            self.logfile = self.config_obj.get('DEFAULT', 'logfile')

        # Set the baud rate
        if (not self.args_obj.baud_rate and self.config_obj.has_option('DEFAULT','baud')):
            self.baudrate = self.config_obj.get('DEFAULT', 'baud')

        # Set the device port 
        if (not self.args_obj.device and self.config_obj.has_option('DEFAULT','device')):
            self.device = self.config_obj.get('DEFAULT', 'device')

        # Set the connection timeout
        if (not self.args_obj.timeout and self.config_obj.has_option('DEFAULT','timeout')):
            self.timeout = self.config_obj.get('DEFAULT','timeout')

        if DEBUG:
            print('(DEBUG) Config Options:')
            self.pp.pprint(self.config_obj.sections())


    def startLogger(self):
        """\
        Starts the logger.
        
        """
        #------------------------------------------
        # Initialize logger
        log_level = getattr(logging, str(self.loglevel).upper())
        logging.basicConfig(filename=self.logfile,level=log_level, format=DEFAULT_LOG_FORMAT)
        logging.info(START_STRING)
   

    def createWidgets(self):
        """\
        Creates the gui and all of its content.

        """
        # create top menus
        self.menu= Menu(self.master)
        self.master.config(menu=self.menu)
        self.file_menu = Menu(self.menu)
        self.menu.add_cascade(label="File", menu=self.file_menu)
        self.file_menu.add_command(label="Quit", command=self.quit)

        self.edit_menu = Menu(self.menu)
        self.edit_opts_menu = Menu(self.edit_menu)
        self.menu.add_cascade(label="Edit", menu=self.edit_menu)
        self.edit_menu.add_cascade(label="Options", menu=self.edit_opts_menu)
        self.edit_menu.add_command(label="Clear Terminal", command=lambda: self.terminal.delete(1.0,END))

        # Options
        self.autoscroll_value = BooleanVar()
        self.edit_opts_menu.add_checkbutton(label="Autoscroll", onvalue=True, offvalue=False, variable=self.autoscroll_value)


        #----------------------------------------
        # Create the Device entry

        self.device_value = StringVar()
        self.device_value.set(self.device)
        self.device_label = Label( self.master, text="Port:" )
        self.device_label.grid(row=0, column = 0,sticky=E)
        self.device_menu = OptionMenu( self.master,  self.device_value, *self.device_choices) 
        self.device_menu.config(width=40)
        self.device_menu.grid(row=0, column = 1)

        #----------------------------------------
        # Create the Baud rate entry

        self.baudrate_value = IntVar()
        self.baudrate_value.set(self.baudrate) # loaded from default, args, or config
        self.baudrate_choices = [ 9600, 14400, 19200, 28800, 38400, 57600, 102400, 115200, 128000, 230400, 256000, 460800, 512000, 921600, 1843200, 2048000 ]
        self.baudrate_label = Label( self.master, text="Baud rate:" )
        self.baudrate_label.grid(row=0, column = 2, sticky=E)
        self.baudrate_menu  = OptionMenu( self.master,  self.baudrate_value, *self.baudrate_choices)
        self.baudrate_menu.config(width=10)
        self.baudrate_menu.grid(row=0, column = 3)

        #----------------------------------------
        # Create the Log file entry

        self.log_value = StringVar()
        self.log_value.set(self.logfile)
        self.log_label = Label( self.master, text="Log file:" )
        self.log_label.grid(row=1,column = 0, sticky=E)
        self.log_entry = Entry( self.master, width = 46, textvariable=self.log_value )
        self.log_entry.grid(row=1, column = 1)
        self.log_button = Button (self.master, text="Browse", command=self.browseLogFile)
        self.log_button.grid(row=1, column = 2, sticky=W)

        #----------------------------------------
        # Create the connect/disconnect button

        self.connect_button = Button ( self.master, text="Connect", command=self.connect,width=12)
        self.connect_button.grid(row=1,column=3)

        #----------------------------------------
        # Create the terminal window

        self.terminal = Text( self.master, width = 65, background='black', foreground='white' )
        self.terminal.grid(row=2, column = 0, columnspan=4, sticky=E+W)

        # scroll bar
        self.terminal_scroller = AutoScrollbar(self.master, command=self.terminal.yview)
        self.terminal_scroller.grid(row=2,column=4, sticky=N+S)
        self.terminal.config(yscrollcommand=self.terminal_scroller.set)
        self.terminal_scroller_lastpos = (0.0, 1.0)
        self.autoscroll_value.set(True)



    def do_terminal(self):
        """\
        Reads a byte from the serial connection
        
        Note:
        - Beware, is_connected cannot be shared without a lock!
        """
        if (self.is_connected):
            self.mySerialConnection.do_serial()

    
    def printTerminal(self,message):
        """\
        Callback function for printing to the terminal window, which queues
        the message from the worker thread to the gui thread.
        """
        self.queue.put(message)


    def processIncoming(self):
        """\
        Handle all the messages in the queue by printing them in the terminal.
        """
        while (self.queue.qsize()):
            try:
                message = self.queue.get_nowait()
            
                self.terminal.insert(END,message)

                # Autoscroll the terminal if set
                if (self.autoscroll_value.get()):
                    self.terminal.yview(END)

            except Queue.Empty:
                pass


    def browseLogFile(self):
        """\
        Open a file selection window to choose the XML message definition.
        """
        log_file = tkinter.filedialog.askopenfilename(parent=self.master, title='Choose a log file')
        if DEBUG:
            print("Log: " + log_file)
        if log_file and log_file != '':
            self.log_value.set(log_file)

    def connect(self):
        """\
        Opens a connection to a serial device over the selected port.

        """
        logging.debug('Initializing SerialLogger')

        # Initial new terminal object
        self.mySerialConnection = SerialLogger.SerialLogger(
            baud_rate = self.baudrate_value.get(),
            device_port = self.device_value.get(),
            timeout = self.timeout,
            interactive = False,
            print_callback = self.printTerminal
        )

        self.is_connected = True
        self.connect_button.config(command=self.disconnect);
        self.connect_button.config(text="Disconnect")
        pass

    def disconnect(self):
        """\
        Closes a connection to a serial device.
        Deconstructor for Application.

        """
        self.is_connected = False
        self.mySerialConnection = None
        self.connect_button.config(command=self.connect);
        self.connect_button.config(text="Connect")

    def quit(self):
        """\
        Deconstructor for Application.

        """
        self.disconnect()
        mySerialConnection = None
        logging.info(EXIT_STRING)
        self.frame.destroy()
        self.endCommand()
        #sys.exit()



    def updateAvailablePorts(self):
        """\
        Lists the available serial ports.

        """
        # Build a port list
        device_list_all = comports()
        self.device_choices = list()
        for device in device_list_all:
            self.device_choices.append(device[0])

        if len(self.device_choices) < 1:
            tkinter.messagebox.showerror('No Available Serial Ports','No serial ports are available.')


class AutoScrollbar(Scrollbar):
    """\
    Extends the Tkinter scrollbar to remember previous scroll positions,
    which is used in the autoscroll feautre.
    """

    def __init__(self, *args, **kw):
        self.old_x = 0.0
        self.old_y = 1.0
        self.supa = Scrollbar.__init__(self,*args, **kw)
        #self.autoscroll = True


    def set(self, lo, hi):
        pos = Scrollbar.get(self)
        self.old_x = float(pos[0])
        self.old_y = float(pos[1])
        Scrollbar.set(self, lo, hi)

    def getY(self):
        return float(self.old_y)

    def autoscroll(self):
        """\
        Auto scroll to the bottom if desired.
        """
        return self.getY() == float(1.0)
        #return self.autoscroll

# End of Application class 
# ---------------------------------

# --------------- Threaded Client ------------------
class ThreadedClient:
    """\
    This threaded client reads bytes from the serial device and sends
    them to the Gui as queues requests.

    """
    def __init__(self, master):
        self.master = master
        self.queue = Queue.Queue()
        self.gui = Application(master, self.queue, self.endApplication)

        self.running = 1
        self.thread1 = threading.Thread(target=self.workerThread1)
        self.thread1.start()

        self.periodicCall()

    def periodicCall(self):
        """\
        Updates the gui's terminal window by pulling messages off the queue.

        Note:
        - Consider moving instantiation of Application into this function.
        - Rename: doGui()
        """
        self.gui.processIncoming()
        if not self.running:
            import sys
            sys.exit(1)
        self.master.after(UPDATE_DELAY, self.periodicCall)

    def workerThread1(self):
        """\
        Reads bytes from the serial device, where they're sent to the Gui as messages.
        """
        while self.running:
            sleep(READ_DELAY)

            self.gui.do_terminal() 

            #self.queue.put(self.gui.readSerialByte) # this didn't
            #self.gui.readSerialByte() # this works

    def endApplication(self):
        """\
        Causes the terminal update thread to quit.

        """
        self.running = 0

"""-------------------------------------------------------------
                              Start
   -------------------------------------------------------------"""

if __name__ == '__main__':
    root = Tk()
    client = ThreadedClient(root)
    root.mainloop()

