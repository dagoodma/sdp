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
%   St St St St ....
%   C0 C1 C2 C3 C4 C5 C6 C7 .........   C16
% R0  
% R1
% R2
% R3
%   En En En En.... 
%
% Returns:
%   a cell array containing ubx packet data
%

DEBUG = 0;
USE_HEX_STRING = 0; % default

if nargin < 1
    error('Missing argument ''serial_obj''')
elseif nargin < 2
    use_hex_str = USE_HEX_STRING;
end


TOTAL_PIXEL_ROWS =  4;
TOTAL_PIXEL_COLS =  16;
START_INT = hex2dec('FFFF1234'); %start sequence everytime we are ready to read next column
END_COL_INT = hex2dec('FFFFAAAA'); %finish sending 4 pieces of information down each column


i = 0; j = 0;
if ~use_hex_str
    pixels = zeros(TOTAL_PIXEL_ROWS,TOTAL_PIXEL_COLS); %pre-allocate the row by column 
else
    pixels = {};
end

c = 0;
while 1
     % Wait for the start int
     c = fread(serial_obj,1,'uint32');
     while (c ~= START_INT)
         disp(sprintf('%X != %X',c,START_INT));
         c = fread(serial_obj,1,'uint32');
     end
     if DEBUG
     	disp(sprintf('Saw beginning of sequence.\n'));
     end
     % Reading rows
     for i=1:TOTAL_PIXEL_COLS
         for j=1:TOTAL_PIXEL_ROWS
             data = fread(serial_obj,1,'float');
             if ~use_hex_str
                pixels(j,i) = data;
             else
                 pixels{j,i} = sprintf('%X',data);
             end
         end
         % Look for row ending after reading whole row
         if (fread(serial_obj,1,'uint32') ~= END_COL_INT)
             if DEBUG
                 disp(sprintf('Never saw col ending!\n'));
             end
             break;
         end
     end
     % If we read a complete pixel matrix, then return
     if (j == TOTAL_PIXEL_ROWS && i == TOTAL_PIXEL_COLS)
         return;
     end
end

% Done 

end % function



