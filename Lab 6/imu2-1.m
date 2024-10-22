% imu2.m
%
% Tries to calculate angles based on accelerometer and gyroscope data
%
% Seth McNeill
% 2022 February 18

clear
close all

tickFontSize = 16;
labelFontSize = 20;
titleFontSize = 26;
gridAlphaSet = 0.6;

data = readtable("rotate1.csv");
startTime = data(1,'millis');
startTime = startTime{:,:};  % convert to a number
millis = data(:,'millis');
millis = millis{:,:}/1000;
ax = data(:,'AX');
az = data(:,'AZ');
ax = ax{:,:};
az = az{:,:};
gy = data(:,'GY');
gy = gy{:,:};

figure('PaperOrientation', 'landscape', 'PaperUnits', 'normalized', 'PaperPosition', [0 0 1 1])
subplot(2,1,1)
plot(millis,ax,millis,az, 'linewidth', 2)
title('Raw Accelerometer Data', 'fontsize', titleFontSize)
grid on
legend('Ax','Az','Location','se')
xlabel('Time', 'fontsize', labelFontSize)
ylabel('Acceleration (g)', 'fontsize', labelFontSize)
myaxis = gca;
myaxis.FontSize = tickFontSize; 
myaxis.GridAlpha = gridAlphaSet;

subplot(2,1,2)
plot(millis, gy, 'linewidth', 2)
grid on
title('Raw Gyroscope Data (Y-Axis)', 'fontsize', titleFontSize)
xlabel('Time', 'fontsize', labelFontSize)
ylabel('Degrees/Second', 'fontsize', labelFontSize)
myaxis = gca;
myaxis.FontSize = tickFontSize; 
myaxis.GridAlpha = gridAlphaSet;

%set(gcf, 'PaperPosition', [0 0 11 8.5])
saveas(gcf, 'RawData.png')

accelAngle = rad2deg(atan2(az, ax));
amix = 1;

mixedAngle = zeros(1,length(millis));
% gyro angle calculated with for loop to test algorithm for onboard Nano
gyroAngle = zeros(1,length(millis));
mixedAngle(1) = accelAngle(1);
gyroAngle(1) = accelAngle(1); % initialize them both to the same point
for ii = 2:length(millis)
    gyroAngle(ii) = gyroAngle(ii-1) + (millis(ii)-millis(ii-1))*gy(ii-1);
    mixedAngle(ii) = amix*(mixedAngle(ii-1) + (millis(ii)-millis(ii-1))*gy(ii-1)) + (1 - amix)*accelAngle(ii);
%    mixedAngle(ii) = amix*(gyroAngle(ii-1) +
%    (millis(ii)-millis(ii-1))*gy(ii-1)) + (1 - amix)*accelAngle(ii); %
%    wrong way
end

figure('PaperOrientation', 'landscape', 'PaperUnits', 'normalized', 'PaperPosition', [0 0 1 1])
plot(millis,accelAngle, 'linewidth', 2)
hold on
plot(millis,gyroAngle, 'linewidth', 2)
plot(millis, mixedAngle, 'color','#7E2F8E','linewidth', 2)
grid on
legend('Accelerometer', 'Gyroscope', "Mixed (\alpha="+amix+")")
title('Angle Estimation via Accelerometer and Gyroscope', 'fontsize', titleFontSize)
xlabel('Time', 'fontsize', labelFontSize)
ylabel('Angle (degrees)', 'fontsize', labelFontSize)    
myaxis = gca;
myaxis.FontSize = tickFontSize; 
myaxis.GridAlpha = gridAlphaSet;
% create zoomed plot inside main plot
axes('position', [0.41 0.7 0.2 0.2])
box on
indInt = (millis < 24.5) & (millis > 22.5); % range of interest
plot(millis(indInt),accelAngle(indInt), 'linewidth', 2)
hold on
plot(millis(indInt),gyroAngle(indInt), 'linewidth', 2)
plot(millis(indInt), mixedAngle(indInt), 'color','#7E2F8E', 'linewidth', 2)
grid on
myaxis = gca;
myaxis.GridAlpha = gridAlphaSet;
saveas(gcf, 'EstimatedAngle.png')
