function [samples] = sonar_readMessage(serial_obj,use_hex_str)
% [pixels] = sonar_readMessage(serial_obj,use_hex_str)
%
% Reads a message from the sonar device and returns amplitude values
% over time
%
% Arguments:
%   serial_obj: a serial object
%   use_hex_str: returns packet data as hex string when set
%
% Returns:
%   an amplitude vector
%

DEBUG = 0;
USE_HEX_STRING = 0; % default

if nargin < 1
    error('Missing argument ''serial_obj''')
elseif nargin < 2
    use_hex_str = USE_HEX_STRING;
end


TOTAL_SAMPLES = 100;
START_INT = hex2dec('FFFF1234'); %start sequence everytime we are ready to read next column



i = 0;
if ~use_hex_str
    samples = zeros(1,TOTAL_SAMPLES); %pre-allocate the row by column 
else
    samples = {};
end

c = 0;
while 1
     % Wait for the start int
     c = fread(serial_obj,1,'uint32');
     while (c ~= START_INT)
       %  disp(sprintf('%X != %X',c,START_INT));
         c = fread(serial_obj,1,'uint32');
     end
     if DEBUG
     	disp(sprintf('Saw beginning of sequence.\n'));
     end
     % Reading rows

     for i=1:TOTAL_SAMPLES
         data = fread(serial_obj,1,'uint32');
         if ~use_hex_str
            samples(i) = data;
         else
             samples{i} = sprintf('%X',data);
         end
     end
     return;
end

% Done 

end % function



