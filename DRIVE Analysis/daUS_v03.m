%% STEP YY - TRACK ULTRASOUND DATA

[ultrasound.data] = dicomTrack_v03;

% %% Plot IVC changes
ultrasound.data.resp = smooth(ultrasound.data.pointDist,ultrasound.data.FsUS);%, 'rloess');

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

%% Check video
implay(permute(ultrasound.data.DICOM,[1 2 4 3]))


%% Stroke Volume Variation Analysis
hold on
plot(ultrasound.data.card,'r')
% plot(abs(hilbert(ultrasound.data.card)),'k')
dataEnvTop = envelope(ultrasound.data.timeUS,ultrasound.data.card,'top',40,'cubic');
dataEnvBot = envelope(ultrasound.data.timeUS,ultrasound.data.card,'bottom',40,'cubic');
plot(dataEnvTop)
plot(dataEnvBot)
hold off

%% Shifted data
cardShift = ultrasound.data.card-dataEnvBot';
plot(cardShift,'r')

%% 
close
plot(cardShift,'r')
indValleys = find(cardShift==0);
hold on
n = 4;
trapz(cardShift(indValleys(n):indValleys(n+1)))
area(indValleys(n):indValleys(n+1),cardShift(indValleys(n):indValleys(n+1)),...
    'FaceColor','r')
hold off


