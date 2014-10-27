%% DRIVE ANALYSIS V0.2

%% STEP 1 - GET DATA
% SELECT PATIENT FOLDER
[bioimp, ultrasound] = getDRIVEdata;

bioimp.acq = loadACQ(char(bioimp.files(1)));


%% STEP 2 - TRACK ULTRASOUND DATA
[ultrasound.data] = dicomTrack(ultrasound.files(1).name);

% implay(permute(ultrasound.data.DICOM,[1 2 4 3]))
%%

dicomInfo = dicominfo(ultrasound.files(1).name);
FsUS = 1/((dicomInfo.FrameTime)*0.001);
timeUS = (1:dicomInfo.NumberOfFrames)/FsUS;

% Plot the diameter/distensibility over time
% subplot(2,1,1)
plot(timeUS, ultrasound.data.pointDist,'k')
% hold on, plot(time, ultrasound.data.envelope(1,:)','r')
% hold on, plot(time, ultrasound.data.envelope(2,:)','r')
xlabel('Time [s]'); ylabel('Diameter [px]')
title('Distance between 2 points')

subplot(2,1,2)
plot(timeUS, ultrasound.data.distens,'r')
xlabel('Time [s]'); ylabel('Distensibility [%]')
title('Distance between 2 points')

distensAvg = mean(ultrasound.data.distens(FsUS:end-FsUS));

%% STEP XX - BIOIMPEDANCE

dtBI = acq.hdr.graph.sample_time;

