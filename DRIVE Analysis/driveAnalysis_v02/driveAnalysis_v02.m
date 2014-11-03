%% DRIVE ANALYSIS V0.2

%% STEP 1 - GET DATA
% SELECT PATIENT FOLDER
[bioimp, ultrasound] = getDRIVEdata;

bioimp.acq = loadACQ(char(bioimp.files(1)));


%% STEP 2 - TRACK ULTRASOUND DATA
fileUS = 1;
[ultrasound.data] = dicomTrack(ultrasound.files(fileUS).name);

implay(permute(ultrasound.data.DICOM,[1 2 4 3]))

%070714a - file4 = not trackable
%070714a - file5 = not trackable (forced maneuvers = hard)
%080814a - file5 = not trackable
%080814a - file6 = crashes MATLAB
%081814c3 - file1 = complete collapse
%101314c - file3 = not so good looking



%%

ultrasound.info = dicominfo(ultrasound.files(fileUS).name);
ultrasound.info.FsUS = 1/((ultrasound.info.FrameTime)*0.001);
ultrasound.data.timeUS = (1:ultrasound.info.NumberOfFrames)...
    /ultrasound.info.FsUS;

ultrasound.data.envelope.card = [envelope(ultrasound.data.timeUS, ...
    ultrasound.data.pointDist,'top',ultrasound.info.FsUS,'linear') 
    envelope(ultrasound.data.timeUS, ...
    ultrasound.data.pointDist,'bottom',ultrasound.info.FsUS,'linear')];
ultrasound.data.envelope.resp = [envelope(ultrasound.data.timeUS, ...
    ultrasound.data.pointDist,'top',ultrasound.info.FsUS*5,'linear') 
    envelope(ultrasound.data.timeUS, ...
    ultrasound.data.pointDist,'bottom',ultrasound.info.FsUS*5,'linear')];


% %% Respiratory Signal from IVC

ultrasound.data.resp = smooth(ultrasound.data.pointDist,ultrasound.info.FsUS);%, 'rloess');

subplot(2,1,1), plot(ultrasound.data.timeUS,...
    ultrasound.data.resp./ultrasound.data.PIXELS_PER_MM)
hold on, plot(ultrasound.data.timeUS,...
    ultrasound.data.pointDist./ultrasound.data.PIXELS_PER_MM,'k')
xlabel('Time [s]')
ylabel('IVC_D [mm]')

% %% Cardiac Signal from IVC
ultrasound.data.card = ultrasound.data.pointDist- ultrasound.data.resp;
subplot(2,1,2), plot(ultrasound.data.timeUS,ultrasound.data.card,'r')
xlabel('Time [s]')
ylabel('\Delta IVC_{D,cardiac} [mm]')

%%



% Plot the diameter/distensibility over time
subplot(2,1,1)
plot(ultrasound.data.timeUS, ultrasound.data.pointDist,'k')
hold on, plot(timeUS, ultrasound.data.envelope.card(1,:)','r')
hold on, plot(timeUS, ultrasound.data.envelope.card(2,:)','r')
hold on, plot(timeUS, ultrasound.data.envelope.resp(1,:)','b')
hold on, plot(timeUS, ultrasound.data.envelope.resp(2,:)','b')
xlabel('Time [s]'); ylabel('Diameter [mm]')
title('Distance between 2 points')

%%
% ultrasound.data.distens = 
subplot(2,1,2)
plot(ultrasound.data.timeUS, ultrasound.data.distens,'r')
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

