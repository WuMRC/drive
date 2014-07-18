

%%
% SELECT PATIENT FOLDER
WORKING_DIRECTORY = uigetdir('','Select the patient directory');
ULTRASOUND_DIRECTORY = strcat(WORKING_DIRECTORY,'/ultrasound/');

addpath(genpath(WORKING_DIRECTORY));
cd(WORKING_DIRECTORY);
disp(horzcat('Working on subject: ',WORKING_DIRECTORY((end-7):end)))

% SELECT RELEVANT ULTRASOUND IMAGES
ultrasoundFileArrayInfo = dir(ULTRASOUND_DIRECTORY);
ultrasoundFileArray = ultrasoundFileArrayInfo(arrayfun(@(x) x.name(1), ...
    ultrasoundFileArrayInfo) ~= '.');

% SELECT RELEVANT BIOIMPEDANCE DATA
bioimpedanceFileArrayInfo = dir(WORKING_DIRECTORY);
bioimpedanceFileArray = bioimpedanceFileArrayInfo(arrayfun(@(x) x.name(1), ...
    bioimpedanceFileArrayInfo) ~= '.');
bioimpedanceFileArray = bioimpedanceFileArray(arrayfun(@(x) x.isdir, ...
    bioimpedanceFileArray) ~= 1);

%%
% Look for viable trials 
for trial = 1:size(ultrasoundFileArray,1)
    ultrasoundFile = ultrasoundFileArray(trial).name;
    imshow(ultrasoundFile(:,:,1,1))
    prompt = 'Is this a good trial? [Y/N] \n';
    str = input(prompt,'s');
    if str == 'Y' | str ==  'y'
        trialCheckArray(trial) = true;
    elseif str == 'N' | str == 'n'
        trialCheckArray(trial) = false;
%     elseif str ~= 'Y' | str ~=  'y' | str ~= 'N' | str ~= 'n'
%         disp('Please select either Y or N.')
%         if str == 'Y' | str ==  'y'
%             trialCheckArray(trial) = 1;
%         elseif str == 'N' | str == 'n'
%             trialCheckArray(trial) = 0;
%         end
    end
end
close

% Remove all nonrelavent ultrasound images
trialCheckArray = logical(trialCheckArray);
ultrasoundFileArray = ultrasoundFileArray(trialCheckArray);

% Time of file
for nTrials = 1:size(ultrasoundFileArray,1)
    x = str2num(ultrasoundFileArray(nTrials).name(9:14));
    timeMarkerUltrasound(nTrials) = x;
end
clear x;

%%
% Bioimpedance

bioimpedanceFile = bioimpedanceFileArray(1).name;
acq = load_acq(bioimpedanceFile);

bioimpedanceArm = acq.data(:,1);
bioimpedanceLeg = acq.data(:,2);
biomipedanceChest = acq.data(:,3);
bioimpedanceForearm = acq.data(:,4);

dtBioimpedance = acq.hdr.graph.sample_time;
timeStartBioimpedance = acq.hdr.graph.first_time_offset/1000;

day = 60*60*24;
timeStartBioimpedance6 = str2num(datestr(timeStartBioimpedance/day,'HHMMSS'));


% Test marker script
for nMarkers = 1:length(acq.markers.lSample)
    str2num(datestr(timeStartBioimpedance/day+...
        (double(acq.markers.lSample(nMarkers)/200)/day),'HHMMSS'));
end




%%
% load('imagedata.mat')

% [image, map, alpha, overlays] = dicomread(filename);

ultrasoundFile = ultrasoundFileArray(1).name;

image = dicomread(ultrasoundFile);
image_new = imageperm(image);
image_roi = double(selectroi(image_new));
% image_roi = image_new(:,:,:,1);

image_roiNORM = image_roi./max(max(max(image_roi)));

[nRows, nCols, nFrames] = size(image_roi);


%%
imshow(image_roiNORM(:,:,1,1))
title('Select points along the edge of the vessel, then hit "Enter"')
figHandle = gcf;
[poiX, poiY] = getpts(figHandle);
close

poiX = round(poiX);     poiY = round(poiY);
nPoints = size(poiX,1);


%%

level = graythresh(image_roiNORM(:,:,1));
imageTrackBW = im2bw(image_roiNORM(:,:,1),level);
% imageTrackEDGE = edge(currentFrameData.*imageTrackBW,'sobel');
imageTrackFILT = image_roiNORM(:,:,1).*imageTrackBW;
imagesc(imageTrackFILT)


%%
rowKernel   = 5; colKernel   = 5;   % KERNEL SIZE
rowSearch   = 5; colSearch   = 5;   % SEARCH WINDOW

filt = ones(5,5);

imageTrack = image_roi;

for i = 1:nPoints
posOriginal = [poiY(i), poiX(i)];
posNew = posOriginal;


h = waitbar(0 ,'Progress');
frameIncrement = 1;

total = nFrames-1;
for ind = 1:total
    
%     level = graythresh(image_roiNORM(:,:,ind));
%     imageTrackBW(:,:,ind) = im2bw(image_roiNORM(:,:,ind),level);
%     imageTrackBW(:,:,ind+1) = im2bw(image_roiNORM(:,:,ind+1),level); 
%     imageTrackFILT(:,:,ind) = image_roiNORM(:,:,ind).*imageTrackBW(:,:,ind);
%     imageTrackFILT(:,:,ind+1) = image_roiNORM(:,:,ind+1).*imageTrackBW(:,:,ind+1);

    currentFrameData = image_roiNORM(:,:,ind);
    nextFrameData = image_roiNORM(:,:,ind+1);
    
    [rho_c(:,:,ind)] = corr2D(currentFrameData, nextFrameData, filt, ...
        [rowKernel, colKernel], [rowSearch, colKernel], posNew);
    rho_n(:,:,ind) = rho_c(:,:,ind)./max(max(rho_c(:,:,ind)));
   
    
    % HOW MUCH TO MOVE
    [rowMove(ind), colMove(ind)] = find(rho_n(:,:,ind) ... 
        == max(max(rho_n(:,:,ind))));
    
    rowMove(ind) = rowMove(ind) - (rowSearch + 1);
    colMove(ind) = colMove(ind) - (colKernel + 1);
    
    rowMove_total = sum(rowMove);
    colMove_total = sum(colMove);
    
    poiRow = posOriginal(1) + rowMove_total;
    poiCol = posOriginal(2) + colMove_total;
    
    posNew = [poiRow, poiCol];
    
    % TRACKING SINGLE PIXEL
    imageTrack(poiRow,poiCol,ind) = 400;
    

%     [motionVectES, computationsES] = motionEstES(currentFrameData,nextFrameData,...
%         colKernel,colSearch);
%     [motionVectARPS, computationsARPS] = motionEstARPS(currentFrameData,nextFrameData,...
%         colKernel,colSearch);
    
    prog = (ind*i)/(total*nPoints);
    waitbar(prog,h,'Progress')
%     disp(horzcat('Frame ',num2str(ind),' of ', num2str(total)))
end

% close(h)
clear rowMove colMove rowMove_total colMove_total posNew posOriginal

% implay(rho_n)
% implay(imageTrack./max(max(max(imageTrack))))
end

close(h)
implay(imageTrack./max(max(max(imageTrack))))




%%
total = nFrames-1;
frame2frame = 2;
ctotal = microblockanalysis(image_roiNORM,20,2);




%%
clear, close, clc


