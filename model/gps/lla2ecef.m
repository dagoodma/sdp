function [ x y z ] = lla2ecef( lat, lon, alt )
%UNTITLED2 Summary of this function goes here
%   Detailed explanation goes here

% Ellipsoid (olbate) constants for coordinate conversions
ECC = 0.0818191908426; % eccentricity
ECC2 =  (ECC*ECC);
ECCP2 = (ECC2 / (1.0 - ECC2)); % square of second eccentricity
FLATR =  (ECC2 / (1.0 + sqrt(1.0 - ECC2))); % flattening ratio

% Radius of earth's curviture on semi-major and minor axes respectively
R_EN =  6378137.0;  % (m) prime vertical radius (semi-major axis)
R_EM =  (R_EN * (1.0 - FLATR)); % meridian radius (semi-minor axis)


DEGREE_TO_RADIAN = pi/180;
sinlat = sin(DEGREE_TO_RADIAN.*lat);
coslat = cos(DEGREE_TO_RADIAN.*lat);

rad_ne = R_EN ./ sqrt(1.0 - (ECC2 .* sinlat .* sinlat));
x = (rad_ne + alt) .* coslat .* cos(lon.*DEGREE_TO_RADIAN);
y = (rad_ne + alt) .* coslat .* sin(lon.*DEGREE_TO_RADIAN);
z = (rad_ne*(1.0 - ECC2) + alt) .* sinlat;



end

