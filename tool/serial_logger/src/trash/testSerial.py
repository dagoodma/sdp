#!/usr/bin/env python
import serial
import pprint
from serial.tools.list_ports import *

pp = pprint.PrettyPrinter(indent=4)

pp.pprint(comports())

