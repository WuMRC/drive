
%% Import potentiometer data
rawData = csvread('POT1_POT2_RESISTANCE_1000REPS.csv');

step        = rawData(:,1);
r1          = rawData(:,2); % Convert to kHz
r2          = rawData(:,3);

nOfPotValues = 50;
nOfPotReps = size(step) / nOfPotValues;
nOfPotReps = nOfPotReps(1); % Get first item off the array returned

r1Sheet = zeros(nOfPotReps, nOfPotValues);
r2Sheet = zeros(nOfPotReps, nOfPotValues);

r1SheetAverage = zeros(1, nOfPotValues);
r2SheetAverage = zeros(1, nOfPotValues);

% Break into separate potentiometer steps
for potIndex = 1:nOfPotValues
    r1Sheet(:,potIndex) = r1(potIndex : nOfPotValues : end);
    r2Sheet(:,potIndex) = r2(potIndex : nOfPotValues : end);
end

% Find the average value
for potIndex = 1:50
    r1SheetAverage(:,potIndex) = mean(r1Sheet(:,potIndex));
    r2SheetAverage(:,potIndex) = mean(r2Sheet(:,potIndex));
end

% Transpose to a column vector
r1SheetAverage = r1SheetAverage.';
r2SheetAverage = r2SheetAverage.';

%% Import model data

for noOfRuns = 1 : 2
    
    rawFile = strcat('RAW_', noOfRuns, '_MODEL_4_50_50_99.csv');
    
    fileName = strcat('FINAL_', noOfRuns, '_MODEL_4_50_50_99.csv');
    
    outputDir = fullfile('Users', 'Shemanigans', 'Google Drive', 'ShihLab',...
        'Analysis', 'calibration files', strcat('Model Run ', noOfRuns));
    
    
    rawData = csvread(rawFile);
    
    frequency           = rawData(:,1)./ 1000; % Convert to kHz
    R1                  = rawData(:,2);
    R2                  = rawData(:,3);
    C                   = rawData(:,4);
    impedance           = rawData(:,5);
    phaseAngle          = rawData(:,6);
    
    % Replace potentiometer index with actual resistance values
    for index = 1:size(R1)
        R1(index) = r1SheetAverage(R1(index) + 1); % Change index to actual resistance value
        R2(index) = r2SheetAverage(R2(index) + 1); % Change index to actual resistance value
    end
    
    javaImport = horzcat(R1, R2, rawData);
    
    %csvwrite('javaImport', javaImport)
    
    csvwrite(fullfile(outputDir, fileName), javaImport);
       
end