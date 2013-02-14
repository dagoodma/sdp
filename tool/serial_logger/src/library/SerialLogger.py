# Module SerialLogger
import os
import serial
import pprint
import logging
from SerialLoggerException import *
from serial.tools.list_ports import *

   
class SerialLogger:
    """\
    SerialLogger connects to the specified serial port. If none is
    given then the user is prompted for one.

    Args:
        baud_rate: number for serial connection's baud rate
        device_port: path or id of the serial device's port
        timeout: number for the serial connection's timeout in seconds
    Returns: 
        an SerialLogger object
    Raises:
        
       
    """
    def __init__(self, baud_rate, device_port, timeout, interactive=True):
        self.greeting = "\nWelcome to the serial_logger tool."

        self.goodbye = "\nGoodbye!"

        self.prompt = '> '
        self.want_exit = False
        self.is_connected = False
        self.connection = None
        self.baud_rate = baud_rate
        self.device_port = device_port
        self.interactive = interactive
        self.last_id = 0

        # --------------------------------
        # Initialize logger
        FORMAT = "(message)s"
        logging.basicConfig(format=FORMAT, propogate=True)


        try:
            self.timeout = int(timeout)
        except ValueError, TypeError:
            self.timeout = 5
    

        # Ensure port and baud rate are supplied if we can't query 
        if not self.interactive:
            if not self.device_port:
                raise MissingArgumentException('device')
            if not self.baud_rate:
                raise MissingArgumentException('baud')
        else:
            self.ask_connect()


        if self.want_exit:
            return


        self.connect()
        

    
    def ask_connect(self):
        """\
        Lists available serial devices and queries the user for a
        valid serial device and baud rate. Loops until valid parameters
        are supplied.

        """

        print(self.greeting)

        # Query the user for a serial port
        self.validate_port() # clears the port if it does not exist
        if not self.device_port:
            self.list_ports()

            if self.want_exit:
                logging.debug('No available ports to connect to.')
                return

            while not self.device_port:
                self.device_port = self.ask_port()
                self.validate_port()

        # Query the user for a baud rate
        self.validate_baud()
        while not self.baud_rate:
            self.baud_rate = self.ask_baud()
            self.validate_baud()



    def connect(self):
        """\
        Connects to a serial device.

        """
        logging.debug('Connecting to \'{0}\' at {1!s} baud'.format(self.device_port, self.baud_rate))
        try:
            self.connection = serial.Serial(port = self.device_port, baudrate = self.baud_rate, timeout=self.timeout, rtscts = False, dsrdtr = False)
        except (ValueError, SerialException) as ex:
            message = 'Failed to connect to \'{0}\': {1!s}'.format(self.device_port, ex)
            logging.exception(message)
            if self.interactive:
                print(message)
            raise
            # TODO try again or prompt for a new port
        self.is_connected = True
        message = 'Connected to \'{0}\''.format(self.device_port)
        logging.info(message)
        if self.interactive:
            print('{0}.\n'.format(message))



    def __del__(self):
        """\
        Deconstructor for SerialLogger.

        """
        self.disconnect()

    def validate_baud(self):
        """\
        Ensure that baud_rate is an integer or clear it.

        """
        logging.debug('Validating baud_rate \'{0!s}\''.format(self.baud_rate))
        try:
            self.baud_rate = int(self.baud_rate)
        except ValueError, TypeError:
            print('Invalid baud rate \'{0!s}\''.format(self.baud_rate))
            self.baud_rate = None

    def validate_port(self):
        """\
        Ensure that device_port exists or clear it.

        """
        logging.debug('Validating port \'{0}\''.format(self.device_port))
        if (not os.name == 'nt') and self.device_port and not os.path.exists(self.device_port):
            print('No such device \'{0}\''.format(self.device_port))
            self.device_port = None

    def disconnect(self):
        """\
        Disconnects from an open serial port.

        """
        # Are we connected?
        if (self.is_connected and self.connection):
            logging.debug('Disconnecting from device \'{0}\''.format(self.device_port))
            self.connection.close()
            sys.stdout.write('\nDisconnected from device \'{0}\''.format(self.device_port))
            logging.info('Disconnected from device \'{0}\''.format(self.device_port))
 
                
    def start_terminal(self):
        """\
        Begins a session where the user can interact by typing commands and
        seeing output from the connected serial device. The session is
        terminated and this function returns when the user types the 'exit'
        command.

        """
        # pp = pprint.PrettyPrinter(indent=4)

        # print('Enter a command. For a list type \'help\'.')

        # Main loop
        line = ''
        while (not self.want_exit):
            # user_input = raw_input(self.prompt)
            data = self.connection.read(1)
            sys.stdout.write(data)


            if (data == '\n'):
                line.rstrip('\r\n')
                logging.info(line)
                line = ''
            elif (data == '\r'):
                pass
            else:
                line += repr(data)[1:-1]

            #if data == '\n':
            #    line.rstrip('\r\n')


            # print(self.connection.readline())
            # self.execute_command(user_input)
            sys.stdout.flush()

    
    def execute_command(self,command):
        """\
        Parses and executes the given command. Valid commands are
        documented in the show_help() function.

        Args:
            command: string to be parses and executed
        Returns:
            error or success code
            0 -- success 
            * -- error code

        """
        # Parse command
        command = command.strip().lower()

        # Execute command
        if (command == 'exit'):
            print self.goodbye
            self.want_exit = True
        elif (command == 'help'):
            self.show_help()
        elif (command == 'listing'):
            print('Not implemented!')
        elif (command == 'dumpfile'):
            print('Not implemented!')
        elif (command == 'info'):
            print('Connected to \'{0}\' at {1!s} baud.'.format(self.device_port,self.baud_rate))
        else:
            print('Unknown command \'{0}\'\nUse \'help\' for a list of valid commands.'.format(command))


    
    def ask_port(self):
        """\
        Queries the user to enter a serial device port.

        Returns:
        the device port path or id given by the user.

        """
        user_input = raw_input('Choose a port: ')
        return user_input.strip()


    def ask_baud(self):
        """\
        Queries the user for a baud rate.

        Returns:
            the baud rate given by the user.

        """
        default_str = ''
        if (self.baud_rate):
            default_str = ' [%s]' % self.baud_rate
        user_input = raw_input('Choose a baud rate%s: ' % default_str)
        return user_input.strip()

    def list_ports(self):
        """\
        Lists the available serial ports.

        """
        # Build a port list
        port_list_all = comports()
        port_list = list()
        for device in port_list_all:
            port_list.append(device[0])
    
        if len(port_list) < 1:
            print('There are no available serial ports.');
            self.want_exit = True
            return

        print('Available serial ports:')
        for port in port_list:
            print('\t{0}'.format(port))

        print('\n')


