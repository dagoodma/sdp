function [checksum] = gps_checksum_ubx(command)
% [command] = gps_checksum_ubx(command)
%
% Calculates the UBX checksum to proceed the command in two bytes. Checksums
% are calculated by XORing the bytes from the command. See Commands for more
% information.
%
% Arguments:
%   command: an UBX command packed in an array of unsigned bytes
%            ie. [181 98 1 12 04 00 01 01]
%
% Returns: the UBX command packed in an array proceeded by the calculated
%   checksum bytes CK_A and CK_B.
%
% Note: Length is packed in two bytes (positions 5 and 6).
%   Use uint8() on command array to drop decimal and truncate.
%


if nargin < 1
    error('Missing argument ''command''')
elseif ~isinteger(command)
    error('Expected ''command'' as an integer array');
end

CHECK_RANGE_START = 3;
CHECK_POS = 0;
LENGTH_POS = 5;

% Get packet length (little endian)
length = command(LENGTH_POS) + bitsll(command(LENGTH_POS + 1),8);
length = length + 6; % length up to checksum

ck_a = 0; ck_b = 0;

for i=CHECK_RANGE_START:length,
    ck_a = uint8(ck_a + command(i));
    ck_b = uint8(ck_b + ck_a);
end

command(length + 1) = ck_a;
command(length + 2) = ck_b;

end % function


