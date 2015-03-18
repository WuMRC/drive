clear, close, clc

%% Select file
[filename, pathname] = uigetfile('*.csv;*.CSV', ...
    'Choose CSV images to work with', pwd, ...
    'MultiSelect', 'off');

addpath(genpath(pathname));
cd(pathname)

% Data to be analyzed
% This is in the form FREQ | R1 | R2 | C | Z | THETA
% And in units Hz | ohms | ohms | nF | ohms | degree
rawData = csvread(filename);

% Voltage divider data for reference
voltageDivider = csvread('50point_voltage_divider.csv');

%% Parse data and establish model
frequency = rawData(:,1)./1000; % kHz are easier to graph
R1 = mean(rawData(:,2)); % There should be a single value throughout
R2 = mean(rawData(:,3));
% C = mean(rawData(:,4)).*10^-9; % I am told the actual value is 4.69
C = 4.69*10^-9;
impedance = rawData(:,5);
theta = rawData(:,6);

% Model values

omega = 2*pi*frequency*1000;
tau = (R1+R2)*C;
R0 = R1;
Rinf = (R1*R2)/(R1+R2);

resistanceModel = Rinf + ((R0-Rinf)./(1 +(omega.*tau).^2));
reactanceModel = ((R0-Rinf).*omega*tau)./(1+(omega*tau).^2);

resistanceMeasured = impedance.*cos(theta);
reactanceMeasured = -impedance.*sin(theta);

%% Figure 1a, Resistance uncalibrated
hold on
semilogx(frequency,resistanceModel,'k-','LineWidth',2)
semilogx(frequency,resistanceMeasured,'ro','LineWidth',2);
hold off

legend('Predicted', 'Measured')
set(gca, 'FontSize', 20);
xlabel('Frequency [kHz]')
ylabel('Resistance [\Omega]')

%% Figure 1b, Reactance uncalibrated
figure
hold on
semilogx(frequency,reactanceModel,'k-','LineWidth',2)
semilogx(frequency,reactanceMeasured,'ro','LineWidth',2);
hold off
legend('Predicted', 'Measured')
set(gca, 'FontSize', 20);

xlabel('Frequency [kHz]')
ylabel('Reactance [\Omega]')
%% Figure 2, Whole situation uncalibrated
figure, hold on, grid on
plot3(frequency,resistanceModel,reactanceModel,'k-','LineWidth',2)
plot3(frequency,resistanceMeasured,reactanceMeasured,'ro','LineWidth',2)
hold off
view(60,30)

legend('Predicted', 'Measured','Location','northeast')
set(gca, 'FontSize', 20);
xlabel('Frequency [kHz]')
ylabel('Resistance [\Omega]')
zlabel('Reactance [\Omega]')
%% Calibration is easy!
% Get fitting parameters
resistanceFit = fit(resistanceModel,resistanceMeasured,'poly1');
reactanceFit = fit(reactanceModel,reactanceMeasured,'poly1');

% Apply fitting parameters
resistanceCalibrated = resistanceMeasured/resistanceFit.p1-resistanceFit.p2;
reactanceCalibrated = reactanceMeasured/reactanceFit.p1-reactanceFit.p2;

%% Figure 3a, Resistance calibrated
figure
hold on
semilogx(frequency,resistanceModel,'k-','LineWidth',2)
semilogx(frequency,resistanceCalibrated,'bo','LineWidth',2);
hold off
legend('Predicted', 'Calibrated')
set(gca, 'FontSize', 20);

xlabel('Frequency [kHz]')
ylabel('Resistance [\Omega]')
%% Figure 3b, Reactance uncalibrated
figure
hold on
semilogx(frequency,reactanceModel,'k-','LineWidth',2)
semilogx(frequency,reactanceCalibrated,'bo','LineWidth',2);
hold off

legend('Predicted', 'Calibrated')
set(gca, 'FontSize', 20);
xlabel('Frequency [kHz]')
ylabel('Reactance [\Omega]')
%% Figure 4, Whole situation calibrated
figure, hold on, grid on
plot3(frequency,resistanceModel,reactanceModel,'k-','LineWidth',2)
plot3(frequency,resistanceCalibrated,reactanceCalibrated,'bo','LineWidth',2)
view(60,30)

legend('Predicted', 'Calibrated','Location','northeast')
set(gca, 'FontSize', 20);
xlabel('Frequency [kHz]')
ylabel('Resistance [\Omega]')
zlabel('Reactance [\Omega]')
%% Figure 5, Errors
% The errors are pretty randomly distributed (though not entirely)
plot3(frequency,...
    resistanceCalibrated-resistanceModel,...
    reactanceCalibrated-reactanceModel,'ko','LineWidth',2)
