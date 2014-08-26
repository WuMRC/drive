clear all; close all; clc
%%
clear NAMES dZOverall
[bioimpedanceFile, bioimpedancePath] = uigetfile('*.*','Pick an ACQ file');
acq = load_acq(strcat(bioimpedancePath,bioimpedanceFile));

cd(bioimpedancePath);

ohmsPerVolt = 20;

bArm = acq.data(:,1)*ohmsPerVolt;
bLeg = acq.data(:,2)*ohmsPerVolt;
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
totalTime = 10; % seconds
offset = 0;
time = offset:dt:totalTime;


bLegSMOOTH = smooth(bLeg,Fs/2);
bLegSMOOTH = smooth(bLegSMOOTH, Fs/10);
respLegSMOOTH = smooth(bLegSMOOTH, Fs);
cardLegSMOOTH = smooth(bLegSMOOTH - respLegSMOOTH);

bArmSMOOTH = smooth(bArm,Fs/2);
bArmSMOOTH = smooth(bArmSMOOTH, Fs/10);
respArmSMOOTH = smooth(bArmSMOOTH, Fs);
cardArmSMOOTH = smooth(bArmSMOOTH - respArmSMOOTH);

totalData = [bLegSMOOTH, respLegSMOOTH, cardLegSMOOTH, ...
    bArmSMOOTH, respArmSMOOTH, cardArmSMOOTH];

for indMarker = 1%:size(timeMarkerBioimpedance,2)-1;    
    % The time of the region of interest
    tBegin = timeMarkerBioimpedanceInd(indMarker)+Fs*offset;
    tEnd = timeMarkerBioimpedanceInd(indMarker)+Fs*totalTime;
    
    % Signal during the time of interest (maneuver)
    for indDataType = 1:size(totalData,2)
        maxOverall(indMarker,indDataType) = ...
            max(totalData(tBegin:tEnd,indDataType));
        minOverall(indMarker,indDataType) = ...
            min(totalData(tBegin:tEnd,indDataType));
        dZOverall(indMarker,indDataType) = ...
            maxOverall(indMarker,indDataType) - minOverall(indMarker,indDataType);
        
        NAMES{indMarker,1} = acq.markers.szText{1,indMarker+1}(12:end);
    end
%     open NAMES
%     open dZOverall
    

subplot(3,1,1), plot(time, totalData(tBegin:tEnd,4))
subplot(3,1,2), plot(time, totalData(tBegin:tEnd,5))
subplot(3,1,3), plot(time, totalData(tBegin:tEnd,6))




%     acq.markers.szText{1,indMarker+1}(12:end)
    
%     plot(legData(tBegin:tEnd,3))
%     
%     figure, plot(time', legROI,time(indMaxLeg),maximaLeg,'g*',time(indMinL),legROI(indMinL),'ro')
%     title(horzcat('Leg ',acq.markers.szText{1,indMarker+1}(12:end)));
%     xlabel('Time [s]');
%     ylabel('Impedance [\Omega]');
    
end

% plot(time,bLegSMOOTH(tBegin:tEnd))


