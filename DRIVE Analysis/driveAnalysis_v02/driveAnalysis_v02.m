%% DRIVE ANALYSIS V0.2

%% STEP 1 - GET DATA
% SELECT PATIENT FOLDER
[bioimp, ultrasound] = getDRIVEdata;

bioimp.acq = loadACQ(char(bioimp.files(1)));


%% STEP X - TRACK ULTRASOUND DATA
[ultrasound.data] = dicomTrack(ultrasound.files(12).name);

dicomInfo = dicominfo(ultrasound.files(12).name);
FsUS = 1/((dicomInfo.FrameTime)*0.001);
time = (1:dicomInfo.NumberOfFrames)/FsUS;

% Plot the diameter over time
plot(time, ultrasound.data.pointDist,'k')
hold on, plot(time, ultrasound.data.envelope(1,:)','r')
hold on, plot(time, ultrasound.data.envelope(2,:)','r')
xlabel('Time [s]'); ylabel('Distance [px]')
title('Distance between 2 points')

