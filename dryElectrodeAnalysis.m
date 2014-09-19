clear all; close all; clc
%%
clear NAMES dZOverall
[bioimpedanceFile, bioimpedancePath] = uigetfile('*.*','Pick an ACQ file');
acq = load_acq(strcat(bioimpedancePath,bioimpedanceFile));

cd(bioimpedancePath);


%%
ohmsPerVolt = 20;

bArm = acq.data(:,1)*ohmsPerVolt;
ppg = acq.data(:,3); 


dtBioimpedance = acq.hdr.graph.sample_time;
timeStartBioimpedance = acq.hdr.graph.first_time_offset/1000;

day = 60*60*24;
timeStartBioimpedance = str2double(datestr(timeStartBioimpedance/day,'HHMMSS'));

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
%         clipName{1,nMarkers-shift} = acq.markers.szText{1,nMarkers}(11:end);
        timeMarkerBioimpedanceInd(nMarkers-shift) = acq.markers.lSample(nMarkers);
    end
end
clear x

timeMarkerBioimpedance = unique(timeMarkerBioimpedance);

Fs = 200;
dt = 1/Fs;
% totalTime = 10; % seconds
% offset = 0;
% time = offset:dt:totalTime;


bArmSMOOTH = smooth(bArm,Fs/2);
bArmSMOOTH = smooth(bArmSMOOTH, Fs/10);
respArmSMOOTH = smooth(bArmSMOOTH, Fs);
cardArmSMOOTH = smooth(bArmSMOOTH - respArmSMOOTH);

time = (0:length(bArmSMOOTH)-1).*dt;

totalData = [bArmSMOOTH, respArmSMOOTH, cardArmSMOOTH, ppg];

% figure
% subplot(2,1,1), plot(time, cardArmSMOOTH)
% subplot(2,1,2), plot(time, ppg)



bArmNoise = bArmSMOOTH - bArm;
plot(time, bArmNoise)

