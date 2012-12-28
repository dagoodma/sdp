% Autoconfigures the 2 GPS devices for the lifeguard test system

% device enumeration
ublox1=1;
ublox2=2;

% device names
clear names;
names(ublox1)={'Ublox-1'};      % A
names(ublox2)={'Ublox-2'};      % B

% close all serial ports
delete(instrfindall)

% com ports (configure these)
clear portnums;
%portnums(ublox1)=1;
%portnums(ublox2)=2;
portnums(ublox1)={'/dev/tty.usbserial-A1012WFD'};
portnums(ublox2)={'/dev/tty.usbserial-A1012WEE'};

% connect to devices
clear ports;
%ports(ublox1) = {gps_configure_ublox(portnums(ublox1))};
ports(ublox2) = {gps_configure_ublox(portnums(ublox2))};

disp(sprintf('Success!\n'));

disp(sprintf('Listening...'));

% Dump messages
while 1
    disp(gps_read_message2(ports{ublox2}));
end


% Done
