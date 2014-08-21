[bioimpedanceFile, bioimpedancePath] = uigetfile('*.*','Pick an ACQ file');
acq = load_acq(strcat(bioimpedancePath,bioimpedanceFile));

%%
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

%%
Fs = 200;
dt = 1/Fs;
totalTime = 15; % seconds
offset = 0.75;
time = offset:dt:totalTime;


bLegSMOOTH = smooth(bLeg,Fs/2);
bLegSMOOTH = smooth(bLegSMOOTH, Fs/10);
respLegSMOOTH = smooth(bLegSMOOTH, Fs);
cardLegSMOOTH = smooth(bLegSMOOTH - respLegSMOOTH);

bArmSMOOTH = smooth(bArm,Fs/2);
bArmSMOOTH = smooth(bArmSMOOTH, Fs/10);
respArmSMOOTH = smooth(bArmSMOOTH, Fs);
cardArmSMOOTH = smooth(bArmSMOOTH - respArmSMOOTH);

legData = [bLegSMOOTH, respLegSMOOTH, cardLegSMOOTH];

for indMarker = 1:size(timeMarkerBioimpedance,2)-1;    
    % The time of the region of interest
    tBegin = timeMarkerBioimpedanceInd(indMarker)+Fs*offset;
    tEnd = timeMarkerBioimpedanceInd(indMarker)+Fs*totalTime;
    
    % Signal during the time of interest (manuever)
    legROI = bLegSMOOTH(tBegin:tEnd) - mean(bLegSMOOTH(tBegin:tEnd));
    respLegROI = respLegSMOOTH(tBegin:tEnd) - mean(respLegSMOOTH(tBegin:tEnd));
    cardLegROI = cardLegSMOOTH(tBegin:tEnd) - mean(cardLegSMOOTH(tBegin:tEnd));
    armROI = bArmSMOOTH(tBegin:tEnd) - mean(bArmSMOOTH(tBegin:tEnd));
    respArmROI = respArmSMOOTH(tBegin:tEnd) - mean(respArmSMOOTH(tBegin:tEnd));
    cardArmROI = cardArmSMOOTH(tBegin:tEnd) - mean(cardArmSMOOTH(tBegin:tEnd));
    
    % Overall signal changes
    [legMax, indLegMax] = findpeaks(legROI,'MINPEAKDISTANCE',Fs/2);
    [armMax, indArmMax] = findpeaks(armROI,'MINPEAKDISTANCE',Fs/2);
    legROIinv = 1.01*max(legROI) - legROI;
    armROIinv = 1.01*max(armROI) - armROI;
    [legMin, indLegMin] = findpeaks(legROIinv,'MINPEAKDISTANCE',Fs/2);
    [armMin, indArmMin] = findpeaks(armROIinv,'MINPEAKDISTANCE',Fs/2);
    
    maxOverallLeg = max(legROI); maxOverallArm = max(armROI);
    minOverallLeg = min(legROI); minOverallArm = min(armROI);
    
    dZOverallLeg(indMarker) = maxOverallLeg - minOverallLeg;
    dZOverallArm(indMarker) = maxOverallArm - minOverallArm;
    
    
    
%     figure, plot(time', legROI,time(indMaxLeg),maximaLeg,'g*',time(indMinL),legROI(indMinL),'ro')
%     title(horzcat('Leg ',acq.markers.szText{1,indMarker+1}(12:end)));
%     xlabel('Time [s]');
%     ylabel('Impedance [\Omega]');
    
end

