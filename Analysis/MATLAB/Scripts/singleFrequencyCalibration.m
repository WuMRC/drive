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

index = 1;

for j = 1 : 100
    
    %M(j) = ((fullRealR(index + 49) - fullRealR(index)) / (fullA(index + 49) - fullA(index)));
    %C(j) = fullRealR(index) - (M(j) * fullA(index));
    
    fitObject = fit(fullA(index:index + 49), fullRealR(index:index + 49), 'poly1');
    
    M(j) = fitObject.p1;
    C(j) = fitObject.p2;
    
    index = index + 50;
end

% Correct AD5933 values

fullAcorrected = zeros(5000, 1);

index = 1;

for k = 1 : 5000
    
    if index == 51
        index = 1;
    end
    
    fullAcorrected(k) = (M(index) * fullA(k)) + C(index);
    
    index = index + 1;
end

fitObject2 = fit(fullA, fullRealR, 'poly1');

fullAcorrected2 = (fitObject2.p1 * fullA) + fitObject2.p2;

%figure,plot(fullRealR, fullA - fullRealR, 'bo', fullRealR, fullAcorrected - fullRealR, 'ro')

figure,plot(fullRealR, fullA - fullRealR, 'bo', fullRealR, fullAcorrected2 - fullRealR, 'ro')

h_legend = legend('Raw AD5933', 'Corrected AD5933');
set(h_legend,'FontSize',20);
set(gca,'FontSize',20);

ylabel('Error [\Omega]','FontSize',20) % x-axis label
xlabel('Actual resistance [\Omega]','FontSize',20) % y-axis label