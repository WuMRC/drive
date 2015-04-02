
clear, close, clc
%% Import data
load('calibrationData(4-50-50-99)-033115.mat')


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

%% Removing most outliers
% Resistance errors of e<10 and e>45 were seen to be random outlying errors
errorsResistance = measuredResistance-predictedResistance;
measuredResistance(errorsResistance<10 | errorsResistance>45) = NaN;

nanR = find(isnan(measuredResistance)==1);
dataR = find(isnan(measuredResistance)==0);

wR = interp1(dataR,measuredResistance(dataR),nanR,'spline');
measuredResistance(nanR) = wR;

% Reactance errors of e<-83 and e>13 were seen to be random outlying errors
errorsReactance = measuredReactance-predictedReactance;
measuredReactance(errorsReactance<-83 | errorsReactance>13) = NaN;

nanX = find(isnan(measuredReactance)==1);
dataX = find(isnan(measuredReactance)==0);

wX = interp1(dataX,measuredReactance(dataX),nanX,'spline');
measuredReactance(nanX) = wX;




%% Calibration 
errorsOverall = zeros(99,50,50,4);

x = 1;
h = waitbar(0);
for indC = 1:4
    for indR1 = 1:50;
        for indR2 = 1:50;
            for indFreq = 1:99
                
%                 % For TJ if he wants to see the 4D error space
%                 errorsOverall(indFreq,indR2,indR1,indC) = ...
%                     errorsResistance(x) + 1i*errorsReactance(x);
                x = x+1;
                
            end
            
            if x < size(impedance,1)
                index = x:x+98;
                
                resistanceFit = fit(predictedResistance(index),...
                    measuredResistance(index),'poly1');
                resistanceFit_slope(indR2,indR1,indC) = resistanceFit.p1;
                resistanceFit_intercept(indR2,indR1,indC) = resistanceFit.p2;
                
                reactanceFit = fit(predictedReactance(index),...
                    measuredReactance(index),'poly1');
                reactanceFit_slope(indR2,indR1,indC) = reactanceFit.p1;
                reactanceFit_intercept(indR2,indR1,indC) = reactanceFit.p2;
                
                
                
                waitbar(x/size(impedance,1),h)
            end
            
        end
    end
end
close(h)
%%

for indC = 1:4
    subplot(2,4,indC*2-1), pcolor(resistanceFit_slope(:,:,indC))
    xlabel('R1'), ylabel('R2')
    title(strcat('Calibration slope for C = ',num2str(C(indC*(990000/4)))))
    colorbar('SouthOutside')
    caxis([0.95 1.25])
    subplot(2,4,indC*2), pcolor(resistanceFit_intercept(:,:,indC))
    xlabel('R1'), ylabel('R2')
    title(strcat('Calibration intercept for C = ',num2str(C(indC*(990000/4)))))
    colorbar('SouthOutside')
    caxis([-15 50])
end

%%

