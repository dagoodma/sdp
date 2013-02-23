% Records geodetic position from GPS devices into files
close all;

% Options
DEBUG=1; % toggle printing of debug messages
breakkey = 'q'; % key to stop recording

RECORD_DLM = 1; % Record to a DLM file for analysis
RECORD_KML = 0; % Record to a KML file for viewing in google maps
% KML plot settings
KML_COLOR = [ 0 1 0 1];
KML_SCALE = 1;

GPS_TOTAL = 2;
LAT = 2;
LON = 1;
ALT = 3;


%% GPS Configuration
% device enumeration
ublox1=1;
ublox2=2;

record(ublox1)=1;
record(ublox2)=1;

% device names
clear names;
names(ublox1)={'Ublox-1'};      % A
names(ublox2)={'Ublox-2'};      % B

% close all serial ports
delete(instrfindall)

% com ports (configure these)
clear portnums;
%portnums(ublox1)=1;
%portnums(ublox2)=6;
portnums(ublox1)={'/dev/tty.usbserial-A1012WFD'};
portnums(ublox2)={'/dev/tty.usbserial-A1012WEE'};

% connect to devices
clear ports;
if (record(ublox1))
    ports(ublox1) = {gps_configure_ublox(portnums(ublox1))};
end
if (record(ublox2))
    ports(ublox2) = {gps_configure_ublox(portnums(ublox2))};
end

if DEBUG
    disp(sprintf('Success!\n'));
    disp(sprintf('Recording...'));
end

% Class and Type Flags
NAV_CLASS = 1;
% Message Types
POSLLH_MSG = 2;
STATUS_MSG = 3;
% Fix Types
NO_FIX = 0;

% Device Statuses
clear fixes;
fixes(ublox1) = {NO_FIX};
fixes(ublox2) = {NO_FIX};
everfixed(ublox1) = {0};
everfixed(ublox2) = {0};

% Log filenames
datafolder='data';
filenames(ublox1)={sprintf('%s_ublox1_geodetic',datestr(now,'yyyy.mm.dd-HHMMSS'))};
filenames(ublox2)={sprintf('%s_ublox2_geodetic',datestr(now,'yyyy.mm.dd-HHMMSS'))};
files(ublox1)={''};
files(ublox2)={''};
if ~ismac
    files(ublox1)={sprintf('%s\\%s',datafolder,filenames{ublox1})};
    files(ublox2)={sprintf('%s\\%s',datafolder,filenames{ublox2})};
else
    
    files(ublox1)={sprintf('%s/%s',datafolder,filenames{ublox1})};
    files(ublox2)={sprintf('%s/%s',datafolder,filenames{ublox2})};
end

% Recorded coordindates
clear coords;
coords(GPS_TOTAL, 3) = {[]};


err = 0;
want_exit = 0;
%% Read GPS data

% Dump messages
try
    while 1
        % Iterate over GPS devices and print nav messages
        for i=1:GPS_TOTAL
            if (~record(i))
                continue
            end
            msg = gps_readMessage_ubx(ports{i},0);
            if ~iscell(msg)
                continue;
            end
            %------------- NAVIGATION MESSAGES --------------
            if msg{3} == NAV_CLASS
                % NAV-STATUS, fix or not
                if (msg{4} == STATUS_MSG)
                    parsed = gps_parseMessage_ubx(msg);

                    % No lock, so skip
                    if parsed(2) == NO_FIX
                        if DEBUG
                            disp(sprintf('%s not fixed.',names{i}));
                        end
                        continue
                    end

                    if DEBUG && fixes{i} == NO_FIX
                        disp(sprintf('%s has a lock.\n',names{i}));
                    end
                    fixes{i} = parsed(2);
                    everfixed{i} = 1;
                end
                % NAV-POSLLH, just want lat, lon data
                if (fixes{i} ~= NO_FIX && msg{4} == POSLLH_MSG)
                    parsed = gps_parseMessage_ubx(msg);
                    lat = parsed(3)*10^(-7);
                    lon = parsed(2)*10^(-7);
                    %hmsl = (parsed(5)*10^(-3))*3.28084; % (ft)
                    hmsl = (parsed(5)*10^(-3)); % (m)

                    if DEBUG
                        disp(sprintf('%s - NAV-POSLLH (%.0f)\n\tLat: %.2f\n\tLon: %.2f\n\thMSL: %.2f\n',names{i},fixes{i},lat,lon,hmsl));
                    end
                    
                    % Save coordinates
                    disp(i);
                    coords{i,LON} = [coords{i,LON} lon];
                    coords{i,LAT} = [coords{i,LAT} lat];
                    coords{i,ALT} = [coords{i,ALT} hmsl];
                    %k{ublox2}.plot3(lon,lat,hmsl)
                end

            end
            pause(0.01);
            % Check for key press
            if strcmp(get(gcf,'currentkey'),breakkey)
                disp(sprintf('Stopped recording.\n'));
                want_exit = 1;
                break
            end
        end % for
        
        if want_exit == 1
            break
        end
    end % while
catch err
    % Do nothing
end

%% Clean up
% Save the results
for i=1:GPS_TOTAL
    if everfixed{i}
        % Record to a DLM file for analysis
        if RECORD_DLM
            clear coords_to_file;
            coords_to_file = [ coords{i,LAT}' coords{i,LON}' coords{i,ALT}' ];
            %coords = [ latArr(i)' lonArr(i)' altArr(i)' ];
            disp(sprintf('Saving %s.dlm...',files{i}));
            dlmwrite(sprintf('%s.dlm',files{i}),coords_to_file,'precision','%.7f');
        end

        % Record to a KML file for viewing in google maps
        if RECORD_KML
            %k(ublox1) = {kml(filenames(ublox1))};
            k = kml(filenames{i});
            disp(sprintf('Saving %s.kml...',files{i}));
            k.scatter3(lonArr(i)',latArr(i)',altArr(i)','iconScale',KML_SCALE,'iconColor',KML_COLOR);
            k.save(sprintf('%s.kml',files{i}));
        end
    else
        disp(sprintf('Skipping saving of %s data since never fixed.',names{i}));
    end
    
    fprintf('Finished saving data.');
end % for

fprintf('Closing serial ports...');
closeAllSerialPorts;
clear k;


if (strcmp(class(err), 'MException'))
    disp('Failed recording GPS data:');
    rethrow(err);
end

% Done

