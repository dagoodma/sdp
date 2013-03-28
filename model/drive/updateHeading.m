function [ leftPWM, rightPWM, thetaError ] = updateHeading( currentHeading, desiredHeading,...
        lastThetaError, velocity)
%UPDATEHEADING Summary of this function goes here
%   Detailed explanation goes here

HEADING_UPDATE_DELAY = 250; % (ms)
KP = 1; KD = 1;
thetaError = desiredHeading - currentHeading;
thetaErrorDerivative = (thetaError - lastThetaError)/HEADING_UPDATE_DELAY;

% Calculate Compensator's Ucommand
Ucmd = KP*(thetaError) + KD*(thetaErrorDerivative);

% Set PWM's: we have a ratio of 3:1 when Pulsing the motors given a Ucmd
if (thetaError < 180)
    % Turning Left
    rightPWM = Ucmd*velocity;
    leftPWM = ((1/3)*Ucmd)*velocity;
elseif (thetaError > 180)
    leftPWM = Ucmd*velocity;
    rightPWM = ((1/3)*Ucmd)*velocity;
end

lastThetaError = thetaError;

end

