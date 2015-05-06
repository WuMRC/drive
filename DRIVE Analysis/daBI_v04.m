%% DRIVE ANALYSIS V0.4 - BIOIMP
clear, close, clc

%% STEP 1 - GET DATA
% Select patient folder
[filename, pathname] = uigetfile('*.*','Pick an ACQ file');
addpath(genpath(pathname));
cd(pathname)
bioimp.acq = loadACQ(filename);

% % Get patient ID
patientID = pathname(end-9:end);
patientID(ismember(patientID,' ,.:;!/')) = [];

% FOR DIALYSIS
% Get patient ID
% patientID = pathname(end-25:end-19);
patientID(ismember(patientID,' ,.:;!/')) = [];

% %% STEP 2 - FILENAME BASED ON MARKER

nMarker = length(bioimp.acq.markers.szText);

for indMarker = 2:nMarker
    markerText = bioimp.acq.markers.szText{:,indMarker};%(11:end);
    % FOR DIALYSIS
    saveFilename = strcat('b','-after-',patientID,'-',markerText);
    saveFilename(ismember(saveFilename,' ,.:;!/')) = []
%     if length(markerText) > 12
%         markerText = markerText(11:end);
%         saveFilename = strcat('b','-before-',patientID,'-',markerText)
%         
%     else
%         saveFilename = strcat('b','-before-',patientID,'-',markerText)
%     end



    timeLength = 15;
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
        
        
        % PLOT
        % RESPIRATORY SIGNAL - ARM
        figure('units','normalized','outerposition',[0 0 1 1])
        subplot(2,2,1), hold on
        leading = 20; lagging = 20;
        plot(bioimp.nb.time(leading:end-lagging),...
            bioimp.nb.armS.total(leading:end-lagging),...
            'Color',[0.5 0.5 0.5],'LineWidth',2)
        plot(bioimp.nb.time(leading:end-lagging),...
            bioimp.nb.armS.resp(leading:end-lagging),...
            'Color','b','LineWidth',2)

        xlabel('Time [s]')
        ylabel('Total and Resp Z [Volts (need conversion)]')
        
        % CARDIAC SIGNAL - ARM
        subplot(2,2,2), hold on
        title('Arm impedance changes')
        plot(bioimp.nb.time(leading:end-lagging),...
            bioimp.nb.armS.card(leading:end-lagging),'Color','r','LineWidth',2)

        xlabel('Time [s]')
        ylabel('Card Z [Volts (need conversion)]')
        
        
        % RESPIRATORY SIGNAL - LEG
        subplot(2,2,3), hold on
        plot(bioimp.nb.time(leading:end-lagging),...
            bioimp.nb.legS.total(leading:end-lagging),...
            'Color',[0.5 0.5 0.5],'LineWidth',2)
        plot(bioimp.nb.time(leading:end-lagging),...
            bioimp.nb.legS.resp(leading:end-lagging),...
            'Color','b','LineWidth',2)

        xlabel('Time [s]')
        ylabel('Total and Resp Z [Volts (need conversion)]')
        
        % CARDIAC SIGNAL - LEG
        subplot(2,2,4), hold on
        title('Leg impedance changes')
        plot(bioimp.nb.time(leading:end-lagging),...
            bioimp.nb.legS.card(leading:end-lagging),'Color','r','LineWidth',2)

        xlabel('Time [s]')
        ylabel('Card Z [Volts (need conversion)]')
      
        savefig(saveFilename)
        
        % Want to save data file as well, for later manipulation
        
        close all
        
%     end
end