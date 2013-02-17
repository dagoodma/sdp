% GPS Convergence Test
%   Plots GPS data in time steps for 15 minute GPS data from 2013.02.05.

USE_TRUTH = 1;

truthCoord = {36.9552103,-122.0598241};

% Change these numbers
DATA_FREQUENCY = 1.893; % (points/sec)
DATA_FILE = 'data\2013.02.05-172157_ublox2_geodetic.dlm';
PLOT_TIMESTEPS = 60 * [2 10 13 15]; % (seconds)


coords = dlmread(DATA_FILE);
totalPoints = length(coords);


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