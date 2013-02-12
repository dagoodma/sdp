function [pixels] = thermal_readMessage(serial_obj,use_hex_str)
% [pixels] = thermal_readMessage(serial_obj,use_hex_str)
%
% Reads a message from the Thermal device and returns a pixel matrix
%   containing temperatures.
%
% Arguments:
%   serial_obj: a serial object
%   use_hex_str: returns packet data as hex string when set
%
% Returns:
%   a cell array containing ubx packet data
%

DEBUG = 1;
USE_HEX_STRING = 0; % default

if nargin < 1
    error('Missing argument ''serial_obj''')
elseif nargin < 2
    use_hex_str = USE_HEX_STRING;
end


TOTAL_PIXEL_ROWS =  4;
TOTAL_PIXEL_COLS =  16;
START_INT = hex2dec('FFFF1234');
END_ROW_INT = hex2dec('FFFFAAAA');


i = 0; j = 0;
if ~use_hex_str
    pixels = zeros(TOTAL_PIXEL_ROWS,TOTAL_PIXEL_COLS);
else
    pixels = {};
end


while 1
     % Wait for the start int
     while (fread(serial_obj,1,'uint32') ~= START_INT)
         % Do nothing
     end
     if DEBUG
     	disp(sprintf('Saw beginning of sequence.\n'));
     end
     % Reading rows
     for i=1:TOTAL_PIXEL_ROWS
         for j=1:TOTAL_PIXEL_COLS
             data = fread(serial_obj,1,'float');
             if ~use_hex_str
                pixels(i,j) = data;
             else
                 pixels{i,j} = sprintf('%X',data);
             end
         end
         % Look for row ending after reading whole row
         if (fread(serial_obj,1,'uint32') ~= END_ROW_INT)
             if DEBUG
                 disp(sprintf('Never saw row ending!\n'));
             end
             break;
         end
     end
     % If we read a complete pixel matrix, then return
     if (i == TOTAL_PIXEL_ROWS && j == TOTAL_PIXEL_COLS)
         return;
     end
end

% Done 

end % function



