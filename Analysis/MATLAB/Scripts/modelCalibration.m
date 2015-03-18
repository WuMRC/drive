R1 =  csvread('POT1_POT2_RESISTANCE_1000REPS.csv',0,1,[0,1,49999,1]);
R2 =  csvread('POT1_POT2_RESISTANCE_1000REPS.csv',0,2,[0,2,49999,2]);

Re = mean(R1(20:50:end)); % Get value at wiper position 33 (R1 indexing started at 14)
Ri = mean(R2(43:50:end)); % Get value at wiper position 56 (R1 indexing started at 14)
R0 = Re;
Rinf = (Ri * Re) / (Ri + Re);

frequency(:,1) = 2000:1000:100000;
w = 2 * pi * frequency;
Capacitance = 4.7 * 10^-9;
tau = Capacitance * (Ri + Re);

predictedResistance = Rinf + ((R0 - Rinf) ./ (1 + (w * tau).^2));
predictedReactance = ((R0 - Rinf) * (w * tau)) ./ (1 + (w * tau).^2);

measuredZ = csvread('MODEL_NOREPS.csv',0,4,[0,4,98,4]);
measuredP = csvread('MODEL_NOREPS.csv',0,5,[0,5,98,5]);

measuredR = measuredZ .* cos(measuredP);
measuredX = -1 * measuredZ .* sin(measuredP);

correctedZ = measuredZ;
correctedP = measuredP;

for i = 1 : 99 % 2KHz - 100 KHz
    correctedZ(i) = (M(i) * measuredZ(i)) + C(i); % Correct at individual frequencies
    correctedP(i) = measuredP(i);

end

correctedR = correctedZ .* cos(correctedP);
correctedX = -1 * correctedZ .* sin(correctedP);

plot(predictedResistance,predictedReactance,'bo',...
    measuredR,measuredX,'ro',...
    correctedR,correctedX,'go')