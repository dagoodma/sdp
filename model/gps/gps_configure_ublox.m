function [serial_obj] = gps_configure_ublox(port,baudrate)
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
name='uBlox';
start_baudrate=38400;
default_baudrate=start_baudrate;%115200;
% 
% http://diydrones.com/forum/topics/connecting-3dr-ublox-lea6-module-via-usb
% "for anybody wondering: the default baud rate for the 3DR LEA6 is 38400 bps."
custom_baud_command='$PSRF100,1,115200,8,1,0*05';


if nargin < 2
    baudrate = default_baudrate;
end

serial_obj = gps_configureDevice(name,port,start_baudrate,baudrate);

% Done

end % function




