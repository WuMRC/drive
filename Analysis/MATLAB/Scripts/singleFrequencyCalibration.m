
% Import AD5933 data (50 points, repeated 1000 times)
measuredImpedance = csvread('POT1_50KHZ_1000REPS.csv',0,2,[0,2,49999,2]);

% Import predicted impedance values from voltage divider
predictedImpedance =  csvread('POT1_POT2_RESISTANCE_1000REPS.csv',0,1,[0,1,49999,1]);

% Import steps
steps =  csvread('POT1_POT2_RESISTANCE_1000REPS.csv',0,0,[0,0,49999,0]);

% Initialize slope and y intercept vectors
M = zeros(1000,1);
C = zeros(1000,1);

% Fill in values for M and C

index = 1;

for j = 1 : 1000
        
    fitObjectLocal = fit(measuredImpedance(index:index + 49), predictedImpedance(index:index + 49), 'poly1');
    
    M(j) = fitObjectLocal.p1;
    C(j) = fitObjectLocal.p2;
    
    index = index + 50;
end

% Correct AD5933 values

correctedImpedanceLocal = zeros(50000, 1);

index = 1;

for k = 1 : 50000
    
    if index == 51
        index = 1;
    end
    
    correctedImpedanceLocal(k) = (M(index) * measuredImpedance(k)) + C(index);
    
    index = index + 1;
end

fitObjectGlobal = fit(measuredImpedance, predictedImpedance, 'poly1');

correctedImpedanceGlobal = (fitObjectGlobal.p1 * measuredImpedance) + fitObjectGlobal.p2;

figure,plot(predictedImpedance, measuredImpedance - predictedImpedance, 'bo', predictedImpedance, correctedImpedanceLocal - predictedImpedance, 'ro')

figure,plot(predictedImpedance, measuredImpedance - predictedImpedance, 'bo', predictedImpedance, correctedImpedanceGlobal - predictedImpedance, 'ro')

h_legend = legend('Raw AD5933', 'Corrected AD5933');
set(h_legend,'FontSize',20);
set(gca,'FontSize',20);

ylabel('Error [\Omega]','FontSize',20) % x-axis label
xlabel('Actual resistance [\Omega]','FontSize',20) % y-axis label