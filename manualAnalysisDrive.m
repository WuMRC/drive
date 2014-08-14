
% Get DICOM for analysis
[dicomImageFile.filename, dicomImageFile.pathname] = ...
    uigetfile('*.dcm,*.DCM','Pick a DICOM file');
addpath(genpath(dicomImageFile.pathname))

dicomImage = dicomread(dicomImageFile.filename);
dicomImage = permute(dicomImage, [1 2 4 3]);

% Display
imtool3D(dicomImage)

%%
bioimpedanceFile = uigetfile('*.*','Pick an ACQ file');
acq = load_acq(bioimpedanceFile);

ohmsPerVolt = 20;

bioimpedanceArm = acq.data(:,1)*ohmsPerVolt;
bioimpedanceLeg = acq.data(:,2)*ohmsPerVolt;
biomipedanceChest = acq.data(:,3)*ohmsPerVolt;
bioimpedanceForearm = acq.data(:,4)*ohmsPerVolt;

dtBioimpedance = acq.hdr.graph.sample_time;
timeStartBioimpedance = acq.hdr.graph.first_time_offset/1000;

day = 60*60*24;
timeStartBioimpedance6 = str2double(datestr(timeStartBioimpedance/day,'HHMMSS'));

shift = 0;
% Find markers in ACQKnowledge data
timeMarkerBioimpedance = zeros(1,length(acq.markers.lSample));
timeMarkerBioimpedanceInd = zeros(1,length(acq.markers.lSample));
for nMarkers = 1:length(acq.markers.lSample)
    if acq.markers.lSample(nMarkers) == 0
        shift = shift + 1;
    else
        x = str2double(datestr(timeStartBioimpedance/day+...
            (double(acq.markers.lSample(nMarkers)/200)/day),'HHMMSS'));
        timeMarkerBioimpedance(nMarkers-shift) = x;
        clipName{1,nMarkers-shift} = acq.markers.szText{1,nMarkers}(11:end);
        timeMarkerBioimpedanceInd(nMarkers-shift) = acq.markers.lSample(nMarkers);
    end
end
clear x

timeMarkerBioimpedance = unique(timeMarkerBioimpedance);
clipName = unique(clipName);

% Impedance analysis
Fs = 200;
dt = 1/Fs;
totalTime = 12; % second
offset = 0.75;

timeBioimpedance = offset:dt:totalTime;

bioimpedanceLegSMOOTH = smooth(bioimpedanceLeg,Fs/2).*20;
bioimpedanceLegSMOOTH = smooth(bioimpedanceLegSMOOTH, 20);
respLegSMOOTH = smooth(bioimpedanceLegSMOOTH, 200);
cardLegSMOOTH = smooth(bioimpedanceLegSMOOTH - respLegSMOOTH);

bioimpedanceArmSMOOTH = smooth(bioimpedanceArm,Fs/2).*20;
bioimpedanceArmSMOOTH = smooth(bioimpedanceArmSMOOTH, 20);






DATA_TO_ANALYZE = 1;
% LEG DATA
figure, subplot(3,1,1), plot(timeBioimpedance', ...
    bioimpedanceLegSMOOTH(timeMarkerBioimpedanceInd(DATA_TO_ANALYZE)+Fs*offset:...
    (timeMarkerBioimpedanceInd(DATA_TO_ANALYZE)+Fs*totalTime)) ...
    - mean(bioimpedanceLegSMOOTH(timeMarkerBioimpedanceInd(DATA_TO_ANALYZE)+Fs*offset:...
    (timeMarkerBioimpedanceInd(DATA_TO_ANALYZE)+Fs*totalTime))))
title(clipName(DATA_TO_ANALYZE))

subplot(3,1,2), plot(timeBioimpedance', ...
    respLegSMOOTH(timeMarkerBioimpedanceInd(DATA_TO_ANALYZE)+Fs*offset:...
    (timeMarkerBioimpedanceInd(DATA_TO_ANALYZE)+Fs*totalTime)) ...
    - mean(respLegSMOOTH(timeMarkerBioimpedanceInd(DATA_TO_ANALYZE)+Fs*offset:...
    (timeMarkerBioimpedanceInd(DATA_TO_ANALYZE)+Fs*totalTime))))
title('Respiratory Signal')

subplot(3,1,3), plot(timeBioimpedance', ...
    cardLegSMOOTH(timeMarkerBioimpedanceInd(DATA_TO_ANALYZE)+Fs*offset:...
    (timeMarkerBioimpedanceInd(DATA_TO_ANALYZE)+Fs*totalTime)) ...
    - mean(cardLegSMOOTH(timeMarkerBioimpedanceInd(DATA_TO_ANALYZE)+Fs*offset:...
    (timeMarkerBioimpedanceInd(DATA_TO_ANALYZE)+Fs*totalTime))))
title('Cardiac Signal')



