
% Import AD5933 data (50 points, repeated 100 times for 99 frequencies)
measuredImpedance = csvread('2-100KHZ_100REPS.csv',0,2,[0,2,494999,2]);

% Import predicted impedance values from voltage divider
predictedImpedance =  csvread('POT1_POT2_RESISTANCE_1000REPS.csv',0,1,[0,1,49999,1]);

% Initialize slope and y intercept vectors
M = zeros(99,1);
C = zeros(99,1);

% Fill in values for M and C for frequencies of interest

index = 1;

for i = 1 : 99 % 2KHz - 100 KHz
    
    %50 points at 100 repetition gives 5000 points for each frequency
    fitObjectSingleFrequency = fit(measuredImpedance(index:index + 4999), predictedImpedance(1:5000), 'poly1');
    
    M(i) = fitObjectSingleFrequency.p1;
    C(i) = fitObjectSingleFrequency.p2;
    
    index = index + 5000;
end

