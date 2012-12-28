function [msg] = gps_readMessage_nmea(serial_obj)
% [str] = gps_readMessage_nmea(serial_obj)
%
% Reads an NMEA message (CR/LF terminated) from a GPS device
% and returns it as a string.
%
% Arguments:
%   serial_obj: a serial object
%
% Returns:
%   a string containing an NMEA message
%

if nargin < 1
    error('Missing argument ''serial_obj''')
end

%MAX_STRING_SIZE = 150;
START_CHAR = '$';
CHECK_CHAR  = '*';
DELIM_CHAR = ',';
LF_CHAR = char(10);
CR_CHAR = char(13);

c = '';
str = '';
pos = 1;


while 1
    c = fread(serial_obj,1,'char');
    
    % Look for start of string or skip
    if pos == 1 && c ~= START_CHAR
        continue;
    end

    str = [str c];
    pos = pos + 1;
    
      
    if c == LF_CHAR % || pos >= MAX_STRING_SIZE
        break;
    end
end

% Done 

end % function



