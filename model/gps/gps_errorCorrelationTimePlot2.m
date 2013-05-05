function [ coords ] = gps_errorCorrelationTimePlot( filename1, filename2, truth_coordinate )
%GPS_ERRORCORRELATIONTIMEPLOT Graphs error correlation in time.
%   Creates a lines graph that compares the instantaneous error of two GPS
%   data files with timestamps against each other at equal times. Note that
%   if two data files don't share any common timestamps, then an error will
%   be reported.
%
%   Data format is in:
%       lat, lon, alt (m), time (ms)
%   
%   ie. 
%       gps_errorCorrelationTimePlot(datafile1, datafile2, {lat lon})
%

% Constants
% Missing required arguments?
if nargin < 1 || ~ischar(filename1) || length(filename1) < 1
    error('Missing arguments ''filename1''')
elseif nargin < 2 || ~ischar(filename2) || length(filename2) < 1
    error('Missing argument ''filename2''') 
elseif nargin < 3 || ~iscell(truth_coordinate)
    error('Missing argument ''truth_coordinate''')
elseif length(truth_coordinate) ~= 3
    error('Expected three coordinates in ''truth_coordinate''')
elseif exist(filename1) ~= 2
    error(sprintf('%s: file does not exist',filename1));
elseif exist(filename2) ~= 2
    error(sprintf('%s: file does not exist',filename2));
elseif nargin > 3
    error('Too many arguments given');
end

% Default Options
USE_METERS = 0;
% -- don't touch these ---

    unit = 'm';

% Constants
LAT_INDEX = 1;
LON_INDEX = 2;
ALT_INDEX = 3;
TIME_INDEX = 4;

LON_TO_METERS = 67592.4; % (m/deg)
LON_TO_FEET = 221760; % (ft/deg)
LAT_TO_METERS = 111319.892; % (m/deg)
LAT_TO_FEET = 365223; % (ft/deg)

TIME_DELTA_MAX = 500; % (ms) max time between two timesteps when comparing


% Read the raw data
coords1 = dlmread(filename1);
coords2 = dlmread(filename2);





% Iterate through filename1's points until a matching timepoint is found in
% filename2.
% x = [];
% ylat = [];
% ylon = [];
% for i1=1:length(timestamp1)
%     time1_i = timestamp1(i1);
%     timediff = abs(timestamp2 - time1_i);
%     [i2] = min(timediff);
%     
%     % compare time1_i and time2_i in lat and lon
%     if (i2 > 0)
%         x=[x time1_i];
%         ylat=[ylat abs(latError1(i1) - latError2(i2))];
%     
%         ylon=[ylon abs(lonError1(i1) - lonError2(i2))];
%     end
%     
% end
% t_steps=min(length(time1),length(time2));
% time1=time1(1:t_steps);
% time2=time2(1:t_steps);
% time1=time1(1:t_steps);
% time2=time2(1:t_steps);
% % latError1 = latError1(1:t_steps)
% % latError2 = latError2(1:t_steps)
% % lonError1 = lonError1(1:t_steps);
% % lonError2 = lonError2(1:t_steps);

% NOT WORKINg
% time = zeros(length(time1),1);
%[bool1,spot1]=ismember(timestamp1,timestamp2);
%indices1 = spot1(bool1);
%northError1 = northError1(indices1);
%eastError1 = eastError1(indices1);
%time1=time1(indices1);
%[bool2,spot2]=ismember(timestamp2,timestamp1);
%indices2 = spot2(bool2);
%northError2 = eastError2(indices2);
%eastError2 = eastError2(indices2);
%time2=time2(indices2);
%isequal(time1,time2)
%timestamp1(4624)
%timestamp2(4624)
% indices2 = spot(bool)
% latErrorDiff = abs(latError1(indices1) - latError2(indices2)); 
% 

% % Convert time to seconds from start time
% % x=abs(x(1) - x)/1000;
% % Chop extra values off
% time1=(timestamp1 - timestamp1(1))/1000;
% time2=(timestamp2 - timestamp2(1))/1000;

%% Statistics
% Transform to ECEF coordinates
[ecef1_x ecef1_y ecef1_z] = lla2ecef(coords1(:,LAT_INDEX), coords1(:,LON_INDEX), coords1(:,ALT_INDEX));
[ecef2_x ecef2_y ecef2_z] = lla2ecef(coords2(:,LAT_INDEX), coords2(:,LON_INDEX), coords2(:,ALT_INDEX));
[ecefTruth_x ecefTruth_y ecefTruth_z] = lla2ecef(truth_coordinate{LAT_INDEX},truth_coordinate{LON_INDEX},truth_coordinate{ALT_INDEX});

% Translate to NED from truth point
[ned1_n ned1_e ned1_d] = ecef2ned(ecef1_x,ecef1_y,ecef1_z,ecefTruth_x, ecefTruth_y, ecefTruth_z,truth_coordinate{LAT_INDEX},truth_coordinate{LON_INDEX});
[ned2_n ned2_e ned2_d]  = ecef2ned(ecef2_x,ecef2_y,ecef2_z,ecefTruth_x, ecefTruth_y, ecefTruth_z,truth_coordinate{LAT_INDEX},truth_coordinate{LON_INDEX});
% nedTruth is origin, (0,0,0)

timestamp1=coords1(:,TIME_INDEX);
timestamp2=coords2(:,TIME_INDEX);

% Interpolate ublox
ned2_n = interp1(timestamp2,ned2_n,timestamp1);
ned2_e = interp1(timestamp2,ned2_e,timestamp1);
ned2_d = interp1(timestamp2,ned2_d,timestamp1);
timestamp2 = timestamp1;

northError1 = 0 - ned1_n;
eastError1 = 0 - ned1_e;
downError1 = 0-ned1_d;

northError2 = 0 - ned2_n;
eastError2 = 0 - ned2_e;
downError2 = 0-ned2_d;


% Apply correction

northError3 = ned2_n - ned1_n;
eastError3 = ned2_e - ned1_e;
downError3 = ned2_d - ned1_d;
timestamp3=timestamp2;

% Plot results
isequal(timestamp1,timestamp2)
clf;
subplot(2,1,1)
% plot(x,ylat)
plot(timestamp1,northError1,timestamp2,northError2);%,time,latErrorDiff);
hold on;
plot(timestamp3, northError3,'--r');
%hold on; plot([xlim(1) xlim(2)],[0 0],'k'); hold off;
 title(sprintf('GPS Error Comparison vs. Time'))
ylabel(sprintf('North error [%s]',unit));
legend('Ublox1','Ublox2')
hold off;

subplot(2,1,2)
% plot(x,ylon)
plot(timestamp1,eastError1,timestamp2,eastError2);
hold on;
plot(timestamp3, eastError3,'--r');
ylabel(sprintf('East error [%s]',unit));
xlabel(sprintf('Time (s)'));
legend('Ublox1','Ublox2')
%% Labeling

% latitude
% longitude

hold off;
disp(sprintf('Compared %d data points.\n',length(time1)));

end