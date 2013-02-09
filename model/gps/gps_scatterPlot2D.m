function [ coords ] = gps_scatterPlot2D( filename, maximum_length, truth_coordinate )
%GPS_SCATTERPLOT@D Creates a 2D scatter plot of GPS data from a specified 
%   DLM file. Note: coordinates are stored: lat,lon,alt
%   
%   ie. 
%       gps_scatterPlot2D('data\2013.01.29-154816_ublox2_geodetic.dlm')
% Constants
% Missing required arguments?
if nargin < 1 || ~ischar(filename) || length(filename) < 1
    error('Missing argument ''filename''')
elseif exist(filename) ~= 2
    error(sprintf('%s: file does not exist',filename));
elseif nargin > 1 && ~isnumeric(maximum_length)
    error('Expected numeric ''maximum_length''');
elseif nargin > 2 && ~iscell(truth_coordinate)
    error('Expected coordinate cell ''truth_coordinate''');
elseif nargin > 3
    error('Too many arguments given');
end

% Options
USE_MEAN = 1;
USE_NEW_ORIGIN = 1;
USE_METERS = 0;

% Constants
LAT_INDEX = 1;
LON_INDEX = 2;
ALT_INDEX = 3;

LON_TO_METERS = 67592.4; % (m/deg)
LON_TO_FEET = 221760; % (ft/deg)
LAT_TO_METERS = 111319.892; % (m/deg)
LAT_TO_FEET = 365223; % (ft/deg)


% Read the raw data
coords = dlmread(filename);

% Limit data read length by max
if nargin == 2
    maximum_length = min(maximum_length,length(coords));
    if (maximum_length == 0)
        maximum_length = length(coords);
    end
    coords = coords(1:maximum_length,:);
end


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
    scatter(transCoords(:,LON_INDEX),transCoords(:,LAT_INDEX),'CData',[0 0 1]);
    % Draw truth
else
    % Print the GPS coordinates exactly
    scatter(coords(:,LON_INDEX),coords(:,LAT_INDEX),'CData',[0 0 1]);
    
end
hold on;
% Draw origin
if USE_MEAN && USE_NEW_ORIGIN
    scatter(0,0, 'd','CData',[1 0 0],'MarkerFaceColor',[1 0 0]);
end

% Draw truth
if exist('truth_coordinate')
    transTruthLat = latMean - truth_coordinate{LAT_INDEX};
    transTruthLon = lonMean - truth_coordinate{LON_INDEX};
    if USE_MEAN
        if USE_METERS
            transTruthLat = transTruthLat* LAT_TO_METERS;
            transTruthLon = transTruthLon* LON_TO_METERS;
        else
            transTruthLat = transTruthLat* LAT_TO_FEET;
            transTruthLon = transTruthLon* LON_TO_FEET;
        end
    end
    scatter(transTruthLon,transTruthLat,'x','CData',[1 0.5 0.1],'MarkerFaceColor',[1 0.5 0.1]);
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
disp(sprintf('Plotted %d data points.\n',length(coords)));

end