% Battery specification model.
%   Makes a discrete plot of run time vs. mAh/V of battery power for 
%   various RC boats.

% Cells represent boats by:
% { Ah_batt (mAh), V_batt (V), T_run (min) }
boats = [ {700,9.6,15}, {} ]

Ah = [ 700 ]; %(mAh)
V = [ 9.6 ]; % (V)

T_run = [15]; % (min)


Y = zeros(length(boats));
% Iterate over boats and order by runtime

for i=1:length(boats),

end


% Discrete plot
stem(T_run
