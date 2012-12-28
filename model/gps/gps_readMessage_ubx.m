function [msg] = gps_readMessage_ubx(serial_obj)
% [str] = gps_readMessage_ubx(serial_obj)
%
% Reads a message UBX message from a UBlox GPS device and packs data
% into cells.
%
% Arguments:
%   serial_obj: a serial object
%
% Returns:
%   a cell array containing ubx packet data
%

if nargin < 1
    error('Missing argument ''serial_obj''')
end

DEBUG = 1;
USE_HEX_STRING = 1;

START_CHAR = 181;
START_CHAR2 = 98;
SYNC1_POS = 1;
SYNC2_POS = 2;
LENGTH_POS = 5;

pos = 1; % absolute position
i = 1; % message index

max_pos = 10;

msg = {};


while 1
     % Read data from serial port
     if pos == LENGTH_POS
         % Read length field
         c = fread(serial_obj,1,'uint16');
     else
         % Read arbitrary byte
         c = fread(serial_obj,1,'uint8');
     end
     
     % Add data to message
     if USE_HEX_STRING
         msg{i} = sprintf('%X',c);
     else
         msg{i} = c;
     end

     % Validate message by checking position
     if pos == SYNC1_POS && c ~= START_CHAR
         continue; % keep reading until start char
     elseif pos == SYNC2_POS && c ~= START_CHAR2
         pos = 1; % restart if not 2nd start char
         continue;
     elseif pos == LENGTH_POS
        % Read 2-byte length
        pos = pos + 1;
        length = c;
        max_pos = 6 + length + 2;
     end

     pos = pos + 1;
     i = i + 1;

     % At the end of the packet?
     if pos >= max_pos
         break
     end
end

% Done 

end % function



