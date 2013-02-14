function [ coords ] = gps_scatterPlot2D( filename, maximum_length, truth_coordinate )
%GPS_SCATTERPLOT@D Creates a 2D scatter plot of GPS data from a specified 
%   DLM file. Note: coordinates are stored: lat,lon,alt
%   
%   ie. 
%       gps_scatterPlot2D('data\2013.01.29-154816_ublox2_geodetic.dlm')
%       gps_scatterPlot2D('data\2013.01.29-154816_ublox2_geodetic.dlm',1000)
%       gps_scatterPlot2D('data\2013.01.29-154816_ublox2_geodetic.dlm',1000,{lat lon})
%

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
%filename, maximum_length, truth_coordinate

% Default Options
USE_MEAN = 1;
USE_METERS = 0;
% -- don't touch these ---
USE_NEW_ORIGIN = 0; % this is set below
USE_TRUTH = 0; % don't touch this since depends on argument
SHOW_POINTS = 1;

%
if nargin > 2
    USE_TRUTH = 1;
end

if USE_TRUTH || USE_MEAN
    USE_NEW_ORIGIN = 1;
end


% Constants
LAT_INDEX = 1;
LON_INDEX = 2;
ALT_INDEX = 3;

LON_TO_METERS = 67592.4; % (m/deg)
LON_TO_FEET = 221760; % (ft/deg)
LAT_TO_METERS = 111319.892; % (m/deg)
LAT_TO_FEET = 365223; % (ft/deg)

MEAN_SYMBOL = 'd';
TRUTH_SYMBOL = 'x'; % diamond
MEAN_COLOR = [0 1 0];
TRUTH_COLOR = [1 0.5 0.1];


% Read the raw data
coords = dlmread(filename);

% Limit data read length by max
if nargin >= 2
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
lon=coords(:,LON_INDEX);

latMean=sum(lat')/length(lat');
lonMean=sum(lon')/length(lon');
latTruth = 0;
lonTruth = 0;


% Determine new origin
if USE_NEW_ORIGIN   
    % Default mean as new origin
    latOrigin = latMean;
    lonOrigin = lonMean;
    
    if USE_TRUTH
        latTruth = truth_coordinate{LAT_INDEX};
        lonTruth = truth_coordinate{LON_INDEX};
        
        latOrigin = latTruth;
        lonOrigin = lonTruth;
    end
    %coords;
    coords(:,LON_INDEX) = lonOrigin - lon;
    coords(:,LAT_INDEX) = latOrigin - lat;
    %coords;
end


% Translate coordinates to new origin (either mean or truth)



if USE_NEW_ORIGIN && USE_METERS
    coords(:,LAT_INDEX) = coords(:,LAT_INDEX).* LAT_TO_METERS;
    coords(:,LON_INDEX) = coords(:,LON_INDEX).* LON_TO_METERS;
    unit = 'm';
elseif USE_NEW_ORIGIN
    coords(:,LAT_INDEX) = coords(:,LAT_INDEX).* LAT_TO_FEET;
    coords(:,LON_INDEX) = coords(:,LON_INDEX).* LON_TO_FEET;
    unit = 'ft';
end

% Take translated mean for stats below
latTrans=coords(:,LAT_INDEX);
lonTrans=coords(:,LON_INDEX);
latMeanTrans=sum(latTrans')/length(latTrans');
lonMeanTrans=sum(lonTrans')/length(lonTrans');

% Variance
latVar = sum((latTrans - latMeanTrans).^2)/(length(latTrans) - 1);
lonVar = sum((lonTrans - lonMeanTrans).^2)/(length(lonTrans) - 1);

% Standard deviation
latStd = std(latTrans);
lonStd = std(lonTrans);

% Mean squared average
if USE_TRUTH
    len = length(lat);
    latMse = 1/len * sum((latTruth*ones(len,1)' - lat').^2);
    lonMse = 1/len * sum((lonTruth*ones(len,1)' - lon').^2);
    
    if USE_METERS
        latMse = latMse * (LAT_TO_METERS)^2;
        lonMse = lonMse * (LON_TO_METERS)^2;
    else
        latMse = latMse * (LAT_TO_FEET)^2;
        lonMse = lonMse * (LON_TO_FEET)^2;
    end
    
    latMse = sqrt(latMse);
    lonMse = sqrt(lonMse);
end


%% Plotting

% Print the GPS coordinates in a scatter plot
if SHOW_POINTS
    scatter(coords(:,LON_INDEX),coords(:,LAT_INDEX),'CData',[0 0 1]);
end
    
hold on;

% Draw origin
if USE_NEW_ORIGIN
    originSymbol = TRUTH_SYMBOL;
    originColor = TRUTH_COLOR;
    if ~USE_TRUTH
        originSymbol = MEAN_SYMBOL; 
        originColor = MEAN_COLOR;
    end
    scatter(0,0, originSymbol,'CData',originColor,'MarkerFaceColor',originColor);
    
    % Draw mean separate if origin is truth
    if USE_TRUTH
        scatter(lonMeanTrans,latMeanTrans, MEAN_SYMBOL,'CData',MEAN_COLOR,'MarkerFaceColor',MEAN_COLOR);
    end
end

% Draw truth
% if USE_TRUTH
%     transTruthLat = latMean - truth_coordinate{LAT_INDEX};
%     transTruthLon = lonMean - truth_coordinate{LON_INDEX};
%     if USE_MEAN
%         if USE_METERS
%             transTruthLat = transTruthLat* LAT_TO_METERS;
%             transTruthLon = transTruthLon* LON_TO_METERS;
%         else
%             transTruthLat = transTruthLat* LAT_TO_FEET;
%             transTruthLon = transTruthLon* LON_TO_FEET;
%         end
%     end
%     scatter(transTruthLon,transTruthLat,'x','CData',[1 0.5 0.1],'MarkerFaceColor',[1 0.5 0.1]);
% end

%% Labeling
if USE_TRUTH
    title(sprintf('2D GPS Coordinate Scatter Plot\nLat: var = %.2f, std = %.2f, mse =%.2f\nLon: var = %.2f, std = %.2f, mse=%.2f',latVar,latStd,latMse,lonVar,lonStd,lonMse));
else
    title(sprintf('2D GPS Coordinate Scatter Plot\nLat: var = %.2f, std = %.2f\nLon: var = %.2f, std = %.2f',latVar,latStd,lonVar,lonStd));
end

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