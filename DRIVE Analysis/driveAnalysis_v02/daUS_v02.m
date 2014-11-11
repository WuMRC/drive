%% DRIVE ANALYSIS V0.2 - ULTRASOUND

%% STEP 1 - GET DATA
% SELECT PATIENT FOLDER
clear, close, clc
[bioimp, ultrasound] = getDRIVEdata;

%% STEP YY - TRACK ULTRASOUND DATA
fileUS = 1;
[ultrasound.data] = dicomTrack(ultrasound.files(fileUS).name);

implay(permute(ultrasound.data.DICOM,[1 2 4 3]))

%070714a - file4 = not trackable
%070714a - file5 = not trackable (forced maneuvers = hard)
%080814a - file5 = not trackable
%080814a - file6 = crashes MATLAB
%081814c3 - file1 = complete collapse
%101314c - file3 = not so good looking



%% YYY

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




