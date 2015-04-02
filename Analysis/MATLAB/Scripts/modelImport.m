rawData = csvread('MODEL_4_50_50_99.csv');

frequency           = rawData(:,1)./ 1000; % Convert to kHz
R1                  = rawData(:,2);
R2                  = rawData(:,3);
C                   = rawData(:,4);
impedance           = rawData(:,5);
phaseAngle          = rawData(:,6);

for index = 1:size(R1)
    R1(index) = r1SheetAverage(R1(index) + 1);
    R2(index) = r2SheetAverage(R2(index) + 1);
end