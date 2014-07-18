function [ctotal] = microblockanalysis(imageStack,blockSize,frame2frame)


[nBlockRows, nBlockCols, nFrames] = size(imageStack);

% blockSize = 8;

% nBlockRows = floor(nBlockRows/blockSize);
% nBlockCols = floor(nBlockCols/blockSize);


% h = waitbar(0 ,'Progress');
total = nFrames-1;
for indFrames = 1:total
    
currentFrameData = imageStack(:,:,indFrames);
nextFrameData = imageStack(:,:,indFrames+frame2frame);


for indRow = 1:blockSize:nBlockRows-blockSize
    for indCol = 1:blockSize:nBlockCols-blockSize
        
        indRowOffset = indRow + blockSize/2;
        indColOffset = indCol + blockSize/2;
        
        currentBlock = currentFrameData(...
            (indRowOffset-blockSize/2):(indRowOffset+blockSize/2),...
            (indColOffset-blockSize/2):(indColOffset+blockSize/2));
        nextBlock = nextFrameData(...
            (indRowOffset-blockSize/2):(indRowOffset+blockSize/2),...
            (indColOffset-blockSize/2):(indColOffset+blockSize/2));
        
        indRowSave = (indRow+blockSize-1)/blockSize;
        indColSave = (indCol+blockSize-1)/blockSize;
        x = normxcorr2(currentBlock,nextBlock);
        y = x((blockSize/2):(2*blockSize-blockSize/2),...
            (blockSize/2):(2*blockSize-blockSize/2));

        [max_c, imax] = max(abs(x(:)));
        [ypeak, xpeak] = ind2sub(size(x),imax(1));
        
        rowMove(indRowSave,indColSave,indFrames) = (ypeak-size(currentBlock,1));
        colMove(indRowSave,indColSave,indFrames) = (xpeak-size(currentBlock,2));
        
        % Check to see if motion detected is out of bounds
        % If it is, treat it as erroneous 
%         if abs(rowMove(indRowSave,indColSave,indFrames)) == blockSize |...
%                 abs(colMove(indRowSave,indColSave,indFrames)) == blockSize
%             
%                 
%             
%         end
        
%         corr_offset = [(ypeak-size(currentBlock,1)),...
%             (xpeak-size(currentBlock,2))];
        
        
        ctotal((indRow):(indRow+blockSize),...
            (indCol):(indCol+blockSize),indFrames) = y;
        
        
%         x((indRowOffset-blockSize):(indRowOffset+blockSize),...
%          (indColOffset-blockSize):(indColOffset+blockSize)) ...
%          = normxcorr2(currentBlock,nextBlock);
     
%         ctotal((indRowOffset-blockSize/2):(indRowOffset+blockSize/2),...
%             (indColOffset-blockSize/2):(indColOffset+blockSize/2)) = ...
%             x((indRowOffset-blockSize/2):(indRowOffset+blockSize/2),...
%             (indColOffset-blockSize/2):(indColOffset+blockSize/2));
        
%         [max_c, imax] = max(abs(c(:,:,indRowSave,indColSave)));
%         [ypeak, xpeak] = ind2sub(size(c),imax(1));
%         corr_offset(:,:,indRowSave,indColSave) = [(ypeak-size(currentBlock,1)), ...
%             (xpeak-size(currentBlock,2))];


%         normxcorr2(currentFrameData)
    end
end

% prog = (indFrames)/(total);
% waitbar(prog,h,'Progress')
end
% figure, imagesc(ctotal)
% close(h)

% %%
% frame = 200;
% indRow = 1:blockSize:nBlockRows-blockSize;
% indCol = 1:blockSize:nBlockRows-blockSize;
% [xx,yy] = meshgrid(indRow,indCol);
% figure,imshow(imageStack(:,:,frame))
% hold on, quiver(xx,yy,rowMove(:,:,frame),colMove(:,:,frame),'Color','y')
% 
% 
% %%
% clear ctotal xx yy rowMove colMove indRow inCol



