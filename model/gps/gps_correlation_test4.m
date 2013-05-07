% GPS Convergence Test3
%   Plots GPS data in time steps for 5hr, 19min, 48sec minute GPS data
%   from 2013.02.14 (ublox1).
%
%   Location: lat=36.98156271307515, lon=-121.9288632273674
%               alt=37.044
%       811 Pinetree Ln, Aptos, 95003   
%
%       Used: http://itouchmap.com/latlong.html
%

SEC = @(s) s;
MIN = @(s) 60*s;
HOUR = @(s) 60*60*s;

USE_TRUTH = 1;

truthCoord = {36.981544532282655,-121.9288632273674,37.078};
% 38260 points over 19188 sec
DATA_FREQUENCY = 1.99; % (Hz | points/sec)
DATA_FILES = { 'data/2013.04.28-010102_ublox1_geodetic.dlm', 'data/2013.04.28-010102_ublox2_geodetic.dlm'};
PLOT_TIMESTEPS = [MIN(15) HOUR(2) HOUR(6)]; % (seconds)

figure(5); clf;

gps_errorCorrelationTimePlot2(DATA_FILES{1}, DATA_FILES{2},truthCoord);
