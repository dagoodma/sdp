% Dumps packets from the connected serial device

% variables
DEBUG=1;
%record=1;

%% GPS Configuration
% device enumeration
uno32=1;

% device names
clear names;
names(uno32)={'Uno32 Sonar'};
baudrate = 115200;

% close all serial ports
delete(instrfindall)

% com ports (configure these)
clear portnums;
%portnums(uno32)=4; %Com Port
portnums(uno32)={'/dev/tty.usbserial-AM01ALQX'};

% connect to devices
clear ports;
ports(uno32) = {sonar_configureDevice(names{uno32}, portnums(uno32), baudrate)};

if DEBUG
    disp(sprintf('\nListening to the %s...',names{uno32}));
end

%% Read Thermal data
% Dump messages
figure(); time = 1:100; clf;
while 1

    samples = sonar_readMessage(ports{uno32});    
    if DEBUG
        %disp('Thermal Pixels: ');
        %pixels
        %disp(sprintf('------------------\n'));
        %clf;
        %figure(1)
        plot(time, samples);
        grid on;
           pause(0.05)

    end
end

%% Clean up
delete(instrfindall)
clear ports portnums names;

% Done
