%% DRIVE ANALYSIS V0.2

%% STEP 1 - GET DATA
% SELECT PATIENT FOLDER
clear, close, clc
[bioimp, ultrasound] = getDRIVEdata;

%%
bioimp.acq = loadACQ(char(bioimp.files(1)));

%%
filename = 'b101514b-nb3.fig';
markerToInvestigate = 4;
fprintf(bioimp.acq.markers.szText{:,markerToInvestigate})
fprintf('\n')

%%
if bioimp.acq.markers.lMarkers > 2
    
    timeLength = 15;
    % NORMAL BREATHING
    if strfind(bioimp.acq.markers.szText{:,markerToInvestigate},'nb') > 0
        % GET RAW SIGNAL AND SAMPLING DATA
        bioimp.nb.dt = (bioimp.acq.hdr.graph.sample_time)*0.001; % in sec
        bioimp.nb.time = 0:bioimp.nb.dt:15-bioimp.nb.dt;
        bioimp.nb.FsBI = 1/bioimp.nb.dt;
        bioimp.nb.data = bioimp.acq.data(...
            bioimp.acq.markers.lSample(markerToInvestigate):...
            bioimp.acq.markers.lSample(markerToInvestigate)...
            +(timeLength*bioimp.nb.FsBI-1),:);
        
        % TOTAL SMOOTHED SIGNAL
        bioimp.nb.armS.total = smooth(smooth(...
            bioimp.nb.data(:,1),...
            bioimp.nb.FsBI/2),bioimp.nb.FsBI/10);
        bioimp.nb.legS.total = smooth(smooth(...
            bioimp.nb.data(:,2),...
            bioimp.nb.FsBI/2),bioimp.nb.FsBI/10);
        
        % RESPIRATORY SIGNAL
        bioimp.nb.armS.resp = smooth(bioimp.nb.armS.total,bioimp.nb.FsBI);
        bioimp.nb.legS.resp = smooth(bioimp.nb.legS.total,bioimp.nb.FsBI);
        
        % CARDIAC SIGNAL
        bioimp.nb.armS.card = bioimp.nb.armS.total-bioimp.nb.armS.resp;
        bioimp.nb.legS.card = bioimp.nb.legS.total-bioimp.nb.legS.resp;
        
%         % ENVELOPE DETECTION - RESPIRATORY SIGNAL
%         bioimp.nb.armEnv.respTop = envelope(bioimp.nb.time,...
%             bioimp.nb.armS.resp,'top',bioimp.nb.FsBI*2,'linear');
%         bioimp.nb.armEnv.respBot = envelope(bioimp.nb.time,...
%             bioimp.nb.armS.resp,'bottom',bioimp.nb.FsBI,'linear');
%         bioimp.nb.legEnv.respTop = envelope(bioimp.nb.time,...
%             bioimp.nb.legS.resp,'top',bioimp.nb.FsBI,'linear');
%         bioimp.nb.legEnv.respBot = envelope(bioimp.nb.time,...
%             bioimp.nb.legS.resp,'bottom',bioimp.nb.FsBI,'linear');
%%
        % ENVELOPE DETECTION - RESPIRATORY SIGNAL
        bioimp.nb.armEnv.respTop = smooth(abs(hilbert(bioimp.nb.armS.resp...
            -mean(bioimp.nb.armS.resp))) ...
            +mean(bioimp.nb.armS.resp),bioimp.nb.FsBI);
        bioimp.nb.armEnv.respBot = smooth(-abs(hilbert(bioimp.nb.armS.resp...
            -mean(bioimp.nb.armS.resp))) ...
            + mean(bioimp.nb.armS.resp),bioimp.nb.FsBI);
        bioimp.nb.legEnv.respTop = smooth(abs(hilbert(bioimp.nb.legS.resp...
            -mean(bioimp.nb.legS.resp))) ...
            +mean(bioimp.nb.legS.resp),bioimp.nb.FsBI);
        bioimp.nb.legEnv.respBot = smooth(-abs(hilbert(-bioimp.nb.legS.resp...
            +mean(bioimp.nb.legS.resp))) + mean(bioimp.nb.legS.resp),bioimp.nb.FsBI);
        
        
        bioimp.nb.armEnv.cardTop = abs(hilbert(bioimp.nb.armS.card...
            -mean(bioimp.nb.armS.card))) ...
            +mean(bioimp.nb.armS.card);
        bioimp.nb.armEnv.cardBot = -abs(hilbert(-bioimp.nb.armS.card...
            +mean(bioimp.nb.armS.card))) + mean(bioimp.nb.armS.card);
        bioimp.nb.legEnv.cardTop = abs(hilbert(bioimp.nb.legS.card...
            -mean(bioimp.nb.legS.card))) ...
            +mean(bioimp.nb.legS.card);
        bioimp.nb.legEnv.cardBot = -abs(hilbert(-bioimp.nb.legS.card...
            +mean(bioimp.nb.legS.card))) + mean(bioimp.nb.legS.card);


        
        
        % PLOT
        % RESPIRATORY SIGNAL - ARM
        figure('units','normalized','outerposition',[0 0 1 1])
        subplot(2,3,1), hold on
        leading = 20; lagging = 20;
        plot(bioimp.nb.time(leading:end-lagging),...
            bioimp.nb.armS.total(leading:end-lagging),...
            'Color',[0.5 0.5 0.5],'LineWidth',2)
        plot(bioimp.nb.time(leading:end-lagging),...
            bioimp.nb.armS.resp(leading:end-lagging),...
            'Color','b','LineWidth',2)
        plot(bioimp.nb.time(leading:end-lagging),...
            bioimp.nb.armEnv.respTop(leading:end-lagging),':b')
        plot(bioimp.nb.time(leading:end-lagging),...
            bioimp.nb.armEnv.respBot(leading:end-lagging),':b')
        xlabel('Time [s]')
        ylabel('Total and Resp Z [Volts (need conversion)]')
        
        % CARDIAC SIGNAL - ARM
        subplot(2,3,2), hold on
        title('Arm impedance changes to normal breathing')
        plot(bioimp.nb.time(leading:end-lagging),...
            bioimp.nb.armS.card(leading:end-lagging),'Color','r','LineWidth',2)
        plot(bioimp.nb.time(leading:end-lagging),...
            bioimp.nb.armEnv.cardTop(leading:end-lagging),':r')
        plot(bioimp.nb.time(leading:end-lagging),...
            bioimp.nb.armEnv.cardBot(leading:end-lagging),':r')
        xlabel('Time [s]')
        ylabel('Card Z [Volts (need conversion)]')
        
        % OVERALL CHANGES
        subplot(2,3,3), hold on
        plot(bioimp.nb.time(leading:end-lagging), ...
            bioimp.nb.armEnv.respTop(leading:end-lagging)...
            -bioimp.nb.armEnv.respBot(leading:end-lagging),'b')
        
        plot(bioimp.nb.time(leading:end-lagging), ...
            bioimp.nb.armEnv.cardTop(leading:end-lagging)...
            -bioimp.nb.armEnv.cardBot(leading:end-lagging),'r')
        
        legend(strcat('dZ_{Resp}=',...
            num2str(mean(bioimp.nb.armEnv.respTop(leading:end-lagging)...
            -bioimp.nb.armEnv.respBot(leading:end-lagging)))),...
            strcat('dZ_{Card}=',...
            num2str(mean(bioimp.nb.armEnv.cardTop(leading:end-lagging)...
            -bioimp.nb.armEnv.cardBot(leading:end-lagging)))))
        plot(bioimp.nb.time(leading:end-lagging), ...
            mean(bioimp.nb.armEnv.respTop(leading:end-lagging)...
            -bioimp.nb.armEnv.respBot(leading:end-lagging)),'b-')
        plot(bioimp.nb.time(leading:end-lagging), ...
            mean(bioimp.nb.armEnv.cardTop(leading:end-lagging)...
            -bioimp.nb.armEnv.cardBot(leading:end-lagging)),'r-')
        
        
        % RESPIRATORY SIGNAL - LEG
        subplot(2,3,4), hold on
        plot(bioimp.nb.time(leading:end-lagging),...
            bioimp.nb.legS.total(leading:end-lagging),...
            'Color',[0.5 0.5 0.5],'LineWidth',2)
        plot(bioimp.nb.time(leading:end-lagging),...
            bioimp.nb.legS.resp(leading:end-lagging),...
            'Color','b','LineWidth',2)
        plot(bioimp.nb.time(leading:end-lagging),...
            bioimp.nb.legEnv.respTop(leading:end-lagging),':b')
        plot(bioimp.nb.time(leading:end-lagging),...
            bioimp.nb.legEnv.respBot(leading:end-lagging),':b')
        xlabel('Time [s]')
        ylabel('Total and Resp Z [Volts (need conversion)]')
        
        % CARDIAC SIGNAL - LEG
        subplot(2,3,5), hold on
        title('Leg impedance changes to normal breathing')
        plot(bioimp.nb.time(leading:end-lagging),...
            bioimp.nb.legS.card(leading:end-lagging),'Color','r','LineWidth',2)
        plot(bioimp.nb.time(leading:end-lagging),...
            bioimp.nb.legEnv.cardTop(leading:end-lagging),':r')
        plot(bioimp.nb.time(leading:end-lagging),...
            bioimp.nb.legEnv.cardBot(leading:end-lagging),':r')
        xlabel('Time [s]')
        ylabel('Card Z [Volts (need conversion)]')
        
        % OVERALL CHANGES
        subplot(2,3,6), hold on
        plot(bioimp.nb.time(leading:end-lagging), ...
            bioimp.nb.legEnv.respTop(leading:end-lagging)...
            -bioimp.nb.legEnv.respBot(leading:end-lagging),'b')
        
        plot(bioimp.nb.time(leading:end-lagging), ...
            bioimp.nb.legEnv.cardTop(leading:end-lagging)...
            -bioimp.nb.legEnv.cardBot(leading:end-lagging),'r')
        
        legend(strcat('dZ_{Resp}=',...
            num2str(mean(bioimp.nb.legEnv.respTop(leading:end-lagging)...
            -bioimp.nb.legEnv.respBot(leading:end-lagging)))),...
            strcat('dZ_{Card}=',...
            num2str(mean(bioimp.nb.legEnv.cardTop(leading:end-lagging)...
            -bioimp.nb.legEnv.cardBot(leading:end-lagging)))))
        plot(bioimp.nb.time(leading:end-lagging), ...
            mean(bioimp.nb.legEnv.respTop(leading:end-lagging)...
            -bioimp.nb.legEnv.respBot(leading:end-lagging)),'b-')
        plot(bioimp.nb.time(leading:end-lagging), ...
            mean(bioimp.nb.legEnv.cardTop(leading:end-lagging)...
            -bioimp.nb.legEnv.cardBot(leading:end-lagging)),'r-')
        
        
        savefig(filename)
        
    else fprintf('Not a normal breathing exercise')
    end
end

%%
bioimpedanceLegSMOOTH = smooth(bioimpedanceLeg,Fs/2).*20;
bioimpedanceLegSMOOTH = smooth(bioimpedanceLegSMOOTH, 20);
respLegSMOOTH = smooth(bioimpedanceLegSMOOTH, 200);

bioimpedanceArmSMOOTH = smooth(bioimpedanceArm,Fs/2).*20;
bioimpedanceArmSMOOTH = smooth(bioimpedanceArmSMOOTH, 20);

figure, plot(timeBioimpedance, bioimpedanceArmSMOOTH(timeMarkerBioimpedanceInd(DATA_TO_ANALYZE):...
    (timeMarkerBioimpedanceInd(DATA_TO_ANALYZE)+200*10)))
%

%% STEP XX - BIOIMPEDANCE

FsBI = 1/bioimp.nb.dt;

%%

timeBIend = 15;
timeBI = (0:bioimp.nb.dt:timeBIend);

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




