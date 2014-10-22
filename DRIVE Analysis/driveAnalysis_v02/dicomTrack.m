function [data] = dicomTrack( filename )
%DICOMTRACK Track points on a DICOM
%   Eventual expansion to multiple different ways to track
%

if nargin == 0
    disp('No file selected. Exiting function.')
    return
end

disp(['User selected: ', fullfile(filename)]);
[pathstr, name, ext] = fileparts(filename);

if strcmp(ext,'.DCM') || strcmp(ext,'.dcm')
    dicomFile = permute(dicomread(filename),[1, 2, 4, 3]);
    dicomSize = size(dicomFile);
    dicomFrames = dicomSize(3);
else
    disp('File chosen not a DICOM. Exiting function.')
    return
end


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

pointDist = zeros(dicomFrames,1);
newDicom = dicomFile;

% Create object tracker
tracker = vision.PointTracker('MaxBidirectionalError', 1);

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
envTop = envelope(time,pointDist,'top',FsUS,'linear');
envBot = envelope(time,pointDist,'bottom',FsUS,'linear');
% 
% figure(1), plot(time, pointDist)
% hold on, plot(time, envTop,'r'), plot(time,envBot,'r')
% figure(2), plot(f,2*abs(Y(1:NFFT/2+1)))

data.pointLog = pointLog;
data.pointDist = pointDist;
data.envelope = [envTop; envBot];


end

