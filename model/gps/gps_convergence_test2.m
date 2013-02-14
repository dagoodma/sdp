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

truthCoord = {36.970631,-121.904695};
% 38260 points over 19188 sec
DATA_FREQUENCY = 1.99; % (Hz | points/sec)
DATA_FILE = 'data\2013.02.14-024312_ublox1_geodetic.dlm';
PLOT_TIMESTEPS = [MIN(10) HOUR(1) HOUR(3) HOUR(6)]; % (seconds)


coords = dlmread(DATA_FILE);
totalPoints = length(coords);

figure(4)
if USE_TRUTH
    subplot(2,2,1);
    gps_scatterPlot2D(DATA_FILE,DATA_FREQUENCY * PLOT_TIMESTEPS(1),truthCoord);

    subplot(2,2,2);
    gps_scatterPlot2D(DATA_FILE,DATA_FREQUENCY * PLOT_TIMESTEPS(2),truthCoord);

    subplot(2,2,3);
    gps_scatterPlot2D(DATA_FILE,DATA_FREQUENCY * PLOT_TIMESTEPS(3),truthCoord);

    subplot(2,2,4);
    gps_scatterPlot2D(DATA_FILE,DATA_FREQUENCY * PLOT_TIMESTEPS(4),truthCoord);
else
    subplot(2,2,1);
    gps_scatterPlot2D(DATA_FILE,DATA_FREQUENCY * PLOT_TIMESTEPS(1));

    subplot(2,2,2);
    gps_scatterPlot2D(DATA_FILE,DATA_FREQUENCY * PLOT_TIMESTEPS(2));

    subplot(2,2,3);
    gps_scatterPlot2D(DATA_FILE,DATA_FREQUENCY * PLOT_TIMESTEPS(3));

    subplot(2,2,4);
    gps_scatterPlot2D(DATA_FILE,DATA_FREQUENCY * PLOT_TIMESTEPS(4));
end