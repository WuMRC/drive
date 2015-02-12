%% DRIVE ANALYSIS V0.4 - BIOIMP
clear, close, clc

%% STEP 1 - GET DATA
% Select patient folder
[filename, pathname] = uigetfile('*.*','Pick an ACQ file');
addpath(genpath(pathname));
cd(pathname)
bioimp.acq = loadACQ(filename);

% Get patient ID
patientID = pathname(end-9:end);
patientID(ismember(patientID,' ,.:;!/')) = [];

%% STEP 2 - FILENAME BASED ON MARKER

nMarker = length(bioimp.acq.markers.szText);

for indMarker = 2:nMarker
    markerText = bioimp.acq.markers.szText{:,indMarker};
%     if length(markerText) > 12
%         markerText = markerText(12:end);
        saveFilename = strcat('b',patientID,'-',markerText)
        
        timeLength = 15;
    % NORMAL BREATHING
%     if strfind(bioimp.acq.markers.szText{:,indMarker},'nb') > 0
        % GET RAW SIGNAL AND SAMPLING DATA
        bioimp.nb.dt = (bioimp.acq.hdr.graph.sample_time)*0.001; % in sec
        bioimp.nb.time = 0:bioimp.nb.dt:15-bioimp.nb.dt;
        bioimp.nb.FsBI = 1/bioimp.nb.dt;
        bioimp.nb.data = bioimp.acq.data(...
            bioimp.acq.markers.lSample(indMarker):...
            bioimp.acq.markers.lSample(indMarker)...
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
%
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
        title('Arm impedance changes')
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
        title('Leg impedance changes')
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
        
        
        savefig(saveFilename)
        
        close all
        
%     else
%     end
end