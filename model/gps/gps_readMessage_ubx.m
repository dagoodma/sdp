function [msg] = gps_readMessage_ubx(serial_obj,use_hex_str)
% [str] = gps_readMessage_ubx(serial_obj, use_hex_str)
%
% Reads a message UBX message from a UBlox GPS device and packs data
% into cells.
%
% Arguments:
%   serial_obj: a serial object
%   use_hex_str: returns packet data as hex string when set
%
% Returns:
%   a cell array containing ubx packet data
%

DEBUG = 1;
USE_HEX_STRING = 0;
USE_CHECKSUM = 0;

if nargin < 1
    error('Missing argument ''serial_obj''')
elseif nargin > 1
    USE_HEX_STRING = use_hex_str;
end

START_CHAR = 181;
START_CHAR2 = 98;
SYNC1_POS = 1;
SYNC2_POS = 2;
LENGTH_POS = 5;
PAYLOAD_POS = 7;
CHECKSUM_BYTE_NUMBER = 2;


pos = 1; % message index

max_pos = 10; % starts at 10 but gets changes when length is read

msg = {};


while 1
     % Read byte
     c = fread(serial_obj,1,'uint8');
     
     % Read length
     if pos == (LENGTH_POS)
         payloadlength = c;
     elseif pos == (LENGTH_POS + 1)
         payloadlength = payloadlength + bitshift(c,8);
         max_pos = PAYLOAD_POS + payloadlength + CHECKSUM_BYTE_NUMBER;
     end
     
     % Add data to message
     if USE_HEX_STRING
         msg{pos} = sprintf('%X',c);
     else
         msg{pos} = c;
     end

     if isempty(c) || ~isnumeric(c)
         error('Failed to read from the device.')
     end
     % Validate message by checking position
     if pos == SYNC1_POS && c ~= START_CHAR
         continue; % keep reading until start char
     elseif pos == SYNC2_POS && c ~= START_CHAR2
         pos = 1; % restart if not 2nd start char
         continue;
     end

     pos = pos + 1;

     % At the end of the packet?
     if pos > max_pos
         % Check the checksum
         if (~USE_HEX_STRING)
             % Validate message by checking position and start data again
             if msg{SYNC1_POS} ~= START_CHAR
                 continue; % keep reading until start char
             elseif msg{SYNC2_POS} ~= START_CHAR2
                 pos = 1; % restart if not 2nd start char
                 continue;
             end
             if USE_CHECKSUM
                 checksum = gps_checksum_ubx(msg);
                 if ~iscell(checksum)
                     if (DEBUG)
                         disp(sprintf('Checksum calculation failed!\n'));
                     end
                     msg = 0;
                 elseif (checksum{1} ~= msg{max_pos - 1} || checksum{2} ~= msg{max_pos})
                     if (DEBUG)
                         disp(sprintf('Failed checksum {0x%X,0x%X} != 0x%X%X\n', checksum{1}, checksum{2}, msg{max_pos - 2}, msg{max_pos - 1})); 
                     end
                    msg = 0;
                 end
             end
         end
         break
     end
end

% Done 

end % function



