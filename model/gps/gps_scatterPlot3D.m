function [ coords ] = gps_scatterPlot3D( filename )
%GPS_SCATTERPLOT3D Creates a 3D scatter plot of GPS data from a specified 
%   DLM file.
%   
%   ie. 
%       gps_scatterPlot3D('data\2013.01.29-154816_ublox2_geodetic.dlm')

coords = dlmread(filename);


scatter3(coords(:,1),coords(:,2),coords(:,3))
title('3D GPS Coordinate Scatter Plot')

end