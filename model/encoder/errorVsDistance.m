% Error in feet vs distance at various heights for a 14-bit encoder

height = [10 15 20 25];
d = [100:100:1000];
res = 14;

% Yaw error
yawError = d.*tan(2*pi/(2^res));

figure(1); clf;
plot(d,yawError)
ylabel('Uncertainty (ft)');
xlabel('Distance (ft)');
title(sprintf('Yaw Uncertainty vs. Distance\n(14-bit encoder)'));

%error = height(1)*TAN((B12+360/2^A14)*PI()/180)-B11
% Pitch error
pitch = zeros(length(height),length(d));
pitchError = zeros(length(height),length(d));
figure(2); clf;
colors = {'r', 'g', 'b', 'm'};
legendmatrix = cell(1,length(height));
for i=1:length(height)
    pitch(i,:) = atan(d./height(i))*180/pi;
    for j=1:length(d)
        pitchError(i,j) = height(i) .* tan((pitch(i,j)' + 360/2^res).*pi/180) - d(j);
    end
    hold on;
    plot(d,[pitchError(i,:)]',colors{i})
    text(800,10.5+(5-i)^2.1,sprintf('%d ft',height(i)))
    legendmatrix{i} = sprintf('height: %d ft',height(i));
end
hold off;
legend(legendmatrix,'Location','NorthWest')
ylabel('Uncertainty (ft)');
xlabel('Distance (ft)');
title(sprintf('Pitch Uncertainty vs. Distance\n(14-bit encoder)'));
box on;