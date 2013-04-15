function [ coord_n, coord_e, coord_d ] = projectCoordinate( yaw, pitch, height )
%PROJECTCOORDINATE Summary of this function goes here
%   Detailed explanation goes here
DEGREE_TO_RADIAN = pi/180.0;

mag = height * tan((90.0-pitch)*DEGREE_TO_RADIAN);

if (yaw <= 90.0)
    % First quadrant
    coord_n = mag * cos(yaw*DEGREE_TO_RADIAN);
    coord_e = mag * sin(yaw*DEGREE_TO_RADIAN);
elseif (yaw > 90.0 && yaw <= 180.0)
    % Second quadrant
    yaw = yaw - 270.0;
    coord_n = mag * sin(yaw*DEGREE_TO_RADIAN);
    coord_e = -mag * cos(yaw*DEGREE_TO_RADIAN);

elseif (yaw > 180.0 && yaw <= 270.0)
    % Third quadrant
    yaw = yaw - 180.0;
    coord_n = -mag * cos(yaw*DEGREE_TO_RADIAN);
    coord_e = -mag * sin(yaw*DEGREE_TO_RADIAN);
elseif (yaw > 270 < 360.0)
    % Fourth quadrant
    yaw = yaw - 90.0;
    coord_n = -mag * sin(yaw*DEGREE_TO_RADIAN);
    coord_e = mag * cos(yaw*DEGREE_TO_RADIAN);
end


coord_d = height;

end

