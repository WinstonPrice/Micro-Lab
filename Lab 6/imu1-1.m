% imu1.m
%
% Reads in some IMU data and processes it.
%
% Seth McNeill
% 2022 February 15

clear
close all

tickFontSize = 16;
labelFontSize = 20;
titleFontSize = 26;
gridAlphaSet = 0.6;

data = readtable("fastTranslate.csv"); % read in the data file as a table
startTime = data(1,'millis');
startTime = startTime{:,:};  % convert the first time point to a number
millis = data(:,'millis');
millis = millis{:,:}/1000; % the time vector as an array in seconds
dt = diff(millis(2:end))/1000;
ay = data(:,'AY');
ay = ay{:,:}; % acceleration in the y direction in g's as an array
ay_z = [0; (ay(2:end) - ay(1))]*9.81;  % m/s^2, subtract the first point to zero

% do the summation in a for loop for easy conversion to Arduino
v = zeros(length(millis),1);
s = v;
for ii = 2:length(millis)
    dt = millis(ii) - millis(ii-1);
    v(ii) = v(ii-1) + ay_z(ii-1)*dt;
    s(ii) = s(ii-1) + v(ii-1)*dt + 0.5*ay_z(ii-1)*dt^2;
end

figure('PaperOrientation', 'landscape', 'PaperUnits', 'normalized', 'PaperPosition', [0 0 1 1])
plot(millis, ay_z/9.81, 'linewidth', 2)
hold all
plot(millis, v, 'linewidth', 2)
plot(millis, s, 'linewidth', 2)
grid on
legend('Acceleration (g)', 'Velocity (m/s)', 'Displacement (m)')
title('Linear Position Estimation', 'fontsize', titleFontSize)
xlabel('Time (s)', 'fontsize', labelFontSize)
ylabel('Various', 'fontsize', labelFontSize)    
myaxis = gca;
myaxis.FontSize = tickFontSize; 
myaxis.GridAlpha = gridAlphaSet;
