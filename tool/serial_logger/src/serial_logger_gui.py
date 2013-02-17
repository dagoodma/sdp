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
sys.path.append('library')
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

# ----------------------------------
# Initialize some global variables

DEBUG = True

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
READ_DELAY = 0.001# (sec)

terminal_lock = threading.Lock()

#------------------------------ Gui -------------------------------
class Application():#Frame):              
    def __init__(self, master, queue, endCommand):
        """\
        Contructor for the application.

        """
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
        self.frame.pack_propagate(0)
        self.frame.grid( sticky=N+S+E+W)                    
        self.updateAvailablePorts()
        self.createWidgets()
        self.master.protocol('WM_DELETE_WINDOW', self.quit)


        #-
        # Serial print loop
        self.is_connected = False
        self.readSerialByte()


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
            

    def processIncoming(self):
        """\
        Handle all the messages in the queue.
        """
        while (self.queue.qsize()):
            try:
                callable= self.queue.get() # get_nowait
                #callable, args, kwargs = self.queue.get() # get_nowait
            except Queue.Empty:
                pass
            else:
                callable()



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
        #----------------------------------------
        # Create the Device entry

        self.device_value = StringVar()
        self.device_label = Label( self.master, text="Port:" )
        self.device_label.grid(row=0, column = 0,sticky=E)
        #self.device_menu = Listbox(self.master, height=1, width=40)
        self.device_menu = OptionMenu( self.master,  self.device_value, *self.device_choices) 
        self.device_menu.config(width=40)
        self.device_value.set(self.device_choices[0])
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
        #self.baudrate_button = Button (self.master, text="<", command=self.chooseBaudrateValue)
        #self.baurdrate_button.grid(row=0, column = 2)

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

        self.terminal = Text( self.master, width = 65 )
        self.terminal.grid(row=2, column = 0, columnspan=4)

        # scroll bar
        self.terminal_scroller = AutoScrollbar(self.master, command=self.terminal.yview)
        self.terminal_scroller.grid(row=2,column=4, sticky=N+S)
        self.terminal.config(yscrollcommand=self.terminal_scroller.set)
        self.terminal_scroller_lastpos = (0.0, 1.0)
        #self.terminal_scroller_command = self.terminal_scroller.command
        self.pp.pprint(getattr(self.terminal_scroller,'_tclCommands'))
        

    def readSerialByte(self):
        if (self.is_connected):
            self.mySerialConnection.do_terminal()


    def printTerminal(self,message):
        # Blocking
        while not terminal_lock.acquire(False):
            pass
        try:
            self.terminal.insert(END,message)

            if (self.terminal_scroller.getY() == float(1.0)):
                self.terminal.yview(END)

        finally:
            terminal_lock.release()
       

    """\
    Open a file selection window to choose the XML message definition.
    """
    def browseLogFile(self):
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

    def __init__(self, *args, **kw):
        self.old_x = 0.0
        self.old_y = 1.0
        self.supa = Scrollbar.__init__(self,*args, **kw)


    def set(self, lo, hi):
        pos = Scrollbar.get(self)
        self.old_x = float(pos[0])
        self.old_y = float(pos[1])
        Scrollbar.set(self, lo, hi)

    def getY(self):
        return self.old_y

# End of Application class 
# ---------------------------------

class ThreadedClient:

    def __init__(self, master):
        self.master = master
        self.queue = Queue.Queue()
        self.gui = Application(master, self.queue, self.endApplication)

        self.running = 1
        self.thread1 = threading.Thread(target=self.workerThread1)
        self.thread1.start()

        self.periodicCall()

    def periodicCall(self):
        self.gui.processIncoming()
        if not self.running:
            import sys
            sys.exit(1)
        self.master.after(10, self.periodicCall)

    def workerThread1(self):
        while self.running:
            sleep(READ_DELAY)
            #self.queue.put(self.gui.readSerialByte)
            self.gui.readSerialByte()

    def endApplication(self):
        self.running = 0

#----------------------------------------------------------
def submit_to_application(callable, *args, **kwargs):
    #request_queue.put((callable, args, kwargs))
    return 'faq'
    #return result_queue.get()

def handle_uncaught_exception(ex_type, ex, tb):
    message1 = ''.join(traceback.format_tb(tb))
    logging.critical(message1)
    print(message1)
    message2 = '{0}: {1}'.format(exception_type, exception)
    logging.critical(message2)
    print(message2)

"""-------------------------------------------------------------
                              Start
   -------------------------------------------------------------"""
"""
appThread = None
app = None
#app = None;
def threadmain():
    global app
    app = Application()                    
    app.master.title(TITLE) 
    timertick()
    app.mainloop()  



if __name__ == '__main__':
    client = ThreadedClient(    
    try:
        
        appThread = Thread(target=threadmain, args=())
        #appThread.setDaemon(True)
        appThread.start()

    except Exception as err:
        print err

    while 1: #(appThread.is_alive()):
        #sleep(READ_DELAY)
        print submit_to_application(app.readSerialByte, "")
"""
if __name__ == '__main__':
    root = Tk()
    client = ThreadedClient(root)
    root.mainloop()

