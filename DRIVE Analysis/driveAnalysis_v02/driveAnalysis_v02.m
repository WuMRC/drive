%% DRIVE ANALYSIS V0.2

%% STEP 1 - GET DATA
% SELECT PATIENT FOLDER
[bioimp, ultrasound] = getDRIVEdata;

bioimp.acq = loadACQ(char(bioimp.files(2)));


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

dtBI = (bioimp.acq.hdr.graph.sample_time)*0.001;    % in seconds
FsBI = 1/dtBI;

%%

timeBIend = 15;
timeBI = (0:dtBI:timeBIend);

indSample = 3;

dataOfInterest = bioimp.acq.data(bioimp.acq.markers.lSample(indSample):...
    bioimp.acq.markers.lSample(indSample)+timeBIend*FsBI,1);
dataOfInterestSMOOTH = smooth(dataOfInterest,FsBI/2);

plot(timeBI, dataOfInterest, 'Color', [0.75 0.75 0.75])
hold on
plot(timeBI, dataOfInterestSMOOTH, 'k')

envTopBI = envelope(timeBI,dataOfInterestSMOOTH,'top',FsBI*2,'linear');
envBotBI = envelope(timeBI,dataOfInterestSMOOTH,'bottom',FsBI*2,'linear');

plot(timeBI,envTopBI,'r')
plot(timeBI,envBotBI,'r')

