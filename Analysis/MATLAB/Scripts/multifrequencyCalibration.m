% Choose frequency of interest between 2 and 100KHz 
fOfInterest = 2;

% Voltage in
vIn = 0.213;

% End to end resistance of AD5258
rAB = 1227;

% Index for single frequency values
index = 1;

% Import AD5933 data for all frequencies.
% 99 frequency points for 50 resistor values = 4950 values.

fullA = csvread('AD5933.csv',0,2,[0,2,4949,2]);

% Get the size for all the data
sizeFullData = size(fullA);
% Returns a matrix, get the first item off as length of the vector.
sizeFullData = sizeFullData(1);

% Pre-allocate memory
singleA = zeros(sizeAllData / 99, 1);

for i = fOfInterest - 1 : 99 :sizeAllData - 1
    % Get value at single frequency (2 KHz)
    singleA(index) = fullA(i);
    index = index + 1;
end

% Import voltage divider values
vOut =  csvread('voltage_divider.csv',0,2,[0,2,49,2]);

% Calculate resistance from voltage divider
realR = rAB * (1 - (vOut ./ vIn));

% Calculate correction constants
M = (realR(end) - realR(1)) / (singleA(end) - singleA(1));
C = realR(1) - (M * singleA(1));

% Calculate calibrated values at frequency of interest
calA = (M .* singleA) + C;




