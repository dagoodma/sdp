function [serial_obj] = sonar_configureDevice(name,port,baudrate)
% [serial_obj] = gps_configure_globalsat(name,port,baud_rate)
%
% Connects to the a microcontroller over Serial and dumps incoming data.
%
% Arguments:
%   name: of the device for messages and errors.
%   port: device path (Unix) or number (Windows).
%   baudrate: an optional baud rate to use.
%
% Returns:
%   a serial object or throws an error if something went wrong
%
DEBUG=1;
serial_obj=0;

% Missing required arguments?
if nargin < 1 || ~ischar(name) || length(name) < 1
    error('Missing argument ''name''')
elseif nargin < 2
    error('Missing argument ''port''')
elseif nargin < 3
    error('Missing argument ''baudrate''')
% Validate port
elseif ispc && ~isnumeric(port)
    error('Expected numeric ''port'' on PC')
elseif ~ispc && ~(iscellstr(port) || ischar(port))
    error('Expected ''port'' as string on *nix')
elseif ~isnumeric(baudrate)
    error('Expected numeric ''baudrate''')
elseif nargin > 6
    error('Too many arguments given')
end

% Convert port from cell string or windowsify if needed
if ~ispc && ~ischar(port)
    port = char(port);
elseif ispc
    port = sprintf('COM%d',port);
end

if DEBUG
    disp(sprintf('Connecting to %s on %s at %u baud',name,port,baudrate))
end

% Open connection
serial_obj=serial(port,'BAUD',...
    baudrate,'Terminator','CR/LF','InputBufferSize',8192);
fopen(serial_obj);

% Check connection
if isa(serial_obj, 'serial')
    if DEBUG
        disp(sprintf('Connected to %s',name));
    end
else
    error(sprintf('Failed to connect to %s', name));
end

% Done
if DEBUG
    disp(sprintf('Finished configuring %s',name));
end

end % function

