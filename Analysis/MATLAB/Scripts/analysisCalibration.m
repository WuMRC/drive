clear, close, clc

%% Select file
[filename, pathname] = uigetfile('*.csv;*.CSV', ...
    'Choose CSV images to work with', pwd, ...
    'MultiSelect', 'off');

addpath(genpath(pathname));
cd(pathname)

% Data to be analyzed
rawData = csvread(filename);

% Voltage divider data for reference
voltageDivider = csvread('50point_voltage_divider.csv');

%%
Rab         = 1227;
Vin         = 0.213;
Vout        = voltageDivider(:,3);
resistanceActual = Rab*(1-Vout/Vin);

step        = rawData(:,1);
frequency   = rawData(:,2)./1000; % Convert to kHz
impedance   = rawData(:,3);
resistance  = rawData(:,4);
reactance   = rawData(:,5);

% Break into separate frequencies
nSteps = 50;
for indFreq = 1:99
    
    % Chunk up the data
    resistanceSheet(:,indFreq) = resistance((indFreq-1)*nSteps+1:indFreq*nSteps);
    reactanceSheet(:,indFreq) = reactance((indFreq-1)*nSteps+1:indFreq*nSteps);
    frequencySheet(:,indFreq) = frequency((indFreq-1)*nSteps+1:indFreq*nSteps);
    
    % Find errors for calibration
    resistanceErrorSheet(:,indFreq) = resistanceSheet(:,indFreq)-resistanceActual;
    resistanceErrorSheetPercent(:,indFreq) = ...
        resistanceErrorSheet(:,indFreq)./resistanceActual*100;
    
    % Find linear fit
    resistanceFit = fit(resistanceActual,resistanceSheet(:,indFreq),'poly1');
    resistanceFit_slope(indFreq) = resistanceFit.p1;
    resistanceFit_intercept(indFreq) = resistanceFit.p2;
    
    % Calibrate given individual fit
    resistanceCorrectedSheet(:,indFreq) = ...
        (resistanceSheet(:,indFreq)  -resistanceFit_intercept(indFreq))/resistanceFit_slope(indFreq);
    
    % Find errors after calibration
    resistanceCorrectedErrorSheet(:,indFreq) = ...
        resistanceCorrectedSheet(:,indFreq) - resistanceActual;
    resistanceCorrectedErrorSheetPercent(:,indFreq) = ...
        resistanceCorrectedErrorSheet(:,indFreq)./resistanceActual*100;
    
end


%% Figure 1a, Errors uncorrected (as ohms)
h1 = pcolor(frequencySheet(1,:),resistanceActual,resistanceErrorSheet);
colormap jet; % Might use parula
hColorbar = colorbar;

fontsize = 20;
set(gca, 'FontSize', fontsize);
set(h1, 'EdgeColor', 'none');
set(hColorbar,'FontSize',fontsize);

xlabel('Frequency [kHz]')
ylabel('Resistance [\Omega]')


%% Figure 1b, Errors uncorrected (as percent)
h1 = pcolor(frequencySheet(1,:),resistanceActual,resistanceErrorSheetPercent);
colormap jet; % Might use parula
hColorbar = colorbar;

fontsize = 20;
set(gca, 'FontSize', fontsize);
set(h1, 'EdgeColor', 'none');
set(hColorbar,'FontSize',fontsize);

xlabel('Frequency [kHz]')
ylabel('Resistance [\Omega]')


%% Figure 2, Slope, intercept, and frequency
h2 = plot3(frequencySheet(1,:),resistanceFit_slope,resistanceFit_intercept,...
    'ko','LineWidth',2);
grid on

% hold on, plot(frequencySheet(1,:),resistanceFit_slope,'o')
% plot3(100*ones(99,1),resistanceFit_slope,resistanceFit_intercept,'o')
% plot3(frequencySheet(1,:),0.995*ones(99,1),resistanceFit_intercept,'o')

fontsize = 20;
set(gca, 'FontSize', fontsize);
xlabel('Frequency [kHz]')
ylabel('Calibration Slope [\Omega/\Omega]')
zlabel('Calibration Intercept [\Omega]')

% Need to fit 3D line function to this

%% Figure 3a, Errors corrected (as ohms)
h1 = pcolor(frequencySheet(1,:),resistanceActual,resistanceCorrectedErrorSheet);
colormap jet; % Might use parula
hColorbar = colorbar;

fontsize = 20;
set(gca, 'FontSize', fontsize);
set(h1, 'EdgeColor', 'none');
set(hColorbar,'FontSize',fontsize);

xlabel('Frequency [kHz]')
ylabel('Resistance [\Omega]')

%% Figure 3b, Errors corrected (as percent)
h1 = pcolor(frequencySheet(1,:),resistanceActual,resistanceCorrectedErrorSheetPercent);
colormap jet; % Might use parula
hColorbar = colorbar;

fontsize = 20;
set(gca, 'FontSize', fontsize);
set(h1, 'EdgeColor', 'none');
set(hColorbar,'FontSize',fontsize);

xlabel('Frequency [kHz]')
ylabel('Resistance [\Omega]')

