% GPS Convergence Test2
%   Plots GPS data in time steps. Started at 12:19 am and 32 seconds, ended at 8:03.
%   From 2013.02.23 (ublox1), lasting for 7:43:28   (h:m:s).
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

truthCoord = {36.981465,-121.928802};
% 55319 points over 27808 sec
DATA_FREQUENCY = 1.9174; % (Hz | points/sec)
DATA_FILE = 'data\2013.02.23-001932_ublox1_geodetic.dlm';
PLOT_TIMESTEPS = [MIN(15) HOUR(2) HOUR(5) HOUR(8)]; % (seconds)


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