% End to end resistance of AD5258
rAB = 1227;

% Import AD5933 data (50 points, repeated 100 times)
fullA = csvread('50KHZ_100REPS.csv',0,2,[0,2,4999,2]);

% Voltage in
vIn = 0.213;

% Import voltage divider values
vOut =  csvread('50point_voltage_divider.csv',0,2,[0,2,49,2]);

% Calculate actual resistance
realR = rAB * (1 - (vOut ./ vIn));

% Calculate step
step(:,1) = 1:50;

% Vertically concatenate realR 100 times
fullRealR = zeros(5000, 1);
fullStep = zeros(5000, 1);

index = 1;

for i = 1 : 5000
    
    if index == 51
        index = 1;
    end
    
    fullRealR(i) = realR(index);
    fullStep(i) = step(index);
    
    index = index + 1;
end

% Initialize slope and y intercept vectors
M = zeros(100,1);
C = zeros(100,1);

% Fill in values for M and C
largeIndex = 1;

for j = 1 : 100
    
    M(j) = (fullRealR(largeIndex + 49) - fullRealR(largeIndex)/ fullA(largeIndex + 49) - fullA(largeIndex));
    C(j) = fullRealR(largeIndex) - (M(j) * fullA(largeIndex));
    
    largeIndex = largeIndex + 50;
end








