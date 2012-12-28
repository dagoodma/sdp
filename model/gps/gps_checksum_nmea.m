function [checksum] = gps_checksum_nmea(command)
% [checksum] = gps_checksum_nmea(command)
%
% Calculates the NMEA checksum to proceed the command. Checksums are
% calculated by XORing the bytes from the command. See Commands for more
% information.
%
% Arguments:
%   command: an NMEA command string to calculate the checksum for
%            ie. PSRF103,04,00,01,01
%
% Returns: a hex string representing the an NMEA checksum
%

if nargin < 1
    error('Missing argument ''command''')
elseif ~ischar(command)
    error('Expected ''command'' as string');
end

len = size(command); len = len(2);
checksum = '0'; % hex
checksum_byte = de2bi(unicode2native(command(1)),'left-msb');
for i=1:len - 1
    byte1 = checksum_byte;
    byte2 = [ de2bi(unicode2native(command(i + 1)),'left-msb') ];

    % pad with zeros
    len1 = 8 - length(byte1);
    len2 = 8 - length(byte2);
    if len1 > 0, byte1 = padarray(byte1,[0 len1 ],'pre'); end
    if len2 > 0, byte2 = padarray(byte2,[0 len2 ],'pre'); end
    
    checksum_byte = xor(byte1, byte2);
end

checksum = dec2hex(bi2de(checksum_byte,'left-msb'));

end % function

