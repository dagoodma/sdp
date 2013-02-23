# Serial Logger #

*   Website: http://lifeguardrobotics.com
*   Source: https://github.com/dagoodma/sdp/tree/master/tool/serial\_logger
*   Mailing list: [Google Groups](https://groups.google.com/a/ucsc.edu/forum/#!forum/alg-group)

This tool is used to connect to a device over serial and send and receive data while logging the interaction to a file.

## Usage ##

When run, the tool will list available serial ports and query the user to select one for connecting to. Once connected, the tool will display and record and data recieved. (TODO: Add the ability to send data.)

Follow the three steps below to use the serial logger. If you would like to use the GUI, follow step 1 from *With GUI*, and if you prefer the command line, follow step 1 from *Without GUI*. Steps 2 and 3 are the same for either method.


1. Run the tool.
    - For the GUI, on *Windows* run `serial_logger.exe`, on Mac run `serial_logger.app`.
    - Without GUI, open a terminal in `serial_logger/src`, then run `python serial_logger.py`. See _Arguments_ under the *Configuring*  section below for more information.

2. Choose serial port.
    - Windows devices are numbered ie. *COM4*
    - Mac are mounted with a serial number to the */dev* folder ie. */dev/tty.usbserial-A1012WFD*

3. Done.
    - Incoming serial data will be displayed in the console window.
    - Open *serial_logger.log* to view the recorded session.


## Configuring ##

The serial logger can be configured by setting desired parameters in *serial_logger/src/.config.cfg*. If you are using the command line, arguments may be passed which will override the config. See *Arguments* below for more details.

### Arguments ###

    python serial_logger.py -h | [-l log_file] [--loglevel=level] [-c config_file] [ -t timeout] [-b baud_rate] [device_path]

#### Positional ####
    device_path           device path or id of the serial port

#### Optional ####

    -b BAUD_RATE, --baud BAUD_RATE
                        baud rate for the serial connection (default: 9600)

    -c CONFIG_FILE, --config CONFIG_FILE
                        config file with default values (default: .config.cfg)

    -t TIMEOUT, --timeout TIMEOUT
                        serial connection timeout in seconds (default: 5)

    -l LOG_FILE, --log LOG_FILE
                        log file to record session to (default:
                        serial_logger.log)

    --loglevel {DEBUG,INFO,WARNING,ERROR,CRITICAL}
                        sets the logging level (default: INFO)

    -h, --help          show this help message and exit

## Compatibility ##

This module was written for Python 2.7.3, and has been tested on Windows 7 (64 and 32 bit), and Mac OS X ??.

TODO: Add python 3.3 support.

## Author ##

&copy; 2012-2013 David Goodman
