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

data.MM_PER_PIXEL_X = ...
    info.SequenceOfUltrasoundRegions.Item_1.PhysicalDeltaX*10;
data.MM_PER_PIXEL_Y = ...
    info.SequenceOfUltrasoundRegions.Item_1.PhysicalDeltaY*10;

if data.MM_PER_PIXEL_X == data.MM_PER_PIXEL_Y
    data.MM_PER_PIXEL = data.MM_PER_PIXEL_X;
else % Need to rewrite for not-square possibilities
    data.MM_PER_PIXEL = data.MM_PER_PIXEL_Y;
end


data.FsUS = FsUS;
data.pointLog = pointLog;
data.pointDist = pointDist;    % in px (currently)
data.timeUS = time;
data.DICOM = newDicom;

end

