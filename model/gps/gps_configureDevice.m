function [serial_obj] = gps_configureDevice(name,port,start_baudrate,baudrate,commands,custom_baud_rate)
% [serial_obj] = gps_configure_globalsat(name,port,baud_rate)
%
% Connects to and configures a GPS device.
%
% Arguments:
%   name: of the device for messages and errors.
%   port: device path (Unix) or number (Windows).
%   start_baudrate: starting baud rate for the device.
%   baudrate: an optional baud rate to use.
%   commands: an optional array of cell strings containing commands.
%             if commands contain a checksum then preceed with '$',
%             otherwise the checksum will calculated and appended.
%   custom_baud_change:  optional argument if gps uses custom command to
%   change baud rate
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
    error('Missing argument ''start_baudrate''')
% Validate port
elseif ispc && ~isnumeric(port)
    error('Expected numeric ''port'' on PC')
elseif ~ispc && ~(iscellstr(port) || ischar(port))
    error('Expected ''port'' as string on *nix')
%elseif ~ispc && ~exist(char(port),'dir')
%    error(sprintf('No such port ''%s''',char(port)))
% Validate start_baudrate
elseif ~isnumeric(start_baudrate)
    error('Expected numeric ''start_baudrate''')
% Validate baudrate
elseif nargin >= 4 && ~isnumeric(baudrate)
    error('Expected numeric ''baudrate''')
% Validate commands
elseif nargin == 5 && ~iscellstr(commands)
    error('Exepected ''commands'' as cell string array')
elseif nargin == 6 && ~isstr(custom_baud_rate)
    error('Expected ''custom_baud_rate'' to be a string')
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
    disp(sprintf('Connecting to %s on %s at %u baud',name,port,start_baudrate))
end

% Open connection
serial_obj=serial(port,'BAUD',...
    start_baudrate,'Terminator','CR/LF','InputBufferSize',8192);
fopen(serial_obj);

% Check connection
if isa(serial_obj, 'serial')
    if DEBUG
        disp(sprintf('Connected to %s',name));
    end
else
    error(sprintf('Failed to connect to %s', name));
end

% Configure the baud rate if given
if nargin >= 4 && start_baudrate ~= baudrate
    command = sprintf('PMTK251,%u',baudrate);
    checksum = gps_checksum(command);
    if DEBUG
        disp(sprintf('Changing from %u baud to %u baud with command ''$%s*%s''',...
            start_baudrate,baudrate,command,checksum))
    end
    if nargin == 6
        fprintf(serial_obj,sprintf('%s\r\n',custom_baud_rate));
    else
        fprintf(serial_obj,sprintf('$%s*%s\r\n',command,checksum));
    end
    pause(1);
    set(serial_obj,'BAUD',baudrate);
    if ~gps_configureWait(serial_obj)
        error(sprintf('Device ''%s'' timed out setting baud rate',name))
    end
end

% Run the given commands
if nargin > 5
    for i=1:length(commands)
        command = commands{i};
        % Does command need checksum?
        if command(1) ~= '$'
            command = sprintf('$%s*%s',command,gps_checksum(command));
        end
        if DEBUG
            disp(sprintf('Sending command ''%s''',command));
        end
        fprintf(serial_obj,sprintf('%s\r\n',command));
        if ~gps_configureWait(serial_obj)
            error(sprintf('Device ''%s'' timed out on command ''%s''',name,command))
        end
    end %for
end

% Done
if DEBUG
    disp(sprintf('Finished configuring %s',name));
end

end % function

