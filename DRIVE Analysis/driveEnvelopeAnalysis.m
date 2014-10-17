clear all; close all; clc
%%

[file, path] = uigetfile('*.*','Pick an ACQ file');
totalData = getImpedance(path, file);
timeMarkers = getACQmarkers(path, file);

Fs = 200;
dt = 1/Fs;

totalTime = 10; % seconds
offset = 1;
time = offset:dt:totalTime;

%%
for indMarker = 5%:size(timeMarkerBioimpedance,2)-1;    
    % The time of the region of interest
    tBegin = timeMarkers(indMarker)+Fs*offset;
    tEnd = timeMarkers(indMarker)+Fs*totalTime;
    
    armROI = totalData(tBegin:tEnd,4);
    armRespROI = totalData(tBegin:tEnd,5);
    armCardROI = totalData(tBegin:tEnd,6);
    
    interval = 300;
    interp = 'spline';
    
    armROIEnv_top = envelope(time, armROI, 'top', interval, interp);
    armROIEnv_bot = envelope(time, armROI, 'bottom', interval, interp);
    armRespROIEnv_top = envelope(time, armRespROI, 'top', interval, interp);
    armRespROIEnv_bot = envelope(time, armRespROI, 'bottom', interval, interp);
    armCardROIEnv_top = envelope(time, armCardROI, 'top', interval, interp);
    armCardROIEnv_bot = envelope(time, armCardROI, 'bottom', interval, interp);

    
    figure
    subplot(3,1,1), plot(time, armROI)
        hold on, plot(time, armROIEnv_top,'r'), 
        plot(time, armROIEnv_bot,'r')
    subplot(3,1,2), plot(time, armRespROI)
        hold on, plot(time, armRespROIEnv_top,'r'), 
        plot(time, armRespROIEnv_bot,'r')
    subplot(3,1,3), plot(time, armCardROI)
        hold on, plot(time, armCardROIEnv_top,'r'), 
        plot(time, armCardROIEnv_bot,'r')
    
end

    
%%

for indMarker = 5%:size(timeMarkerBioimpedance,2)-1;    
    % The time of the region of interest
    tBegin = timeMarkers(indMarker)+Fs*offset;
    tEnd = timeMarkers(indMarker)+Fs*totalTime;
    
    armROI = totalData(tBegin:tEnd,4);
    armRespROI = totalData(tBegin:tEnd,5);
    armCardROI = totalData(tBegin:tEnd,6);
    
    interval = 200;
    
%     armROIEnv_top = envelope(time, armROI, 'top', interval);

    figure
    plot(abs(hilbert(armCardROI)),'k')
    hold on, plot(armCardROI,'r')
    
    figure
    plot(abs(hilbert(armRespROI)),'k')
    hold on, plot(armRespROI,'b')
    
end

    




