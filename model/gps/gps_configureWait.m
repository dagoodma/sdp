function [result] = gps_configureWait(serial_obj)
% [result] = gps_configureWait(serial_obj)
%
% After sending a message to a GPS device, use this function to wait for
% the device to finish processing the message. 
%
% Arguments:
%   serial_obj: an open serial connection to a GPS device
%
% Returns:
%   1: if the device is finished with the message
%   0: if the device timed out or failed
%
result=1;

if nargin < 1
    error('Missing argument ''serial_obj''')
end

% Catch the serial timeout warning and return false
lastwarn('');
%disp('in wait');

% flush the serial port
if(serial_obj.BytesAvailable~=0)
    fread(serial_obj,serial_obj.BytesAvailable);
try
    while(fread(serial_obj,1)~='$')
        if(~isempty(lastwarn))
            [msg, msgid] = lastwarn;
            %disp(sprintf('id:%u, msg: %s', msgid, msg))
            error(lastwarn)
        end
    end % while
catch err
    result = 0;
end % try/catch

end % function
