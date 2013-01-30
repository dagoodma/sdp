function [ coords ] = gps_scatterPlot2D( filename )
%GPS_SCATTERPLOT@D Creates a 2D scatter plot of GPS data from a specified 
%   DLM file. Note: coordinates are stored: lat,lon,alt
%   
%   ie. 
%       gps_scatterPlot2D('data\2013.01.29-154816_ublox2_geodetic.dlm')
% Constants
LAT_INDEX = 1;
LON_INDEX = 2;
ALT_INDEX = 3;

LON_TO_METERS = 67592.4; % (m/deg)
LON_TO_FEET = 221760; % (ft/deg)
LAT_TO_METERS = 111319.892; % (m/deg)
LAT_TO_FEET = 365223; % (ft/deg)

% Options
USE_MEAN = 1;
USE_NEW_ORIGIN = 1;
USE_METERS = 0;

% Read the raw data
coords = dlmread(filename);


%% Statistics
% Translate the coords to new origin with mean
transCoords = coords;

lat=coords(:,LAT_INDEX);
latMean=sum(lat')/length(lat');
transCoords(:,LAT_INDEX) = latMean - lat;

lon=coords(:,LON_INDEX);
lonMean=sum(lon')/length(lon');
transCoords(:,LON_INDEX) = lonMean - lon;

if USE_METERS
    transCoords(:,LAT_INDEX) = transCoords(:,LAT_INDEX).* LAT_TO_METERS;
    transCoords(:,LON_INDEX) = transCoords(:,LON_INDEX).* LON_TO_METERS;
    unit = 'm';
else
    transCoords(:,LAT_INDEX) = transCoords(:,LAT_INDEX).* LAT_TO_FEET;
    transCoords(:,LON_INDEX) = transCoords(:,LON_INDEX).* LON_TO_FEET;
    unit = 'ft';
end

latTrans=transCoords(:,LAT_INDEX);
latMeanTrans=sum(latTrans')/length(latTrans');
lonTrans=transCoords(:,LON_INDEX);
lonMeanTrans=sum(lonTrans')/length(lonTrans');

% Variance
latVar = sum((latTrans - latMeanTrans).^2)/(length(latTrans) - 1);
lonVar = sum((lonTrans - lonMeanTrans).^2)/(length(lonTrans) - 1);

% Standard deviation
latStd = std(latTrans);
lonStd = std(lonTrans);

%% Plotting

if USE_MEAN
    % Print the translated coordinates around the new origin
    scatter(transCoords(:,LON_INDEX),transCoords(:,LAT_INDEX),'CData',[0 0 1])
else
    % Print the GPS coordinates exactly
    scatter(coords(:,LON_INDEX),coords(:,LAT_INDEX),'CData',[0 0 1])
end

% Draw origin
if USE_MEAN && USE_NEW_ORIGIN
    hold on;
    scatter(0,0, 'd','CData',[1 0 0]);
end

%% Labeling
title(sprintf('2D GPS Coordinate Scatter Plot\nLat: var = %.2f, std = %.2f\nLon: var = %.2f, std = %.2f',latVar,latStd,lonVar,lonStd));
% latitude
ylabel(sprintf('North/South (%s)',unit));
% longitude
xlabel(sprintf('East/West (%s)',unit));

% Create axes in .1 m range
x = [-20:0.1:20];
y = [-20:0.1:20];

hold off;

end