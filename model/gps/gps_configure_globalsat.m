function [serial_obj] = gps_configure_globalsat(port,baudrate)
% [serial_obj] = gps_configure_globalsat(port,baudrate)
%
% Connects to and configures the GlobalSat SiRF Star GPS device and
% returns a serial object.
%
% Arguments:
%   port: com port that the device is connected to
%   baudrate: optional baud rate (default=115200)
%
% Returns:
%   a serial object or throws an error if something went wrong
%
name='Sirf_Star';
start_baud_rate=4800;
default_baudrate=115200;
custom_baud_command='$PSRF100,1,115200,8,1,0*05';

if nargin < 2
    baudrate = default_baudrate;
end

sample_rate='$PMTK220,1000*1F'; % 1Hz
gga_message='$PSRF103,00,00,01,01*25';
rmc_message='$PSRF103,04,00,01,01*21';
gps_messages_off={'$PSRF103,02,00,00,01*26' '$PSRF103,03,00,00,01*27' '$PSRF103,05,00,00,01*21' '$PSRF103,06,00,00,01*22' '$PSRF103,08,00,00,01*2C'};

commands = {sample_rate, gga_message, rmc_message,gps_messages_off{1:end}};

serial_obj = gps_configureDevice(name,port,start_baud_rate,baudrate,commands,custom_baud_command);

% Done

end % function




