function [poiX, poiY, nPoints] = selectPoints(image)

% This will eventually be replaced by automation and will be used next
% week exclusively for clinician training
imshow(image)
title('Select points along the edge of the vessel, then hit "Enter"')
figHandle = gcf;
[poiX, poiY] = getpts(figHandle);
close

poiX = round(poiX);     poiY = round(poiY);
nPoints = size(poiX,1);