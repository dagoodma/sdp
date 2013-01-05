function [arr] = gps_parseMessage_ubx(msg)
% [str] = gps_readMessage_ubx(msg)
%
% Parses a UBX GPS message and returns the data. Currently only
% works with Navigational Geodetic Position messages (1,2).
%
% Arguments:
%   msg: a raw UBX GPS message
%
% Returns:
%   an array containing each field
%
% 
% Messages:
%   NAV-POSLLH (0x01 0x02)
%   --
%   iTow: GPS millisecond time of week (ms)
%   lon: longitude (deg)
%   lat: latitude (deg)
%   height: height above ellipsoid (mm)
%   hMSL: height above mean sea level (mm)
%   hAcc: horizontal accuracy estimate (mm)
%   vAcc: vertical accuracy estimate (mm)
%

DEBUG = 1;

if nargin < 1
    error('Missing argument ''msg''')
elseif isstr(msg{1})
    error('Expected ''msg'' as integers, but got strings.');
end

id = [msg{3} msg{4}];
arr = [];

% Navigation messages
if id(1) == 1
    % Geodetic Position Solution Message
    if id(2) == 2
        arr(1) = bitconcat(fi(msg{6},0,8), fi(msg{7},0,8), fi(msg{8},0,8), fi(msg{9},0,8));
        arr(2) = typecast(uint32(bitconcat(fi(msg{10},0,8), fi(msg{11},0,8), fi(msg{12},0,8), fi(msg{13},0,8))),'int32');
        arr(3) = typecast(uint32(bitconcat(fi(msg{14},0,8), fi(msg{15},0,8), fi(msg{16},0,8), fi(msg{17},0,8))),'int32');
    else
        error(sprintf('Navigation message 0x%X not implemented.', id(2)));
    end
else
    error(sprintf('Message ID 0x%X 0x%X not implemented.',msg(3),msg(4)));
end

end % function

