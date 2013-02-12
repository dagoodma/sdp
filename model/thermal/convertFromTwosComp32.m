function [ signedint ] = convertFromTwosComp32( integer )
%convertFromTwosComp Converts a two's comp integer to a signed integer
%   if it should be negative.
%
%   ie. to convert a 32bit int properly, use:
%       x = 3074327485;
%       convertFromTwosComp32(x)
%           = 
%
BITS = 32;

signedint = integer;

% If top bit is set, then it's a negative #
if (bitand(integer, bitshift(1,BITS - 1)))
    % Two's complement conversion
    signedint = -int32(bitcmp(uint32(integer - 1)));
end

end

