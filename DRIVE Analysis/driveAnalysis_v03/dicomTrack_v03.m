function [data] = dicomTrack_v03
%DICOMTRACK Track points on a DICOM
%   Eventual expansion to multiple different ways to track
%
[filename, pathname] = uigetfile('*.DCM;*.dcm', ...
    'Choose DICOM images to work with', pwd, ...
    'MultiSelect', 'off');

addpath(genpath(pathname));
cd(pathname)

dicomFile = permute(dicomread(filename),[1, 2, 4, 3]);
[~, ~, dicomFrames, ~] = size(dicomFile);


%Adjust image
indFrame = 1;
while indFrame <= dicomFrames
   dicomFile(:,:,indFrame) = imadjust(dicomFile(:,:,indFrame));
    indFrame = indFrame + 1;
end


% Get region of interest
indFrame = 1;
objectFrame = dicomFile(:,:,indFrame);

imshow(objectFrame)
title('Select 2 points along the edge of the vessel, then hit "Enter"')
figHandle = gcf;
[poiX, poiY] = getpts(figHandle);
close

poiX = round(poiX);     poiY = round(poiY);
nPoints = size(poiX,1);
pointLog = zeros(nPoints, 2, dicomFrames);
points = [poiX, poiY];
% pointImage = insertMarker(objectFrame, points, '+', 'Color', 'white');

% % KALMAN FILTER
% dicomFile = uint8(kalmanStack(double(dicomFile(:,:,:,1)),0.5,0.5));

pointDist = zeros(dicomFrames,1);
newDicom = dicomFile;

% Create object tracker
tracker = vision.PointTracker('MaxBidirectionalError', inf);

% Initialize object tracker
initialize(tracker, points(:,:,1), objectFrame);


while indFrame <= dicomFrames
       %Track the points     
      frame = dicomFile(:,:,indFrame);
      [points, validity] = step(tracker, frame);
      pointLog(:,:,indFrame) = points;
      out = insertMarker(frame, points(validity, :), '+', 'Color', 'white');
      newDicom(:,:,indFrame) = out(:,:,1);
      
      %Compute the distance between the 2 points
      pointDist(indFrame) = ...
          sqrt ((pointLog(1,1,indFrame) - pointLog(2,1,indFrame)).^2 ...
          + (pointLog(1,2,indFrame) - pointLog(2,2,indFrame)).^2);
      
      indFrame = indFrame + 1;
      
end

info = dicominfo(filename);
dtUS = (info.FrameTime)*0.001;
FsUS = 1/dtUS;
time = (1:dicomFrames)/FsUS;
% 
% % Frequency analysis
% nPoints = length(pointDist);
% NFFT = 2^nextpow2(nPoints); % Next power of 2 from length of y
% Y = fft (pointDist,NFFT)/nPoints;
% f = FsUS/2*linspace(0,1,NFFT/2+1);
% 
% % freq = peakFreq(pointDist,FsUS,'band',[0.6, 2.0]);
% freq = 0.5;
% Get envelope of tracked motion
% envTop = envelope(time,pointDist,'top',FsUS,'linear');
% envBot = envelope(time,pointDist,'bottom',FsUS,'linear');
% 
% figure(1), plot(time, pointDist)
% hold on, plot(time, envTop,'r'), plot(time,envBot,'r')
% figure(2), plot(f,2*abs(Y(1:NFFT/2+1)))

% data.PIXELS_PER_MM = (228-152)/(5*10);    % 022414 C
% data.PIXELS_PER_MM = (250-164)/(5*10);    % 030614 C
% data.PIXELS_PER_MM = (210-142)/(5*10);    % 032714 B
% data.PIXELS_PER_MM = (297-185)/(10*10);   % 033114 A
% data.PIXELS_PER_MM = (258-196)/(5*10);   % 040314 A
% data.PIXELS_PER_MM = (258-196)/(5*10);   % 052714 A
% data.PIXELS_PER_MM = (217-119)/(5*10);   % 052914 A
data.PIXELS_PER_MM = (224-103)/(4*10);   % 060514 A


% data.PIXELS_PER_MM = (196-135)/(5*10);    % 041714 C
% data.PIXELS_PER_MM = (228-152)/(5*10);    % 042314 C
% data.PIXELS_PER_MM = (210-142)/(5*10);    % 042814 C
% data.PIXELS_PER_MM = (216-125)/(4*10);    % 052214 C1
% data.PIXELS_PER_MM = (284-165)/(5*10);    % 052214 C2
% data.PIXELS_PER_MM = (387-190)/(5*10);    % 071014 C1
% data.PIXELS_PER_MM = (521-282)/(5*10);    % 071014 C2
% data.PIXELS_PER_MM = (205-145)/(2*10);
% data.PIXELS_PER_MM = (396-244)/(5*10);
% data.PIXELS_PER_MM = (505-302)/(5*10);
% data.PIXELS_PER_MM = (360-224)/(5*10);
% data.PIXELS_PER_MM = (369-174)/(5*10);
% data.PIXELS_PER_MM = (321-94)/(5*10);
% data.PIXELS_PER_MM = (304-102)/(5*10);
% data.PIXELS_PER_MM = (382-196)/(5*10);     % 081514 A
% data.PIXELS_PER_MM = (440-225)/(5*10);     % 100214 A
% data.PIXELS_PER_MM = (348-255)/(2*10);     % 101514 A
% data.PIXELS_PER_MM = (393-271)/(2*10);     % 101514 B







data.FsUS = FsUS;
data.pointLog = pointLog;
data.pointDist = pointDist;    % in px (currently)
% data.envelope = [envTop; envBot];               % in px (currently)
% data.distens = (envTop-envBot)./envTop*100;     % in percent
data.timeUS = time;
data.DICOM = newDicom;

end

