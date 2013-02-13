#!/usr/bin/env python
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

def signal_handler(signal, frame):
    print 'You pressed Ctrl-C'
    cleanup()
    sys.exit(0)


"""-------------------------------------------------------------
                              Start
   -------------------------------------------------------------"""

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

DEBUG = True


# ----------------------------------
# Parse arguments

parser = argparse.ArgumentParser(
    formatter_class=argparse.RawDescriptionHelpFormatter,
    description="""\
Connects to an Animus system and retrieves log fies from an SD card
over a serial connection.
""",
    usage='serial_logger.py -h | [-l log_file] [--loglevel=LEVEL] [-c config_file] [-t timeout] [-b baud_rate] [device_path] ',
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
logging.basicConfig(filename=log_file,level=log_level)
logging.info('Started')

if DEBUG:
    print('(DEBUG) Config Options:')
    pp.pprint(config_obj.sections())

# --------------------------------
# Create and initialize SerialLogger object
myTerminalLogger = None

try:
    logging.debug('Initializing SerialLogger')
    myTerminalLogger = SerialLogger.SerialLogger(
        baud_rate = baud_rate,
        device_port = device_port,
        timeout = timeout,
        interactive = interactive
    )

#----------------------------------------------------------
####################### Main Loop #########################
#----------------------------------------------------------

# Start the SerialLogger
    interactive_str = 'interactive'
    if not interactive:
        interactive_str = 'non-interactive'
    logging.debug('Starting SerialLogger in {0} mode'.format(interactive_str))
    if interactive:
        myTerminalLogger.start_terminal()
except Exception, ex:
    logging.exception('SerialLogger failed: {0!s}'.format(ex))
    raise

cleanup()
def cleanup():
    if not myTerminalLogger is None:
        del myTerminalLogger

logging.info('Exiting')
