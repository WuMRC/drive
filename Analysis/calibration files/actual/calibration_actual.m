clear, close, clc

%% Import data

% Data to be analyzed
bipolar = importdata('bipolar.csv');
tetrapolar = importdata('tetrapolar.txt');


%% Bipolar analysis
frequency   = bipolar(:,1)./1000; % Convert to kHz
pot2ind     = bipolar(:,2);
pot1ind     = bipolar(:,3);
capacitor   = bipolar(:,4); % in nF
Zmag        = bipolar(:,5);
phase       = bipolar(:,6);


nSteps_p1 = max(pot1ind) - min(pot1ind) + 1;
nSteps_p2 = max(pot2ind) - min(pot2ind) + 1;

nFreq = max(frequency) - min(frequency) + 1;

for indStep_p2 = 1:nSteps_p2
    for indStep_p1 = 1:nSteps_p1
        
        minRange = ((indStep_p2-1)*(indStep_p1-1)*nFreq+1);
        maxRange = (indStep_p2*indStep_p1*nFreq);
        
        ZmagSheet(:,indStep_p1,indStep_p2) ...
            = Zmag(minRange:maxRange);
        phaseSheet(:,indStep_p1,indStep_p2) ...
            = phase(((indStep_p1-1)*nFreq+1):(indStep_p1*nFreq));
    end
end

% Remove outliers
% ZmagSheet(60,6) = NaN; ZmagSheet(60,39) = NaN;
% phaseSheet(60,6) = NaN; phaseSheet(60,39) = NaN;

mesh(ZmagSheet(:,:,1))

%%

x = 1;
for indC = 1:4
    for indR1 = 1:nSteps_p1;
        for indR2 = 1:nSteps_p2;
            for indFreq = 1:nFreq
                x = x+1;
            end
            if x < size(bipolar,1)
                index = x:x+98;
                
                measuredZ(:,indR1,indR2,indC) = Zmag(index);
                measuredPhase(:,indR1,indR2,indC) = phase(index);
            end
        end
    end
end

%% Tetrapolar analysis
capacitor   = tetrapolar(:,1);
pot1ind     = tetrapolar(:,2);
pot2ind     = tetrapolar(:,3);
frequency   = tetrapolar(:,4); % in kHz
Zmag        = tetrapolar(:,5);
phase       = tetrapolar(:,6);

nSteps_p1 = max(pot1ind) - min(pot1ind) + 1;
nFreq = max(frequency) - min(frequency) + 1;
for indStep_p1 = 1:nFreq
    ZmagSheet(:,indStep_p1) = Zmag((indStep_p1-1)*nSteps_p1+1:indStep_p1*nSteps_p1);
    phaseSheet(:,indStep_p1) = phase((indStep_p1-1)*nSteps_p1+1:indStep_p1*nSteps_p1);
end

%%
Rab         = 1227;
Vin         = 0.213;
Vout        = voltageDivider(:,3);
resistanceActual = Rab*(1-Vout/Vin);

step        = tetrapolar(:,1);
frequency   = tetrapolar(:,2)./1000; % Convert to kHz
impedance   = tetrapolar(:,3);
resistance  = tetrapolar(:,4);
reactance   = tetrapolar(:,5);

% Break into separate frequencies
nSteps_p1 = 50;
for indStep_p1 = 1:99
    
    % Chunk up the data
    ZmagSheet(:,indStep_p1) = resistance((indStep_p1-1)*nSteps_p1+1:indStep_p1*nSteps_p1);
    reactanceSheet(:,indStep_p1) = reactance((indStep_p1-1)*nSteps_p1+1:indStep_p1*nSteps_p1);
    frequencySheet(:,indStep_p1) = frequency((indStep_p1-1)*nSteps_p1+1:indStep_p1*nSteps_p1);
    
    % Find errors for calibration
    resistanceErrorSheet(:,indStep_p1) = ZmagSheet(:,indStep_p1)-resistanceActual;
    resistanceErrorSheetPercent(:,indStep_p1) = ...
        resistanceErrorSheet(:,indStep_p1)./resistanceActual*100;
    
    % Find linear fit
    resistanceFit = fit(resistanceActual,ZmagSheet(:,indStep_p1),'poly1');
    resistanceFit_slope(indStep_p1) = resistanceFit.p1;
    resistanceFit_intercept(indStep_p1) = resistanceFit.p2;
    
    % Calibrate given individual fit
    resistanceCorrectedSheet(:,indStep_p1) = ...
        (ZmagSheet(:,indStep_p1)  -resistanceFit_intercept(indStep_p1))/resistanceFit_slope(indStep_p1);
    
    % Find errors after calibration
    resistanceCorrectedErrorSheet(:,indStep_p1) = ...
        resistanceCorrectedSheet(:,indStep_p1) - resistanceActual;
    resistanceCorrectedErrorSheetPercent(:,indStep_p1) = ...
        resistanceCorrectedErrorSheet(:,indStep_p1)./resistanceActual*100;
    
end




%%



R0 = zeros(size(R1));       % preallocate memory, or risk a crash
Rinf = zeros(size(R1));     
tau = zeros(size(R1));
omega = zeros(size(R1));


R0 = R1;
Rinf = (R1.*R2)./(R1+R2);
tau = (C*10^-9).*(R1+R2);

omega = 2*pi*frequency*1000;

predictedResistance = Rinf+((R0-Rinf)./(1+(omega.*tau).^2));
predictedReactance = ((R0-Rinf).*(omega.*tau))./(1+(omega.*tau).^2);

measuredResistance = impedance.*cos(phaseAngle);
measuredReactance = -impedance.*sin(phaseAngle);

% Take a look at the data
plot(predictedResistance(1:99),predictedReactance(1:99));
hold on, plot(measuredResistance(1:99),measuredReactance(1:99),'ko')



%%

%% Calibration 
errorsOverall = zeros(99,50,50,4);

x = 1;
for indC = 1:4
    for indR1 = 1:nSteps_p1;
        for indR2 = 1:nSteps_p2;
            for indFreq = 1:nFreq
                x = x+1;
            end
            if x < size(bipolar,1)
                index = x:x+98;
                
                measuredZ(:,indR1,indR2,indC) = Zmag(index);
            end
        end
    end
end

%%

mesh(measuredPhase(:,:,40,4))
xlabel('Frequency [kHz]')
xlabel('Potentiometer 1 index')
ylabel('Frequency [kHz]')
zlabel('Phase angle [rad]')
title('Phase angle at Pot2 = 40, Cind = 4')