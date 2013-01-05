% Records geodetic position from GPS devices into files

% variables
DEBUG=1;
record=1;

%% GPS Configuration
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

% file names
files(ublox1)={sprintf('data/%s_ublox1_geodetic',datestr(now,'yyyy.mm.dd-HHMMSS'))};
files(ublox1)={sprintf('data/%s_ublox2_geodetic',datestr(now,'yyyy.mm.dd-HHMMSS'))};

% connect to devices
clear ports;
%ports(ublox1) = {gps_configure_ublox(portnums(ublox1))};
ports(ublox2) = {gps_configure_ublox(portnums(ublox2))};

if DEBUG
    disp(sprintf('Success!\n'));
    disp(sprintf('Recording...'));
end

%% Read GPS data
% Dump messages
try
    while 1
        out = gps_readMessage_ubx(ports{ublox2},1);
    end
catch err
    % Do nothing
    disp(err.message);
end

%% Clean up
delete(instrfindall)
clear ports portnums names;

% Done

