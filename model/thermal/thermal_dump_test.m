% Dumps packets from the connected serial device

% variables
DEBUG=1;
record=1;

%% GPS Configuration
% device enumeration
uno32=1;

% device names
clear names;
names(uno32)={'Uno32 Thermal Sensor'};
baudrate = 115200;

% close all serial ports
delete(instrfindall)

% com ports (configure these)
clear portnums;
portnums(uno32)=4;
%portnums(ublox1)={'/dev/tty.usbserial-A1012WFD'};

% connect to devices
clear ports;
ports(uno32) = {thermal_configureDevice(names{uno32}, portnums(uno32), baudrate)};

if DEBUG
    disp(sprintf('Success!\n'));
    disp(sprintf('Listening...'));
end

%% Read GPS data
% Dump messages
total=60;
while i <= total,
    c = 0;
    str = '';
    while(c ~= 10)
        c = fread(ports{uno32},1);
        str = strcat(str, c);
    end
    disp(str);
end

%% Clean up
delete(instrfindall)
clear ports portnums names;

% Done
