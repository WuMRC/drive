function [newDicom] = trackAllThePoints(filename,pixelDensity,kalmanGain)
%TRACKALLTHEPOINTS Does some cool stuff

dicomFile = dicomread(filename);

[nRows, nCols, nColors, nFrames] = size(dicomFile);
dicomGray = zeros(nRows, nCols, nFrames);
for indFrame = 1:nFrames;
    dicomGray(:,:,indFrame) = rgb2gray(dicomFile(:,:,:,indFrame));
end

if kalmanGain >0
    dicomGrayFILT = uint8(kalmanStack(double(dicomGray),kalmanGain,0.5));
else
    dicomGrayFILT = uint8(dicomGray);
end
% 
% dicomSize = size(dicomGrayFILT);
% nFrames = dicomSize(3);
% %Adjust image
indFrame = 1;
while indFrame <= nFrames
   dicomGrayFILT(:,:,indFrame) = imadjust(dicomGrayFILT(:,:,indFrame));
%     enhancer = vision.HistogramEqualizer;
%     dicomFile(:,:,indFrame) = step(enhancer, dicomFile(:,:,indFrame));
    indFrame = indFrame + 1;
end


%Create grid of points on the image
pixelsX = nCols; pixelsY = nRows;
% pixelDensity = 5; %percentage of pixels you want to track (between 0-100)

if pixelDensity >100
    pixelDensity = 100;
elseif pixelDensity <=0;
    pixelDensity = 1;
end

% May want to choose a decimation factor?
pixelsBetweenX = (pixelsX-1)/round((pixelsX-1)*pixelDensity/100);
pixelsBetweenY = (pixelsY-1)/round((pixelsY-1)*pixelDensity/100);
count = 1;
countX = 1;

% We get an image that is %PixelDensity^2*(pixelsX*pixelsY)
while countX <= pixelsX+.0001
    countY=1;
    while countY <= pixelsY+.0001
        points(count,:) = [countX countY];
        countY = countY + pixelsBetweenY;
        count = count+1;
    end
    countX = countX + pixelsBetweenX;
end


nPoints = count - 1;
pointLog = zeros(nPoints, 2, nFrames);

framenum = 1;
objectFrame = dicomGrayFILT(:,:,1);
pointImage = insertMarker(objectFrame, points, '+', 'Color', 'white');

pointDist = zeros(nFrames);
newDicom = dicomGrayFILT;
newDicom(:,:,1) = pointImage(:,:,1);

% Create object tracker
tracker = vision.PointTracker('MaxBidirectionalError', 3); 

% Initialize object tracker
initialize(tracker, points(:,:,1), objectFrame);


while framenum <= nFrames
       %Track the points     
      frame = dicomGrayFILT(:,:,framenum);
      [points, validity] = step(tracker, frame);
      pointLog(:,:,framenum) = points;
      out = insertMarker(frame, points(validity, :), '+', 'Color', 'white');
      newDicom(:,:,framenum+1) = out(:,:,1);
      
      framenum = framenum + 1;
      
end

%Show tracked points in the image
% implay(newDicom(:,:,:,1))

%% Strain
% It's tough to unpack the way the points are currently used
% I think there could probably be some revision in the future to addres
% this

% Simple difference
for indFrames = 1:nFrames-1
    pointLogDiff(:,:,indFrames) = pointLog(:,:,indFrames+1) ...
        - pointLog(:,:,indFrames);
end
% Test
%plot(permute(pointLogDiff(2003,2,:),[1 3 2]))


% % Get the points
% counter = 1;
% for indFrame = 1:99
%     for ind = 1:57:4617
%         xPoints(:,counter,indFrame) = pointLog(ind:(ind+56),1,indFrame);
%         yPoints(:,counter,indFrame) = pointLog(ind:(ind+56),2,indFrame);
%         counter = counter+1;
%     end
%     counter = 1;
% end
pixelsXtracked = round(pixelsX*pixelDensity/100)+1;
pixelsYtracked = round(pixelsY*pixelDensity/100)+1;
trackedPixels = pixelsXtracked*pixelsYtracked;
% Separate x and y differences for each point on the image
counter = 1;
for indFrame = 1:nFrames-1
    for ind = 1:pixelsYtracked:trackedPixels
        xDiff(:,counter,indFrame) = pointLogDiff(ind:(ind+pixelsYtracked-1),1,indFrame);
        yDiff(:,counter,indFrame) = pointLogDiff(ind:(ind+pixelsYtracked-1),2,indFrame);
        counter = counter+1;
    end
    counter = 1;
end
totalDiff = sqrt(xDiff.^2 + yDiff.^2);

% Strain video
% implay(xDiff)
% implay(yDiff)
% implay(totalDiff)


end

