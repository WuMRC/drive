function [data] = dicomTrack_v05
%DICOMTRACK Track points on a DICOM
%   Eventual expansion to multiple different ways to track

%% Select file
[filename, pathname] = uigetfile('*.DCM;*.dcm', ...
    'Choose DICOM images to work with', pwd, ...
    'MultiSelect', 'off');

addpath(genpath(pathname));
cd(pathname)

%% Get 'meta' information
% Find patientID based on directory name
patientID = pathname(end-19:end-12);
patientID(ismember(patientID,' ,.:;!/')) = [];

dicomFile = permute(dicomread(filename),[1, 2, 4, 3]);
[~, ~, dicomFrames, ~] = size(dicomFile);

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

%% Image tracking
% Adjust image
indFrame = 1;
while indFrame <= dicomFrames
   dicomFile(:,:,indFrame) = imadjust(dicomFile(:,:,indFrame));
    indFrame = indFrame + 1;
end

% Select points to track
indFrame = 1;
objectFrame = dicomFile(:,:,indFrame);
imshow(objectFrame)
title('Select pairs of two points along the edge of the vessel, then hit "Enter"')
figHandle = gcf;
[poiX, poiY] = getpts(figHandle);
close

poiX = round(poiX);     poiY = round(poiY);
nPoints = size(poiX,1);


pointLog = zeros(nPoints, 2, dicomFrames);
points = [poiX, poiY];
pointDist = zeros(dicomFrames,1);
newDicom = dicomFile;

% Create object tracker
tracker = vision.PointTracker('MaxBidirectionalError', inf);

% Initialize object tracker
initialize(tracker, points(:,:,1), objectFrame);

% Track points while there are frames to track them
while indFrame <= dicomFrames
    % Track the points
    frame = dicomFile(:,:,indFrame);
    [points, validity] = step(tracker, frame);
    pointLog(:,:,indFrame) = points;
    out = insertMarker(frame, points(validity, :), '+', 'Color', 'white');
    newDicom(:,:,indFrame) = out(:,:,1);
    
    for indPair = 1:(nPoints/2)
        
        % Compute the distance between the two points
        pointDist(indFrame,indPair) = ...
            sqrt ((pointLog(indPair*2-1,1,indFrame) ...
            - pointLog(indPair*2,2,indFrame)).^2 ...
            + (pointLog(indPair*2-1,1,indFrame) ...
            - pointLog(indPair*2,2,indFrame)).^2);
        
        
    end
    indFrame = indFrame + 1;
    
    
end

%% Save data
data.filename = filename(1:end-4);
data.FsUS = FsUS;
data.pointLog = pointLog;
data.pointDist = pointDist.*data.MM_PER_PIXEL;    % in mm
data.timeUS = time;
data.DICOM = newDicom;
data.patientID = patientID;

end

