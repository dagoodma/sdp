function [ n e d ] = ecef2ned( x, y, z, ref_x, ref_y,ref_z, ref_lat, ref_lon )
%UNTITLED3 Summary of this function goes here
%   Detailed explanation goes here

DEGREE_TO_RADIAN = pi/180;

path_x = x - ref_x;
path_y = y - ref_y;
path_z = z - ref_z;

cosLat = cos(ref_lat .* DEGREE_TO_RADIAN);
sinLat = sin(ref_lat .* DEGREE_TO_RADIAN);
cosLon = cos(ref_lon .* DEGREE_TO_RADIAN);
sinLon = sin(ref_lon .* DEGREE_TO_RADIAN);

% Offset vector from reference and rotate
t =  cosLon .* path_x + sinLon .* path_y;

n = -sinLat .* t + cosLat .* path_z;
e = -sinLon .* path_x + cosLon .* path_y;
d = -(cosLat .* t + sinLat .* path_z);
    
end


