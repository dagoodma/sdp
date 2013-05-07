% GPS Convergence Test2
%   Plots GPS data in time steps for 5hr, 19min, 48sec minute GPS data
%   from 2013.02.14 (ublox1).
%
%   Location: lat=6.970631, lon=121.904695
%       118 Hainline Rd, Aptos CA, 95003
%
%       Used: http://itouchmap.com/latlong.html
%

SEC = @(s) s;
MIN = @(s) 60*s;
HOUR = @(s) 60*60*s;

USE_TRUTH = 1;

truthCoord = {37.00036050528008,-122.06306055188548,248.420};
% 38260 points over 19188 sec
DATA_FREQUENCY = 1.99; % (Hz | points/sec)
DATA_FILES = { 'data/2013.04.27-184205_ublox1_geodetic.dlm', 'data/2013.04.27-184205_ublox2_geodetic.dlm'};
PLOT_TIMESTEPS = [MIN(15) HOUR(2) HOUR(6)]; % (seconds)

figure(5); clf;

gps_errorCorrelationTimePlot2(DATA_FILES{1}, DATA_FILES{2},truthCoord);
