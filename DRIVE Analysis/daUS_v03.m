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
