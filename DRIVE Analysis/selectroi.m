function [imageroi] = selectroi(I)
% SELECTROI     Select a region of interest from 4D DICOM

figure, imshow(I(:,:,1,1))

hBox = imrect;
% BW = createMask(hBox);
roiPosition = wait(hBox);
% roiPosition;
roi_yind = [roiPosition(1), roiPosition(1)+roiPosition(3), ...
    roiPosition(1)+roiPosition(3), roiPosition(1)];
roi_xind = [roiPosition(2), roiPosition(2), ...
    roiPosition(2)+roiPosition(4), roiPosition(2)+roiPosition(4)];
close

% Create region of interest image
imageroi = I(roi_xind(1):roi_xind(3),roi_yind(1):roi_yind(2),:,1);