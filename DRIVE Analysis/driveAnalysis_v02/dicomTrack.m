function [ultrasound] = dicomTrack( filename )
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
framenum = 1;
objectFrame = dicomFile(:,:,framenum);

imshow(objectFrame)
title('Select 2 points along the edge of the vessel, then hit "Enter"')
figHandle = gcf;
[poiX, poiY] = getpts(figHandle);
close

poiX = round(poiX);     poiY = round(poiY);
nPoints = size(poiX,1);
pointLog = zeros(nPoints, 2, dicomFrames);
points = [poiX, poiY];
pointImage = insertMarker(objectFrame, points, '+', 'Color', 'white');

pointDist = zeros(dicomFrames,1);
newDicom = dicomFile;

% Create object tracker
tracker = vision.PointTracker('MaxBidirectionalError', 1);

% Initialize object tracker
initialize(tracker, points(:,:,1), objectFrame);


while framenum <= dicomFrames
       %Track the points     
      frame = dicomFile(:,:,framenum);
      [points, validity] = step(tracker, frame);
      pointLog(:,:,framenum) = points;
      out = insertMarker(frame, points(validity, :), '+', 'Color', 'white');
      newDicom(:,:,framenum) = out(:,:,1);
      
      %Compute the distance between the 2 points
      pointDist(framenum) = sqrt ((pointLog(1,1,framenum) - pointLog(2,1,framenum)).^2+(pointLog(1,2,framenum) - pointLog(2,2,framenum)).^2);
      
      framenum = framenum + 1;
      
end

%Display figure showing distance between the points
info = dicominfo(filename);
dtUS = (info.FrameTime)*0.001;
FsUS = 1/dtUS;
% sampFreq = 16;  % Taken from image, though the DICOM should have this
time = (1:dicomFrames)/FsUS;


% Plot the tracked pixel movement
plot(time, pointDist)
xlabel('Time [s]'); ylabel('Distance [px]')
title('Distance between 2 points')

% Get envelope of tracked motion
envTop = envelope(time,pointDist,'top',FsUS,'linear');
envBot = envelope(time,pointDist,'bottom',FsUS,'linear');
hold on, plot(time, envTop,'r'), plot(time,envBot,'r')

figure, plot(time(11:93),envTop(11:93)-envBot(11:93))
xlabel('Time [s]'); ylabel('Diameter [px]')
title('Distance between 2 points')

%Show tracked points in the image
implay(newDicom(:,:,:,1))

ultrasound.data.pointDist = pointDist;
ultrasound.data.envelope = [envTop, envBottom];


end

